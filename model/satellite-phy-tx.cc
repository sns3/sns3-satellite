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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <cmath>

#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/object-factory.h"
#include "ns3/log.h"

#include "satellite-utils.h"
#include "satellite-phy.h"
#include "satellite-phy-tx.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel.h"
#include "satellite-antenna-gain-pattern.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyTx");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatPhyTx);


SatPhyTx::SatPhyTx ()
  : m_maxAntennaGain (),
    m_state (IDLE),
    m_beamId (),
    m_txMode (),
    m_defaultFadingValue ()
{
  NS_LOG_FUNCTION (this);
}


SatPhyTx::~SatPhyTx ()
{
  NS_LOG_FUNCTION (this);
}

void SatPhyTx::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_mobility = 0;
  m_fadingContainer = 0;
  Object::DoDispose ();
}

std::ostream& operator<< (std::ostream& os, SatPhyTx::State s)
{
  switch (s)
    {
    case SatPhyTx::IDLE:
      os << "IDLE";
      break;
    case SatPhyTx::TX:
      os << "TX";
      break;
    default:
      os << "UNKNOWN";
      break;
    }
  return os;
}

TypeId
SatPhyTx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyTx")
    .SetParent<Object> ()
    .AddAttribute ("TxMode", "Tx mode of this Phy Tx.",
                   EnumValue (SatPhyTx::NORMAL),
                   MakeEnumAccessor (&SatPhyTx::m_txMode),
                   MakeEnumChecker (SatPhyTx::NORMAL, "Normal Tx mode",
                                    SatPhyTx::TRANSPARENT, "Transparent Tx mode"))
  ;
  return tid;
}

void
SatPhyTx::SetMaxAntennaGain_Db (double gain_db)
{
  NS_LOG_FUNCTION (this << gain_db);

  m_maxAntennaGain = SatUtils::DbToLinear (gain_db);
}

double
SatPhyTx::GetAntennaGain (Ptr<MobilityModel> mobility)
{
  NS_LOG_FUNCTION (this);

  double gain_W (m_maxAntennaGain);

  // Get the transmit antenna gain at the receiver position.
  // E.g. GEO satellite transmits to the UT receiver.
  if (m_antennaGainPattern)
    {
      Ptr<SatMobilityModel> m = DynamicCast<SatMobilityModel> (mobility);
      gain_W = m_antennaGainPattern->GetAntennaGain_lin (m->GetGeoPosition ());
    }

  /**
   * If antenna gain pattern is not set, we use the
   * set maximum antenna gain.
   */

  return gain_W;
}

void
SatPhyTx::SetDefaultFadingValue (double fadingValue)
{
  NS_LOG_FUNCTION (this << fadingValue);

  m_defaultFadingValue = fadingValue;
}

double
SatPhyTx::GetFadingValue (Address macAddress, SatEnums::ChannelType_t channelType)
{
  NS_LOG_FUNCTION (this << macAddress << channelType);

  double fadingValue = m_defaultFadingValue;

  if (m_fadingContainer)
    {
      fadingValue = m_fadingContainer->GetFading (macAddress, channelType);
    }
  // Returns value 1 if fading is not set, as fading value is used as multiplier
  return fadingValue;
}

void
SatPhyTx::SetFadingContainer (Ptr<SatBaseFading> fadingContainer)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_fadingContainer == 0);

  m_fadingContainer = fadingContainer;
}

Ptr<MobilityModel>
SatPhyTx::GetMobility ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_mobility != 0);

  return m_mobility;
}

void
SatPhyTx::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  NS_ASSERT (m_mobility == 0);

  m_mobility = m;
}

void
SatPhyTx::SetAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_antennaGainPattern == 0);

  m_antennaGainPattern = agp;
}

void
SatPhyTx::SetChannel (Ptr<SatChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  NS_ASSERT (m_channel == 0);

  m_channel = c;
}

Ptr<SatChannel>
SatPhyTx::GetChannel ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_channel != 0);

  return m_channel;
}

void
SatPhyTx::ChangeState (State newState)
{
  NS_LOG_FUNCTION (this << newState);
  NS_LOG_INFO (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}

void
SatPhyTx::StartTx (Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this << txParams);
  NS_LOG_INFO (this << " state: " << m_state);

  switch (m_state)
    {
    case TX:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;

    case IDLE:
      {
        NS_ASSERT (m_channel);
        ChangeState (TX);
        m_channel->StartTx (txParams);

        /**
         * The SatPhyTx is mapped to a spot-beam, which means that there may be several
         * carriers handled by the same SatPhyTx. This is why the SatPhyTx state machine
         * needs to be overtaken, thus the SatPhyTx is allowed to send several overlapping
         * packets; in different carriers though.
         * TODO: The SatPhyTx state machine may need some (code quality) improvements.
         * E.g. different inherited implementations may be done for satellite and terrestrial
         * domain nodes.
         */
        if ( m_txMode == TRANSPARENT )
          {
            EndTx ();
          }
        else
          {
            Simulator::Schedule (txParams->m_duration, &SatPhyTx::EndTx, this);
          }
      }
      break;

    default:
      NS_FATAL_ERROR ("unknown state");
      break;
    }
}

void
SatPhyTx::EndTx ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (this << " state: " << m_state);

  if (m_state != TX)
    {
      NS_FATAL_ERROR ("SatPhyTx::EndTx - unexpected state!");
    }

  ChangeState (IDLE);
}

void
SatPhyTx::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  m_beamId = beamId;
}

} // namespace ns3
