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

#include "ns3/simulator.h"
#include "ns3/log.h"

#include "satellite-gw-llc.h"
#include "satellite-scheduling-object.h"
#include "satellite-return-link-encapsulator.h"
#include "satellite-return-link-encapsulator-arq.h"
#include "satellite-generic-stream-encapsulator.h"
#include "satellite-generic-stream-encapsulator-arq.h"
#include "satellite-node-info.h"
#include "satellite-enums.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatGwLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGwLlc);

TypeId
SatGwLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGwLlc")
    .SetParent<SatLlc> ()
    .AddConstructor<SatGwLlc> ()
  ;
  return tid;
}

SatGwLlc::SatGwLlc ()
{
  NS_LOG_FUNCTION (this);
}

SatGwLlc::~SatGwLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGwLlc::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  SatLlc::DoDispose ();
}


Ptr<Packet>
SatGwLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address utAddr, uint8_t flowId, uint32_t &bytesLeft, uint32_t &nextMinTxO)
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
SatGwLlc::CreateEncap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_source << key->m_destination << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> gwEncap;

  if (m_fwdLinkArqEnabled)
    {
      gwEncap = CreateObject<SatGenericStreamEncapsulatorArq> (key->m_source, key->m_destination, key->m_flowId);
    }
  else
    {
      gwEncap = CreateObject<SatGenericStreamEncapsulator> (key->m_source, key->m_destination, key->m_flowId);
    }

  Ptr<SatQueue> queue = CreateObject<SatQueue> (key->m_flowId);
  gwEncap->SetQueue (queue);

  NS_LOG_INFO ("Create encapsulator with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ")");

  // Store the encapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_encaps.insert (std::make_pair (key, gwEncap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}

void
SatGwLlc::CreateDecap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_source << key->m_destination << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> gwDecap;

  if (m_rtnLinkArqEnabled)
    {
      gwDecap = CreateObject<SatReturnLinkEncapsulatorArq> (key->m_source, key->m_destination, key->m_flowId);
    }
  else
    {
      gwDecap = CreateObject<SatReturnLinkEncapsulator> (key->m_source, key->m_destination, key->m_flowId);
    }

  gwDecap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, this));
  gwDecap->SetCtrlMsgCallback (m_sendCtrlCallback);

  NS_LOG_INFO ("Create decapsulator with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ")");

  // Store the decapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_decaps.insert (std::make_pair (key, gwDecap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_source << ", " << key->m_destination << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}


void SatGwLlc::GetSchedulingContexts (std::vector< Ptr<SatSchedulingObject> > & output) const
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
          Ptr<SatSchedulingObject> so =
            Create<SatSchedulingObject> (cit->first->m_destination, buf, minTxOpportunityInBytes, holDelay, cit->first->m_flowId);
          output.push_back (so);
        }
    }
}

uint32_t
SatGwLlc::GetNBytesInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first->m_destination == utAddress)
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
SatGwLlc::GetNPacketsInQueue (Mac48Address utAddress) const
{
  NS_LOG_FUNCTION (this << utAddress);

  uint32_t sum = 0;

  for (EncapContainer_t::const_iterator it = m_encaps.begin ();
       it != m_encaps.end (); ++it)
    {
      if (it->first->m_destination == utAddress)
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


