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

// TODO implement it
void
SatLoraPhyRx::StartReceive (Ptr<Packet> packet, double rxPowerDbm,
                            uint8_t sf, Time duration,
                            double frequencyMHz)
{

}

// TODO implement it
void
SatLoraPhyRx::EndReceive (Ptr<Packet> packet)
{

}

// TODO implement it
void
SatLoraPhyRx::Send (Ptr<Packet> packet, LoraTxParameters txParams,
                     double frequencyMHz, double txPowerDbm)
{

}

// TODO implement it
bool
SatLoraPhyRx::IsTransmitting (void)
{
  return true;
}

// TODO implement it
bool
SatLoraPhyRx::IsOnFrequency (double frequency)
{
  return false;
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
