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
 */

#include "ns3/satellite-lora-phy-rx.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SatLoraPhyRx");

NS_OBJECT_ENSURE_REGISTERED (SatLoraPhyRx);

TypeId
SatLoraPhyRx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLoraPhyRx")
    .SetParent<SatPhyRx> ();
  return tid;
}

SatLoraPhyRx::SatLoraPhyRx ()
  : m_state (SLEEP)
{
}

SatLoraPhyRx::~SatLoraPhyRx ()
{
}

void
SatLoraPhyRx::StartRx (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);

  // Switch on the current PHY state
  switch (m_state)
    {
    // In the SLEEP, TX and RX cases we cannot receive the packet: we only add
    // it to the list of interferers and do not schedule an EndReceive event for
    // it.
    case SLEEP:
      {
        NS_LOG_INFO ("Dropping packet because device is in SLEEP state");
        break;
      }
    case TX:
      {
        NS_LOG_INFO ("Dropping packet because device is in TX state");
        break;
      }
    case RX:
      {
        NS_LOG_INFO ("Dropping packet because device is already in RX state");
        break;
      }
    // If we are in STANDBY mode, we can potentially lock on the currently
    // incoming transmission
    case STANDBY:
      {
        SatPhyRx::StartRx (rxParams);
        break;
      }
    }
}

bool
SatLoraPhyRx::IsTransmitting (void)
{
  return true;
}

bool
SatLoraPhyRx::IsOnFrequency (double frequency)
{
  return m_frequency == frequency;
}

void
SatLoraPhyRx::SetFrequency (double frequencyMHz)
{
  m_frequency = frequencyMHz;
}

void
SatLoraPhyRx::SetSpreadingFactor (uint8_t sf)
{
  m_sf = sf;
}

SatLoraPhyRx::State
SatLoraPhyRx::GetState (){
  return m_state;
}

void
SatLoraPhyRx::SwitchToStandby (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_state = STANDBY;
}

void
SatLoraPhyRx::SwitchToRx (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_state == STANDBY);

  m_state = RX;
}

void
SatLoraPhyRx::SwitchToTx ()
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_state != RX);

  m_state = TX;
}

void
SatLoraPhyRx::SwitchToSleep (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_state == STANDBY);

  m_state = SLEEP;
}

}
