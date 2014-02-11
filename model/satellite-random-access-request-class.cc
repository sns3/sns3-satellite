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
#include "satellite-random-access-request-class.h"

NS_LOG_COMPONENT_DEFINE ("SatRandomAccessRequestClass");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRandomAccessRequestClass);

TypeId 
SatRandomAccessRequestClass::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRandomAccessRequestClass")
    .SetParent<Object> ()
    ;
  return tid;
}

SatRandomAccessRequestClass::SatRandomAccessRequestClass () :
  m_minRandomizationValue (),
  m_maxRandomizationValue (),
  m_numOfInstances (),
  m_minIdleBlocks (),
  m_payloadBytes (), /// TODO this should be waveform dependent
  m_idleBlocksLeft ()
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatRandomAccessRequestClass::SatRandomAccessRequestClass - Constructor not in use");
}

SatRandomAccessRequestClass::SatRandomAccessRequestClass (uint32_t minRandomizationValue,
                                                          uint32_t maxRandomizationValue,
                                                          uint32_t numOfInstances,
                                                          uint32_t minIdleBlocks,
                                                          uint32_t payloadBytes) :
  m_minRandomizationValue (minRandomizationValue),
  m_maxRandomizationValue (maxRandomizationValue),
  m_numOfInstances (numOfInstances),
  m_minIdleBlocks (minIdleBlocks),
  m_payloadBytes (payloadBytes), /// TODO this should be waveform dependent
  m_idleBlocksLeft (0)
{
  NS_LOG_FUNCTION (this);

  DoVariableSanityCheck ();
}

SatRandomAccessRequestClass::~SatRandomAccessRequestClass ()
{
  NS_LOG_FUNCTION (this);
}

void
SatRandomAccessRequestClass::DoVariableSanityCheck ()
{
  NS_LOG_FUNCTION (this);

  if (m_minRandomizationValue < 0 || m_maxRandomizationValue < 0)
    {
      NS_FATAL_ERROR ("SatRandomAccessRequestClass::DoVariableSanityCheck - min < 0 || max < 0");
    }

  if (m_minRandomizationValue > m_maxRandomizationValue)
    {
      NS_FATAL_ERROR ("SatRandomAccessRequestClass::DoVariableSanityCheck - min > max");
    }

  if (m_numOfInstances < 1)
    {
      NS_FATAL_ERROR ("SatRandomAccessRequestClass::DoVariableSanityCheck - instances < 1");
    }

  if ( (m_maxRandomizationValue - m_minRandomizationValue) < m_numOfInstances)
    {
      NS_FATAL_ERROR ("SatRandomAccessRequestClass::DoVariableSanityCheck - (max - min) < instances");
    }

  NS_LOG_INFO ("SatRandomAccessRequestClass::DoVariableSanityCheck - Variable sanity check done");
}

} // namespace ns3
