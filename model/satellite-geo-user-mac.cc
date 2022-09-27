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
#include "satellite-geo-user-mac.h"
#include "satellite-mac.h"
#include "satellite-time-tag.h"
#include "satellite-address-tag.h"
#include "satellite-signal-parameters.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoUserMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoUserMac);

TypeId
SatGeoUserMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoUserMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatGeoUserMac> ()
  ;
  return tid;
}

TypeId
SatGeoUserMac::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGeoUserMac::SatGeoUserMac (void)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatGeoUserMac default constructor is not allowed to use");
}

SatGeoUserMac::SatGeoUserMac (uint32_t beamId,
                              SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                              SatEnums::RegenerationMode_t returnLinkRegenerationMode)
 : SatMac (beamId)
{
  NS_LOG_FUNCTION (this);

  m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
  m_returnLinkRegenerationMode = returnLinkRegenerationMode;
}

SatGeoUserMac::~SatGeoUserMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoUserMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatGeoUserMac::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
SatGeoUserMac::SendPackets (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this);

  // Update MAC address source and destination
  for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
       it != packets.end (); ++it)
    {
      SatMacTag mTag;
      bool success = (*it)->RemovePacketTag (mTag);

      SatAddressE2ETag addressE2ETag;
      success &= (*it)->PeekPacketTag (addressE2ETag);

      // MAC tag and E2E address tag found
      if (success)
        {
          mTag.SetDestAddress (addressE2ETag.GetE2EDestAddress ());
          mTag.SetSourceAddress (m_nodeInfo->GetMacAddress ());
          (*it)->AddPacketTag (mTag);
        }
    }
  m_txUserCallback (txParams);
}

void
SatGeoUserMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
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

  RxTraces (packets);

  m_rxUserCallback (packets, rxParams);
}

void
SatGeoUserMac::SetTransmitUserCallback (TransmitUserCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_txUserCallback = cb;
}

void
SatGeoUserMac::SetReceiveUserCallback (ReceiveUserCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxUserCallback = cb;
}

void
SatGeoUserMac::RxTraces (SatPhy::PacketContainer_t packets)
{
  NS_LOG_FUNCTION (this);

  if (m_isStatisticsTagsEnabled)
    {
      for (SatPhy::PacketContainer_t::const_iterator it1 = packets.begin ();
           it1 != packets.end (); ++it1)
        {
          // Remove packet tag
          SatMacTag macTag;
          bool mSuccess = (*it1)->PeekPacketTag (macTag);
          if (!mSuccess)
            {
              NS_FATAL_ERROR ("MAC tag was not found from the packet!");
            }

          // If the packet is intended for this receiver
          Mac48Address destAddress = macTag.GetDestAddress ();

          if (destAddress == m_nodeInfo->GetMacAddress ())
            {
              Address addr; // invalid address.

              bool isTaggedWithAddress = false;
              ByteTagIterator it2 = (*it1)->GetByteTagIterator ();

              while (!isTaggedWithAddress && it2.HasNext ())
                {
                  ByteTagIterator::Item item = it2.Next ();

                  if (item.GetTypeId () == SatAddressTag::GetTypeId ())
                    {
                      NS_LOG_DEBUG (this << " contains a SatAddressTag tag:"
                                         << " start=" << item.GetStart ()
                                         << " end=" << item.GetEnd ());
                      SatAddressTag addrTag;
                      item.GetTag (addrTag);
                      addr = addrTag.GetSourceAddress ();
                      isTaggedWithAddress = true; // this will exit the while loop.
                    }
                }

              m_rxTrace (*it1, addr);

              SatMacLinkTimeTag linkTimeTag;
              if ((*it1)->RemovePacketTag (linkTimeTag))
                {
                  NS_LOG_DEBUG (this << " contains a SatMacLinkTimeTag tag");
                  Time delay = Simulator::Now () - linkTimeTag.GetSenderLinkTimestamp ();
                  m_rxLinkDelayTrace (delay, addr);
                  if (m_lastLinkDelay.IsZero() == false)
                    {
                      Time jitter = Abs (delay - m_lastLinkDelay);
                      m_rxLinkJitterTrace (jitter, addr);
                    }
                  m_lastLinkDelay = delay;
                }
            } // end of `if (destAddress == m_nodeInfo->GetMacAddress () || destAddress.IsBroadcast ())`
        } // end of `for it1 = packets.begin () -> packets.end ()`
    } // end of `if (m_isStatisticsTagsEnabled)`
}

SatEnums::SatLinkDir_t
SatGeoUserMac::GetSatLinkTxDir ()
{
  return SatEnums::LD_FORWARD;
}

SatEnums::SatLinkDir_t
SatGeoUserMac::GetSatLinkRxDir ()
{
  return SatEnums::LD_RETURN;
}

} // namespace ns3
