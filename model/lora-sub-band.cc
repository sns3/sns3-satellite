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

#include <ns3/log.h>

#include "lora-sub-band.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraSubBand");

NS_OBJECT_ENSURE_REGISTERED (LoraSubBand);

TypeId
LoraSubBand::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraSubBand")
    .SetParent<Object> ();
  return tid;
}

LoraSubBand::LoraSubBand ()
{
  NS_LOG_FUNCTION (this);
}

  LoraSubBand::LoraSubBand (double firstFrequency, double lastFrequency, double dutyCycle,
                    double maxTxPowerDbm) :
    m_firstFrequency (firstFrequency),
    m_lastFrequency (lastFrequency),
    m_dutyCycle (dutyCycle),
    m_nextTransmissionTime (Seconds (0)),
    m_maxTxPowerDbm (maxTxPowerDbm)
  {
    NS_LOG_FUNCTION (this << firstFrequency << lastFrequency << dutyCycle <<
                     maxTxPowerDbm);
  }

  LoraSubBand::~LoraSubBand ()
  {
    NS_LOG_FUNCTION (this);
  }

  double
  LoraSubBand::GetFirstFrequency (void)
  {
    return m_firstFrequency;
  }

  double
  LoraSubBand::GetDutyCycle (void)
  {
    return m_dutyCycle;
  }

  bool
  LoraSubBand::BelongsToLoraSubBand (double frequency)
  {
    return (frequency > m_firstFrequency) && (frequency < m_lastFrequency);
  }

  bool
  LoraSubBand::BelongsToLoraSubBand (Ptr<LoraLogicalChannel> logicalChannel)
  {
    double frequency = logicalChannel->GetFrequency ();
    return BelongsToLoraSubBand (frequency);
  }

  void
  LoraSubBand::SetNextTransmissionTime (Time nextTime)
  {
    m_nextTransmissionTime = nextTime;
  }

  Time
  LoraSubBand::GetNextTransmissionTime (void)
  {
    return m_nextTransmissionTime;
  }

  void
  LoraSubBand::SetMaxTxPowerDbm (double maxTxPowerDbm)
  {
    m_maxTxPowerDbm = maxTxPowerDbm;
  }

  double
  LoraSubBand::GetMaxTxPowerDbm (void)
  {
    return m_maxTxPowerDbm;
  }
}
