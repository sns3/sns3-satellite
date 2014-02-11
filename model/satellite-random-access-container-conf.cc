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
    .AddAttribute ("CrdsaBackoffTime",
                   "CRDSA backoff time",
                   UintegerValue (5),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaBackoffTime),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaBackoffProbability",
                   "CRDSA backoff probability",
                   DoubleValue (0.00),
                   MakeDoubleAccessor (&SatRandomAccessConf::m_crdsaBackoffProbability),
                   MakeDoubleChecker<double> (0.0,1.0))
    .AddAttribute ("CrdsaMaximumCrdsaBackoffProbability",
                   "Maximum CRDSA backoff probability for RA logic",
                   DoubleValue (0.2),
                   MakeDoubleAccessor (&SatRandomAccessConf::m_crdsaMaximumBackoffProbability),
                   MakeDoubleChecker<double> (0.0,1.0))
    .AddAttribute ("CrdsaMaxUniquePayloadPerBlock",
                   "CRDSA max unique payloads per block",
                   UintegerValue (3),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaMaxUniquePayloadPerBlock),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CrdsaMaxConsecutiveBlocksAccessed",
                   "CRDSA max consecutive blocks accessed",
                   UintegerValue (4),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaMaxConsecutiveBlocksAccessed),
                   MakeUintegerChecker<uint32_t> ())
    ;
  return tid;
}

SatRandomAccessConf::SatRandomAccessConf () :
  m_slottedAlohaMinRandomizationValue (),
  m_slottedAlohaMaxRandomizationValue (),
  m_crdsaBackoffTime (),
  m_crdsaBackoffProbability (),
  m_crdsaMaximumBackoffProbability (),
  m_crdsaMaxUniquePayloadPerBlock (),
  m_crdsaMaxConsecutiveBlocksAccessed (),
  m_crdsaNumOfConsecutiveBlocksUsed ()
{
  NS_LOG_FUNCTION (this);

  Ptr<SatRandomAccessRequestClass> defaultConfRequestClass_0 = CreateObject<SatRandomAccessRequestClass> (0,129,2,2,2000);
  Ptr<SatRandomAccessRequestClass> defaultConfRequestClass_1 = CreateObject<SatRandomAccessRequestClass> (0,79,3,3,1000);
  Ptr<SatRandomAccessRequestClass> defaultConfRequestClass_2 = CreateObject<SatRandomAccessRequestClass> (0,159,4,4,3000);

  m_requestClassConf.insert (std::make_pair(0,defaultConfRequestClass_0));
  m_requestClassConf.insert (std::make_pair(1,defaultConfRequestClass_1));
  m_requestClassConf.insert (std::make_pair(2,defaultConfRequestClass_2));
}

SatRandomAccessConf::~SatRandomAccessConf ()
{
  NS_LOG_FUNCTION (this);
}


Ptr<SatRandomAccessRequestClass>
SatRandomAccessConf::GetRequestClassConfiguration (uint32_t requestClass)
{
  NS_LOG_FUNCTION (this);

  std::map<uint32_t,Ptr<SatRandomAccessRequestClass> >::iterator iter = m_requestClassConf.find (requestClass);

  if (iter == m_requestClassConf.end ())
    {
      NS_FATAL_ERROR ("SatRandomAccessConf::GetRequestClassConfiguration - Invalid request class");
    }
  return (iter->second);
}
} // namespace ns3
