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

#include "lorawan-mac.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LorawanMac");

NS_OBJECT_ENSURE_REGISTERED (LorawanMac);

TypeId
LorawanMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LorawanMac")
    .SetParent<SatMac> ()
    .AddTraceSource ("SentNewPacket",
                     "Trace source indicating a new packet "
                     "arrived at the MAC layer",
                     MakeTraceSourceAccessor (&LorawanMac::m_sentNewPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("ReceivedPacket",
                     "Trace source indicating a packet "
                     "was correctly received at the MAC layer",
                     MakeTraceSourceAccessor (&LorawanMac::m_receivedPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("CannotSendBecauseDutyCycle",
                     "Trace source indicating a packet "
                     "could not be sent immediately because of duty cycle limitations",
                     MakeTraceSourceAccessor (&LorawanMac::m_cannotSendBecauseDutyCycle),
                     "ns3::Packet::TracedCallback");
  return tid;
}

LorawanMac::LorawanMac ()
{
  NS_FATAL_ERROR ("Default constructor not in use");
}

LorawanMac::LorawanMac (uint32_t beamId)
  : SatMac (beamId)
{
  NS_LOG_FUNCTION (this);
}

LorawanMac::~LorawanMac ()
{
  NS_LOG_FUNCTION (this);
}

void
LorawanMac::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  Send (packet);
}

void
LorawanMac::SetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

Ptr<NetDevice>
LorawanMac::GetDevice (void)
{
  return m_device;
}

Ptr<SatPhy>
LorawanMac::GetPhy (void)
{
  return m_phy;
}

void
LorawanMac::SetPhy (Ptr<SatPhy> phy)
{
  // Set the phy
  m_phy = phy;
  DynamicCast<SatLoraPhyTx> (m_phy->GetPhyTx ())->SetTxFinishedCallback (MakeCallback (&LorawanMac::TxFinished, this));
}

LoraLogicalChannelHelper
LorawanMac::GetLoraLogicalChannelHelper (void)
{
  return m_channelHelper;
}

void
LorawanMac::SetLoraLogicalChannelHelper (LoraLogicalChannelHelper helper)
{
  m_channelHelper = helper;
}

uint8_t
LorawanMac::GetSfFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned(dataRate));

  // Check we are in range
  if (dataRate >= m_sfForDataRate.size ())
    {
      return 0;
    }

  return m_sfForDataRate.at (dataRate);
}

double
LorawanMac::GetBandwidthFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned(dataRate));

  // Check we are in range
  if (dataRate > m_bandwidthForDataRate.size ())
    {
      return 0;
    }

  return m_bandwidthForDataRate.at (dataRate);
}

double
LorawanMac::GetDbmForTxPower (uint8_t txPower)
{
  NS_LOG_FUNCTION (this << unsigned (txPower));

  if (txPower > m_txDbmForTxPower.size ())
    {
      return 0;
    }

  return m_txDbmForTxPower.at (txPower);
}

Time
LorawanMac::GetOnAirTime (Ptr<Packet> packet, LoraTxParameters txParams)
{

  NS_LOG_FUNCTION (packet);

  // The contents of this function are based on [1].
  // [1] SX1272 LoRa modem designer's guide.

  // Compute the symbol duration
  // Bandwidth is in Hz
  double tSym = pow (2, int(txParams.sf)) / (txParams.bandwidthHz);

  // Compute the preamble duration
  double tPreamble = (double(txParams.nPreamble) + 4.25) * tSym;

  // Payload size
  uint32_t pl = packet->GetSize ();      // Size in bytes
  NS_LOG_DEBUG ("Packet of size " << pl << " bytes");

  // This step is needed since the formula deals with double values.
  // de = 1 when the low data rate optimization is enabled, 0 otherwise
  // h = 1 when header is implicit, 0 otherwise
  double de = txParams.lowDataRateOptimizationEnabled ? 1 : 0;
  double h = txParams.headerDisabled ? 1 : 0;
  double crc = txParams.crcEnabled ? 1 : 0;

  // num and den refer to numerator and denominator of the time on air formula
  double num = 8 * pl - 4 * txParams.sf + 28 + 16 * crc - 20 * h;
  double den = (txParams.sf - 2 * de) * txParams.codingRate;
  double payloadSymbNb = 8 + std::max (std::ceil (num / den), double(0));

  // Time to transmit the payload
  double tPayload = payloadSymbNb * tSym;

  NS_LOG_DEBUG ("Time computation: num = " << num << ", den = " << den <<
                ", payloadSymbNb = " << payloadSymbNb << ", tSym = " << tSym);
  NS_LOG_DEBUG ("tPreamble = " << tPreamble);
  NS_LOG_DEBUG ("tPayload = " << tPayload);
  NS_LOG_DEBUG ("Total time = " << tPreamble + tPayload);

  // Compute and return the total packet on-air time
  return Seconds (tPreamble + tPayload);
}

void
LorawanMac::SetSfForDataRate (std::vector<uint8_t> sfForDataRate)
{
  m_sfForDataRate = sfForDataRate;
}

void
LorawanMac::SetBandwidthForDataRate (std::vector<double> bandwidthForDataRate)
{
  m_bandwidthForDataRate = bandwidthForDataRate;
}

void
LorawanMac::SetMaxAppPayloadForDataRate (std::vector<uint32_t> maxAppPayloadForDataRate)
{
  m_maxAppPayloadForDataRate = maxAppPayloadForDataRate;
}

void
LorawanMac::SetTxDbmForTxPower (std::vector<double> txDbmForTxPower)
{
  m_txDbmForTxPower = txDbmForTxPower;
}

void
LorawanMac::SetNPreambleSymbols (int nPreambleSymbols)
{
  m_nPreambleSymbols = nPreambleSymbols;
}

int
LorawanMac::GetNPreambleSymbols (void)
{
  return m_nPreambleSymbols;
}

void
LorawanMac::SetReplyDataRateMatrix (ReplyDataRateMatrix replyDataRateMatrix)
{
  m_replyDataRateMatrix = replyDataRateMatrix;
}
}
