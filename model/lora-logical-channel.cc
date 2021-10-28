/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "ns3/log.h"

#include "ns3/lora-logical-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraLogicalChannel");

NS_OBJECT_ENSURE_REGISTERED (LoraLogicalChannel);

TypeId
LoraLogicalChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraLogicalChannel")
    .SetParent<Object> ();
  return tid;
}

LoraLogicalChannel::LoraLogicalChannel () :
  m_frequency (0),
  m_minDataRate (0),
  m_maxDataRate (5),
  m_enabledForUplink (true)
{
  NS_LOG_FUNCTION (this);
}

LoraLogicalChannel::~LoraLogicalChannel ()
{
  NS_LOG_FUNCTION (this);
}

LoraLogicalChannel::LoraLogicalChannel (double frequency) :
  m_frequency (frequency),
  m_enabledForUplink (true)
{
  NS_LOG_FUNCTION (this);
}

LoraLogicalChannel::LoraLogicalChannel (double frequency, uint8_t minDataRate,
                                        uint8_t maxDataRate) :
  m_frequency (frequency),
  m_minDataRate (minDataRate),
  m_maxDataRate (maxDataRate),
  m_enabledForUplink (true)
{
  NS_LOG_FUNCTION (this);
}

double
LoraLogicalChannel::GetFrequency (void) const
{
  return m_frequency;
}

void
LoraLogicalChannel::SetMinimumDataRate (uint8_t minDataRate)
{
  m_minDataRate = minDataRate;
}

void
LoraLogicalChannel::SetMaximumDataRate (uint8_t maxDataRate)
{
  m_maxDataRate = maxDataRate;
}

uint8_t
LoraLogicalChannel::GetMinimumDataRate (void)
{
  return m_minDataRate;
}

uint8_t
LoraLogicalChannel::GetMaximumDataRate (void)
{
  return m_maxDataRate;
}

void
LoraLogicalChannel::SetEnabledForUplink (void)
{
  m_enabledForUplink = true;
}

void
LoraLogicalChannel::DisableForUplink (void)
{
  m_enabledForUplink = false;
}

bool
LoraLogicalChannel::IsEnabledForUplink (void)
{
  return m_enabledForUplink;
}

bool
operator== (const Ptr<LoraLogicalChannel>& first,
            const Ptr<LoraLogicalChannel>& second)
{
  double thisFreq = first->GetFrequency ();
  double otherFreq = second->GetFrequency ();

  NS_LOG_DEBUG ("Checking equality between logical lora channels: " <<
                thisFreq << " " << otherFreq);

  NS_LOG_DEBUG ("Result:" << (thisFreq == otherFreq));
  return (thisFreq == otherFreq);
}

bool
operator!= (const Ptr<LoraLogicalChannel>& first,
            const Ptr<LoraLogicalChannel>& second)
{
  return !(first == second);
}
}
