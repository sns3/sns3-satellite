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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#include "satellite-time-tag.h"
#include "satellite-utils.h"
#include "satellite-scheduling-object.h"

#include "satellite-geo-llc.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoLlc);

TypeId
SatGeoLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoLlc")
    .SetParent<SatLlc> ()
  ;
  return tid;
}

SatGeoLlc::SatGeoLlc ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false); // this version of the constructor should not been used
}

SatGeoLlc::SatGeoLlc (SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                      SatEnums::RegenerationMode_t returnLinkRegenerationMode)
 : SatLlc (forwardLinkRegenerationMode, returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this);
}

SatGeoLlc::~SatGeoLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoLlc::DoDispose ()
{
  Object::DoDispose ();
}

void
SatGeoLlc::SetReceiveSatelliteCallback (SatGeoLlc::ReceiveSatelliteCallback cb)
{
  m_rxSatelliteCallback = cb;
}

bool
SatGeoLlc::Enque (Ptr<Packet> packet, Address dest, uint8_t flowId)
{
  NS_LOG_FUNCTION (this << packet << dest << (uint32_t) flowId);
  NS_LOG_INFO ("p=" << packet );
  NS_LOG_INFO ("dest=" << dest );
  NS_LOG_INFO ("UID is " << packet->GetUid ());

  Ptr<EncapKey> key = Create<EncapKey> (m_nodeInfo->GetMacAddress (), Mac48Address::ConvertFrom (dest), flowId, m_nodeInfo->GetMacAddress (), Mac48Address::ConvertFrom (dest));

  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it == m_encaps.end ())
    {
      /**
       * Encapsulator not found, thus create a new one. This method is
       * implemented in the inherited classes, which knows which type
       * of encapsulator to create.
       */
      CreateEncap (key);
      it = m_encaps.find (key);
    }

  it->second->EnquePdu (packet, Mac48Address::ConvertFrom (dest));

  SatEnums::SatLinkDir_t ld = GetSatLinkTxDir ();

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_ENQUE,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_LLC,
                 ld,
                 SatUtils::GetPacketInfo (packet));

  return true;
}

Ptr<Packet>
SatGeoLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address utAddr, uint8_t flowId, uint32_t &bytesLeft, uint32_t &nextMinTxO)
{
  NS_LOG_FUNCTION (this << utAddr << bytes << (uint32_t) flowId);

  Ptr<Packet> packet;
  Ptr<EncapKey> key = Create<EncapKey> (m_nodeInfo->GetMacAddress (), utAddr, flowId);
  EncapContainer_t::iterator it = m_encaps.find (key);

  if (it != m_encaps.end ())
    {
      packet = it->second->NotifyTxOpportunity (bytes, bytesLeft, nextMinTxO);

      if (packet)
        {
          SatEnums::SatLinkDir_t ld = SatEnums::LD_FORWARD;

          // Add packet trace entry:
          m_packetTrace (Simulator::Now (),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_LLC,
                         ld,
                         SatUtils::GetPacketInfo (packet));
        }
    }
  else
    {
      NS_FATAL_ERROR ("Encapsulator not found for key (" << m_nodeInfo->GetMacAddress () << ", " << utAddr << ", " << (uint32_t) flowId << ")");
    }

  return packet;
}

void
SatGeoLlc::ReceiveHigherLayerPdu (Ptr<Packet> packet, Mac48Address source, Mac48Address dest)
{
  NS_LOG_FUNCTION (this << packet << source << dest);

  // Peek control msg tag
  SatControlMsgTag ctrlTag;
  bool cSuccess = packet->PeekPacketTag (ctrlTag);

  if (cSuccess)
    {
      if (ctrlTag.GetMsgType () != SatControlMsgTag::SAT_ARQ_ACK)
        {
          m_rxSatelliteCallback (packet, m_nodeInfo->GetMacAddress ());
          return;
        }

      // ARQ ACKs need to be forwarded to LLC/ARQ for processing
      uint32_t ackId = ctrlTag.GetMsgId ();

      Ptr<SatArqAckMessage> ack = DynamicCast<SatArqAckMessage> (m_readCtrlCallback (ackId));

      if ( ack == NULL )
        {
          NS_FATAL_ERROR ("ARQ ACK not found, check that control msg storage time is set long enough!");
        }

      ReceiveAck (ack, source, dest);
    }
  // Higher layer packet
  else
    {
      m_rxSatelliteCallback (packet, m_nodeInfo->GetMacAddress ());
    }
}

void
SatGeoLlc::CreateEncap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_encapAddress << key->m_decapAddress << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> encap = CreateObject<SatBaseEncapsulator> (key->m_encapAddress,
                                                                      key->m_decapAddress,
                                                                      key->m_sourceE2EAddress,
                                                                      key->m_destE2EAddress,
                                                                      key->m_flowId);

  Ptr<SatQueue> queue = CreateObject<SatQueue> (key->m_flowId);
  encap->SetQueue (queue);

  NS_LOG_INFO ("Create encapsulator with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ")");

  // Store the encapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_encaps.insert (std::make_pair (key, encap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}

void
SatGeoLlc::CreateDecap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_encapAddress << key->m_decapAddress << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> decap = CreateObject<SatBaseEncapsulator> (key->m_encapAddress,
                                                                      key->m_decapAddress,
                                                                      key->m_sourceE2EAddress,
                                                                      key->m_destE2EAddress,
                                                                      key->m_flowId);

  decap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, this));
  decap->SetCtrlMsgCallback (m_sendCtrlCallback);

  NS_LOG_INFO ("Create decapsulator with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ")");

  // Store the decapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_decaps.insert (std::make_pair (key, decap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}

void SatGeoLlc::GetSchedulingContexts (std::vector< Ptr<SatSchedulingObject> > & output) const
{
  NS_LOG_FUNCTION (this);

  // Head of link queuing delay
  Time holDelay;

  // Then the user data
  for (EncapContainer_t::const_iterator cit = m_encaps.begin ();
       cit != m_encaps.end ();
       ++cit)
    {
      uint32_t buf = cit->second->GetTxBufferSizeInBytes ();

      if (buf > 0)
        {
          holDelay = cit->second->GetHolDelay ();
          uint32_t minTxOpportunityInBytes = cit->second->GetMinTxOpportunityInBytes ();
          Ptr<SatSchedulingObject> so = Create<SatSchedulingObject> (cit->first->m_decapAddress, buf, minTxOpportunityInBytes, holDelay, cit->first->m_flowId);
          output.push_back (so);
        }
    }
}

uint32_t
SatGeoLlc::GetNBytesInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first->m_encapAddress == utAddress)
        {
          NS_ASSERT (it->second != 0);
          Ptr<SatQueue> queue = it->second->GetQueue ();
          NS_ASSERT (queue != 0);
          sum += queue->GetNBytes ();
        }
    }

  return sum;
}

uint32_t
SatGeoLlc::GetNPacketsInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first->m_encapAddress == utAddress)
        {
          NS_ASSERT (it->second != 0);
          Ptr<SatQueue> queue = it->second->GetQueue ();
          NS_ASSERT (queue != 0);
          sum += queue->GetNPackets ();
        }
    }

  return sum;
}

} // namespace ns3


