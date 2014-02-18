/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/double.h"

#include "satellite-mac-tag.h"
#include "satellite-net-device.h"
#include "satellite-signal-parameters.h"
#include "satellite-gw-mac.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatGwMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGwMac);

TypeId
SatGwMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGwMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatGwMac> ()
    .AddAttribute ("DummyFrameSendingEnabled",
                   "Flag to tell, if dummy frames are sent or not.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatGwMac::m_dummyFrameSendingEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("Scheduler",
                   "Forward link scheduler used by this Sat GW MAC.",
                   PointerValue (),
                   MakePointerAccessor (&SatGwMac::m_scheduler),
                   MakePointerChecker<SatFwdLinkScheduler> ())
    .AddAttribute ("GuardTime",
                   "Guard time in forward link",
                   TimeValue (MicroSeconds (1)),
                   MakeTimeAccessor (&SatGwMac::m_guardTime),
                   MakeTimeChecker ())
  ;
  return tid;
}

SatGwMac::SatGwMac ()
 : SatMac (),
   m_scheduler (),
   m_dummyFrameSendingEnabled (false),
   m_guardTime (MicroSeconds (1))
{
  NS_LOG_FUNCTION (this);
}

SatGwMac::SatGwMac (uint32_t beamId)
 : SatMac (beamId),
   m_scheduler (),
   m_dummyFrameSendingEnabled (false),
   m_guardTime (MicroSeconds (1))
{
  NS_LOG_FUNCTION (this);
}

SatGwMac::~SatGwMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGwMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  SatMac::DoDispose ();
}

void
SatGwMac::StartScheduling()
{
  if ( m_scheduler == NULL )
    {
      NS_FATAL_ERROR ("Scheduler not set for GW MAC!!!");
    }

  // Note, carrierId currently set by default to 0
  Simulator::Schedule (Seconds (0), &SatGwMac::TransmitTime, this, 0);
}

void
SatGwMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this);

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_MAC,
                 SatEnums::LD_RETURN,
                 SatUtils::GetPacketInfo (packets));

  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      // Remove packet tag
      SatMacTag macTag;
      bool mSuccess = (*i)->PeekPacketTag (macTag);
      if (!mSuccess)
        {
          NS_FATAL_ERROR ("MAC tag was not found from the packet!");
        }

      NS_LOG_LOGIC ("Packet from " << macTag.GetSourceAddress() << " to " << macTag.GetDestAddress());
      NS_LOG_LOGIC ("Receiver " << m_nodeInfo->GetMacAddress ());

      // If the packet is intended for this receiver
      Mac48Address destAddress = Mac48Address::ConvertFrom (macTag.GetDestAddress ());

      if (destAddress == m_nodeInfo->GetMacAddress () || destAddress.IsBroadcast ())
        {
          // Remove control msg tag
          SatControlMsgTag ctrlTag;
          bool cSuccess = (*i)->RemovePacketTag (ctrlTag);

          if (cSuccess)
            {
              SatControlMsgTag::SatControlMsgType_t cType = ctrlTag.GetMsgType ();

              if ( cType != SatControlMsgTag::SAT_NON_CTRL_MSG )
                {
                  Mac48Address sourceAddress = Mac48Address::ConvertFrom (macTag.GetSourceAddress ());

                  ReceiveSignalingPacket (sourceAddress, *i, ctrlTag);
                }
              else
                {
                  NS_FATAL_ERROR ("A control message received with not valid msg type!");
                }
            }
          else
            {
              // Pass the source address to LLC
              m_rxCallback (*i, Mac48Address::ConvertFrom (macTag.GetSourceAddress ()));
            }
        }
      else
        {
          NS_LOG_LOGIC("Packet intended for others received by MAC: " << m_nodeInfo->GetMacAddress ());
        }
    }
}

void
SatGwMac::TransmitTime (uint32_t carrierId)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatBbFrame> bbFrame = m_scheduler->GetNextFrame ();

  if ( bbFrame == NULL )
    {
      NS_FATAL_ERROR ("BB Frame is missing!!!");
    }

  Time txDuration = bbFrame->GetDuration ();

  // Always sent if non dummy frame in question. Dummy frames sent only when sending is enabled
  if ( ( bbFrame->GetFrameType () != SatEnums::DUMMY_FRAME ) || m_dummyFrameSendingEnabled )
    {
      // Add packet trace entry:
      m_packetTrace (Simulator::Now (),
                     SatEnums::PACKET_SENT,
                     m_nodeInfo->GetNodeType (),
                     m_nodeInfo->GetNodeId (),
                     m_nodeInfo->GetMacAddress (),
                     SatEnums::LL_MAC,
                     SatEnums::LD_FORWARD,
                     SatUtils::GetPacketInfo (bbFrame->GetTransmitData ()));
                     
      /* TODO: The carrierId should be acquired from somewhere. Now
       * we assume only one carrier in forward link, so it is safe to use 0.
       */
      // Decrease a guard time from BB frame duration.
      SendPacket (bbFrame->GetTransmitData (), carrierId, txDuration - m_guardTime );
    }

  Simulator::Schedule (txDuration, &SatGwMac::TransmitTime, this, 0);
}

void
SatGwMac::ReceiveSignalingPacket (Mac48Address sourceAddress, Ptr<Packet> packet, const SatControlMsgTag &ctrlTag)
{
  switch (ctrlTag.GetMsgType ())
  {
    case SatControlMsgTag::SAT_CR_CTRL_MSG:
      {
        uint32_t msgId = ctrlTag.GetMsgId ();
        Ptr<SatCrMessage> crMsg = DynamicCast<SatCrMessage> ( m_readCtrlCallback (msgId) );

        if ( crMsg == NULL )
          {
            NS_FATAL_ERROR ("Error in reading CR message from container!!!");
          }

        m_scheduler->CnoInfoUpdated (sourceAddress, crMsg->GetCnoEstimate ());

        if ( m_crReceiveCallback.IsNull () == false )
          {
            m_crReceiveCallback (m_beamId, sourceAddress, crMsg);
          }
        break;
      }

    default:
      {
        NS_FATAL_ERROR ("SatGwMac received a non-supported control packet!");
        break;
      }
  }
}

void
SatGwMac::SetCrReceiveCallback (SatGwMac::CrReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_crReceiveCallback = cb;
}

} // namespace ns3
