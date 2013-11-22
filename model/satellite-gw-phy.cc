/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Sami Rantanen <sami.rantanennen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"

#include "satellite-utils.h"
#include "satellite-gw-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"


NS_LOG_COMPONENT_DEFINE ("SatGwPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGwPhy);

SatGwPhy::SatGwPhy (void)
{
  NS_LOG_FUNCTION (this);
}

SatGwPhy::SatGwPhy (Ptr<NetDevice> d, Ptr<SatChannel> txCh, Ptr<SatChannel> rxCh, uint32_t beamId)
  : SatPhy (d, txCh, rxCh, beamId)
{

}

SatGwPhy::~SatGwPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGwPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  SatPhy::DoDispose ();
}

TypeId
SatGwPhy::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

TypeId
SatGwPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGwPhy")
    .SetParent<SatPhy> ()
    .AddConstructor<SatGwPhy> ()
    .AddAttribute ("PhyRx", "The PhyRx layer attached to this phy.",
                    PointerValue (),
                    MakePointerAccessor (&SatGwPhy::GetPhyRx, &SatGwPhy::SetPhyRx),
                    MakePointerChecker<SatPhyRx> ())
    .AddAttribute ("PhyTx", "The PhyTx layer attached to this phy.",
                    PointerValue (),
                    MakePointerAccessor (&SatGwPhy::GetPhyTx, &SatGwPhy::SetPhyTx),
                    MakePointerChecker<SatPhyTx> ())
    .AddAttribute ("RxMaxAntennaGainDb", "Maximum RX gain in Dbs",
                    DoubleValue(61.50),
                    MakeDoubleAccessor(&SatPhy::m_rxMaxAntennaGain_db),
                    MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxAntennaGainDb", "Maximum TX gain in Dbs",
                    DoubleValue(65.20),
                    MakeDoubleAccessor(&SatPhy::m_txMaxAntennaGain_db),
                    MakeDoubleChecker<double_t> ())
    .AddAttribute ("TxMaxPowerDbW", "Maximum TX power in Dbs",
                    DoubleValue(8.97),
                    MakeDoubleAccessor(&SatPhy::m_txMaxPower_dbW),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("TxOutputLossDb", "TX Output loss in Dbs",
                    DoubleValue(2.00),
                    MakeDoubleAccessor(&SatPhy::m_txOutputLoss_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("TxPointingLossDb", "TX Pointing loss in Dbs",
                    DoubleValue(1.10),
                    MakeDoubleAccessor(&SatPhy::m_txPointingLoss_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("TxOboLossDb", "TX OBO loss in Dbs",
                    DoubleValue(6.00),
                    MakeDoubleAccessor(&SatPhy::m_txOboLoss_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("TxAntennaLossDb", "TX Antenna loss in Dbs",
                    DoubleValue(0.00),
                    MakeDoubleAccessor(&SatPhy::m_txAntennaLoss_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("RxAntennaLossDb", "RX Antenna loss in Dbs",
                    DoubleValue(0.00),
                    MakeDoubleAccessor(&SatPhy::m_rxAntennaLoss_db),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("DefaultFadingValue", "Default value for fading",
                    DoubleValue(1.00),
                    MakeDoubleAccessor(&SatPhy::m_defaultFadingValue),
                    MakeDoubleChecker<double_t> ())
  ;
  return tid;
}

Ptr<SatPhyTx>
SatGwPhy::GetPhyTx () const
{
  NS_LOG_FUNCTION (this);
  return SatPhy::GetPhyTx ();
}

Ptr<SatPhyRx>
SatGwPhy::GetPhyRx () const
{
  NS_LOG_FUNCTION (this);
  return SatPhy::GetPhyRx ();
}

void
SatGwPhy::SetPhyTx (Ptr<SatPhyTx> phyTx)
{
  NS_LOG_FUNCTION (this << phyTx);
  SatPhy::SetPhyTx (phyTx);
}

void
SatGwPhy::SetPhyRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);
  SatPhy::SetPhyRx (phyRx);
}

void
SatGwPhy::DoStart ()
{
  NS_LOG_FUNCTION (this);
  SatPhy::DoStart ();
}

void
SatGwPhy::SendPdu (Ptr<Packet> p, uint32_t carrierId, Time duration )
{
  NS_LOG_FUNCTION (this << p << carrierId << duration);
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << carrierId << " duration: " << duration);

  SatPhy::SendPdu (p, carrierId, duration);
}

void
SatGwPhy::SendPdu (Ptr<Packet> p, Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << p << txParams);
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << txParams->m_carrierId << " duration: " << txParams->m_duration);

  SatPhy::SendPdu  (p, txParams);
}

} // namespace ns3
