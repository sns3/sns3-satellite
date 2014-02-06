/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#include "satellite-random-access-container-conf.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("SatRandomAccessConf");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRandomAccessConf);

TypeId 
SatRandomAccessConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRandomAccessConf")
    .SetParent<Object> ()
    .AddAttribute ("SlottedAlohaMinRandomizationValue",
                   "Slotted ALOHA randomization minimum value",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&SatRandomAccessConf::m_slottedAlohaMinRandomizationValue),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SlottedAlohaMaxRandomizationValue",
                   "Slotted ALOHA randomization maximum value",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&SatRandomAccessConf::m_slottedAlohaMaxRandomizationValue),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CrdsaMinRandomizationValue",
                   "CRDSA randomization minimum value",
                   UintegerValue (0),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaMinRandomizationValue),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaMaxRandomizationValue",
                   "CRDSA randomization maximum value",
                   UintegerValue (159),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaMaxRandomizationValue),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaNumOfInstances",
                   "Number of instances within the block for CRDSA",
                   UintegerValue (3),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaNumOfInstances),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaBackoffTime",
                   "CRDSA backoff time",
                   UintegerValue (5),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaBackoffTime),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaBackoffProbability",
                   "CRDSA backoff probability",
                   DoubleValue (0.05),
                   MakeDoubleAccessor (&SatRandomAccessConf::m_crdsaBackoffProbability),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CrdsaMaxUniquePayloadPerBlock",
                   "CRDSA maximum unique payloads per block",
                   UintegerValue (3),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaMaxUniquePayloadPerBlock),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaMaxConsecutiveBlocksAccessed",
                   "CRDSA maximum consecutive blocks accessed",
                   UintegerValue (4),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaMaxConsecutiveBlocksAccessed),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaMinIdleBlocks",
                   "CRDSA minimum idle blocks",
                   UintegerValue (2),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaMinIdleBlocks),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaPayloadBytes",
                   "CRDSA payload bytes",
                   UintegerValue (2000),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaPayloadBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RaLogicMaximumCrdsaBackoffProbability",
                   "Maximum CRDSA backoff probability for RA logic",
                   DoubleValue (0.2),
                   MakeDoubleAccessor (&SatRandomAccessConf::m_raLogicMaximumCrdsaBackoffProbability),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

SatRandomAccessConf::SatRandomAccessConf () :
  m_slottedAlohaMinRandomizationValue (0.5),
  m_slottedAlohaMaxRandomizationValue (2.0),
  m_crdsaMinRandomizationValue (0),
  m_crdsaMaxRandomizationValue (159),
  m_crdsaNumOfInstances (3),
  m_crdsaBackoffTime (5),
  m_crdsaBackoffProbability (0.05),
  m_crdsaMaxUniquePayloadPerBlock (3),
  m_crdsaMaxConsecutiveBlocksAccessed (4),
  m_crdsaMinIdleBlocks (2),
  m_crdsaPayloadBytes (2000), /// TODO this should be waveform dependent
  m_raLogicMaximumCrdsaBackoffProbability (0.20)
{
  NS_LOG_FUNCTION (this);
}

SatRandomAccessConf::~SatRandomAccessConf ()
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
