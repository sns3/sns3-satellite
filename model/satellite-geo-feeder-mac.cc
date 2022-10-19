/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>

#include "satellite-utils.h"
#include "satellite-mac.h"
#include "satellite-time-tag.h"
#include "satellite-address-tag.h"
#include "satellite-signal-parameters.h"

#include "satellite-geo-feeder-mac.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoFeederMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoFeederMac);

TypeId
SatGeoFeederMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoFeederMac")
    .SetParent<SatGeoMac> ()
    .AddConstructor<SatGeoFeederMac> ()
    .AddAttribute ("GuardTime",
                   "Guard time in this SCPC scheduler",
                   TimeValue (MicroSeconds (1)),
                   MakeTimeAccessor (&SatGeoMac::GetGuardTime,
                                     &SatGeoMac::SetGuardTime),
                   MakeTimeChecker ())
    ;
  return tid;
}

TypeId
SatGeoFeederMac::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGeoFeederMac::SatGeoFeederMac (void)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatGeoFeederMac default constructor is not allowed to use");
}

SatGeoFeederMac::SatGeoFeederMac (SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                  SatEnums::RegenerationMode_t returnLinkRegenerationMode)
 : SatGeoMac (0, forwardLinkRegenerationMode, returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this);
}

SatGeoFeederMac::~SatGeoFeederMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoFeederMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatGeoFeederMac::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
SatGeoFeederMac::EnquePacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  SatAddressE2ETag addressE2ETag;
  bool success = packet->PeekPacketTag (addressE2ETag);

  SatMacTag mTag;
  success &= packet->RemovePacketTag (mTag);

  if (m_returnLinkRegenerationMode != SatEnums::REGENERATION_NETWORK)
    {
      // MAC tag and E2E address tag found
      if (success)
        {
          mTag.SetDestAddress (addressE2ETag.GetE2EDestAddress ());
          mTag.SetSourceAddress (m_nodeInfo->GetMacAddress ());
          packet->AddPacketTag (mTag);
        }
    }

  uint8_t flowId = 1;
  SatControlMsgTag ctrlTag;
  if (packet->PeekPacketTag (ctrlTag))
    {
      flowId = 0;
    }

  m_llc->Enque (packet, addressE2ETag.GetE2EDestAddress (), flowId);
}

void
SatGeoFeederMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);

  if (m_forwardLinkRegenerationMode == SatEnums::REGENERATION_LINK || m_forwardLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
      // Add packet trace entry:
      m_packetTrace (Simulator::Now (),
                     SatEnums::PACKET_RECV,
                     m_nodeInfo->GetNodeType (),
                     m_nodeInfo->GetNodeId (),
                     m_nodeInfo->GetMacAddress (),
                     SatEnums::LL_MAC,
                     SatEnums::LD_FORWARD,
                     SatUtils::GetPacketInfo (packets));

      RxTraces (packets);
    }

  rxParams->m_packetsInBurst.clear ();
  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      // Remove packet tag
      SatMacTag macTag;
      bool mSuccess = (*i)->PeekPacketTag (macTag);
      if (!mSuccess)
        {
          NS_FATAL_ERROR ("MAC tag was not found from the packet!");
        }

      NS_LOG_INFO ("Packet from " << macTag.GetSourceAddress () << " to " << macTag.GetDestAddress ());
      NS_LOG_INFO ("Receiver " << m_nodeInfo->GetMacAddress ());

      SatAddressE2ETag satAddressE2ETag;
      mSuccess = (*i)->PeekPacketTag (satAddressE2ETag);
      if (!mSuccess)
        {
          NS_FATAL_ERROR ("SatAddressE2E tag was not found from the packet!");
        }
      Mac48Address destE2EAddress = satAddressE2ETag.GetE2EDestAddress ();
      if (destE2EAddress == m_nodeInfo->GetMacAddress ())
        {
          // Remove control msg tag
          SatControlMsgTag ctrlTag;
          bool cSuccess = (*i)->PeekPacketTag (ctrlTag);

          if (cSuccess)
            {
              SatControlMsgTag::SatControlMsgType_t cType = ctrlTag.GetMsgType ();

              if ( cType != SatControlMsgTag::SAT_NON_CTRL_MSG )
                {
                  ReceiveSignalingPacket (*i);
                }
              else
                {
                  NS_FATAL_ERROR ("A control message received with not valid msg type!");
                }
            }
        }
      else
        {
          rxParams->m_packetsInBurst.push_back (*i);
        }
    }

  if (m_forwardLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
      for (SatPhy::PacketContainer_t::iterator i = rxParams->m_packetsInBurst.begin (); i != rxParams->m_packetsInBurst.end (); i++ )
        {
          // Remove packet tag
          SatMacTag macTag;
          bool mSuccess = (*i)->PeekPacketTag (macTag);
          if (!mSuccess)
            {
              NS_FATAL_ERROR ("MAC tag was not found from the packet!");
            }
          Mac48Address destAddress = macTag.GetDestAddress ();

          NS_LOG_INFO ("Packet from " << macTag.GetSourceAddress () << " to " << macTag.GetDestAddress ());
          NS_LOG_INFO ("Receiver " << m_nodeInfo->GetMacAddress ());

          if (destAddress == m_nodeInfo->GetMacAddress () || destAddress.IsBroadcast () || destAddress.IsGroup ())
            {
              m_rxCallback (*i, macTag.GetSourceAddress (), macTag.GetDestAddress ());
            }
        }
    }
  else
    {
      m_rxNetDeviceCallback (rxParams->m_packetsInBurst, rxParams);
    }
}

void
SatGeoFeederMac::ReceiveSignalingPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Remove the mac tag
  SatMacTag macTag;
  packet->PeekPacketTag (macTag);

  // Peek control msg tag
  SatControlMsgTag ctrlTag;
  bool cSuccess = packet->PeekPacketTag (ctrlTag);

  if (!cSuccess)
    {
      NS_FATAL_ERROR ("SatControlMsgTag not found in the packet!");
    }

  switch (ctrlTag.GetMsgType ())
    {
      case SatControlMsgTag::SAT_CN0_REPORT:
        {
          uint32_t msgId = ctrlTag.GetMsgId ();
          Ptr<SatCnoReportMessage> cnoReport = DynamicCast<SatCnoReportMessage> ( m_readCtrlCallback (msgId) );

          if ( cnoReport != NULL )
            {
              m_fwdScheduler->CnoInfoUpdated (macTag.GetSourceAddress (), cnoReport->GetCnoEstimate ());
            }
          else
            {
              NS_LOG_WARN ("Control message " << ctrlTag.GetMsgType () << " is not found from the RTN link control msg container!" << " at: " << Now ().GetSeconds () << "s");
            }

          packet->RemovePacketTag (macTag);
          packet->RemovePacketTag (ctrlTag);

          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Control message unkonwn on feeder MAC");
        }
    }
}

SatEnums::SatLinkDir_t
SatGeoFeederMac::GetSatLinkTxDir ()
{
  return SatEnums::LD_FORWARD;
}

SatEnums::SatLinkDir_t
SatGeoFeederMac::GetSatLinkRxDir ()
{
  return SatEnums::LD_RETURN;
}

Address
SatGeoFeederMac::GetRxUtAddress (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  Address utAddr; // invalid address.

  SatAddressE2ETag addressE2ETag;
  if (packet->PeekPacketTag (addressE2ETag))
    {
      NS_LOG_DEBUG (this << " contains a SatE2E tag");
      utAddr = addressE2ETag.GetE2EDestAddress ();
    }

  return utAddr;
}

} // namespace ns3
