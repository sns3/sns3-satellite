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
SatGwLlc::NotifyTxOpportunity (uint32_t bytes, Mac48Address macAddr, uint32_t &bytesLeft )
{
  NS_LOG_FUNCTION (this << macAddr << bytes);

  Ptr<Packet> packet;

  /**
   * TODO: This is not the final implementation! The NotifyTxOpportunity
   * will be enhanced with the flow id which to serve. The decision is passed
   * then to the scheduler (UT/NCC or forward link).
   */
  EncapKey_t key;
  // Check whether there are some control messages
  if (m_nodeInfo->GetNodeType () == SatEnums::NT_UT)
    {
      key = std::make_pair<Mac48Address, uint8_t> (macAddr, m_controlFlowIndex);
    }
  else if (m_nodeInfo->GetNodeType () == SatEnums::NT_GW)
    {
      key = std::make_pair<Mac48Address, uint8_t> (Mac48Address::GetBroadcast (), m_controlFlowIndex);
    }

  EncapContainer_t::iterator it = m_encaps.find (key);

  if (!it->second->GetQueue ()->IsEmpty())
    {
      packet = it->second->NotifyTxOpportunity (bytes, bytesLeft);
    }

  key = std::make_pair<Mac48Address, uint8_t> (macAddr, 1);
  it = m_encaps.find (key);
  if (!packet)
    {
      packet = it->second->NotifyTxOpportunity (bytes, bytesLeft);
    }

  if (packet)
    {
      SatEnums::SatLinkDir_t ld =
          (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

      // Add packet trace entry:
      m_packetTrace (Simulator::Now(),
                     SatEnums::PACKET_SENT,
                     m_nodeInfo->GetNodeType (),
                     m_nodeInfo->GetNodeId (),
                     m_nodeInfo->GetMacAddress (),
                     SatEnums::LL_LLC,
                     ld,
                     SatUtils::GetPacketInfo (packet));
    }

  return packet;
}


bool
SatGwLlc::ControlEncapsulatorCreated () const
{
  EncapKey_t key = std::make_pair<Mac48Address, uint8_t> (Mac48Address::GetBroadcast (), m_controlFlowIndex);
  EncapContainer_t::const_iterator it = m_encaps.find (key);
  if (it != m_encaps.end ())
    {
      return true;
    }

  return false;
}


} // namespace ns3


