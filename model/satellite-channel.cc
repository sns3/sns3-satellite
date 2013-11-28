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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <algorithm>
#include "ns3/object.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/mobility-model.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac-tag.h" /// TODO do not introduce the whole class
#include "ns3/satellite-helper.h" /// TODO do not introduce the whole class

NS_LOG_COMPONENT_DEFINE ("SatChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatChannel);

SatChannel::SatChannel ()
 : m_phyList (),
   m_channelType (SatEnums::UNKNOWN_CH),
   m_carrierFreqConverter (),
   m_freqId (),
   m_propagationDelay (),
   m_freeSpaceLoss (),
   m_rxPowerCalculationMode (SatEnums::RX_PWR_CALCULATION), /// TODO add as an attribute
   m_enableRxPowerOutputTrace (false) /// TODO add as an attribute
{
  NS_LOG_FUNCTION (this);
}

SatChannel::~SatChannel ()
{
  NS_LOG_FUNCTION (this);
}

void
SatChannel::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phyList.clear ();
  m_propagationDelay = 0;
  Channel::DoDispose ();
}

TypeId
SatChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatChannel")
    .SetParent<Channel> ()
    .AddConstructor<SatChannel> ()
    .AddTraceSource ("TxRxPointToPoint",
                     "Trace source indicating transmission of packet from the SatChannel, used by the Animation interface.",
                     MakeTraceSourceAccessor (&SatChannel::m_txrxPointToPoint));
  return tid;
}

void
SatChannel::AddRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);
  m_phyList.push_back (phyRx);
}

void
SatChannel::RemoveRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);

  PhyList::iterator phyIter = std::find(m_phyList.begin(), m_phyList.end(), phyRx);

  if (phyIter != m_phyList.end()) // == vector.end() means the element was not found
    {
      m_phyList.erase (phyIter);
    }
}

void
SatChannel::StartTx (Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this << txParams);
  NS_ASSERT_MSG (txParams->m_phyTx, "NULL phyTx");

  Ptr<MobilityModel> senderMobility = txParams->m_phyTx->GetMobility ();

  for (PhyList::const_iterator rxPhyIterator = m_phyList.begin ();
       rxPhyIterator != m_phyList.end ();
       ++rxPhyIterator)
    {
      Time delay = Seconds (0);

      Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility ();
      NS_LOG_LOGIC ("copying signal parameters " << txParams);
      Ptr<SatSignalParameters> rxParams = txParams->Copy ();

      if (m_propagationDelay)
        {
          delay = m_propagationDelay->GetDelay (senderMobility, receiverMobility);
          NS_LOG_LOGIC("Time: " << Simulator::Now ().GetSeconds () << ": setting propagation delay: " << delay);
        }

      Ptr<NetDevice> netDev = (*rxPhyIterator)->GetDevice ();
      uint32_t dstNode =  netDev->GetNode ()->GetId ();
      Simulator::ScheduleWithContext (dstNode, delay, &SatChannel::StartRx, this, rxParams, *rxPhyIterator);

      // Call the tx anim callback on the channel (check net devices from virtual channel)
      // Note: this is only needed for NetAnim. By default, the NetDevice does not have a channel
      // pointer.
      /*
      Ptr<Channel> ch = netDev->GetChannel();
      m_txrxPointToPoint(txParams->m_packet, ch->GetDevice(0), ch->GetDevice(1), Seconds(0), delay );
      */
    }
}

void
SatChannel::StartRx (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << rxParams << phyRx);

  rxParams->m_channel = this;

  double frequency_hz = m_carrierFreqConverter (m_channelType, m_freqId, rxParams->m_carrierId);
  rxParams->m_carrierFreq_hz = frequency_hz;

  switch (m_rxPowerCalculationMode)
  {
    case SatEnums::RX_PWR_CALCULATION:
      {
        DoRxPowerCalculation (rxParams, phyRx);

        if (m_enableRxPowerOutputTrace)
          {
            DoRxPowerOutputTrace (rxParams);
          }
        break;
      }
    case SatEnums::RX_PWR_INPUT_TRACE:
      {
        DoRxPowerInputTrace (rxParams);
        break;
      }
    default :
      {
        NS_FATAL_ERROR ("SatChannel::StartRx - Rx power calculation mode");
        break;
      }
  }

  phyRx->StartRx (rxParams);
}

void
SatChannel::DoRxPowerOutputTrace (Ptr<SatSignalParameters> rxParams)
{
  SatMacTag tag;

  /// TODO get rid of peeking
  rxParams->m_packet->PeekPacketTag (tag);

  Mac48Address destAddress = Mac48Address::ConvertFrom (tag.GetDestAddress ());
  Mac48Address sourceAddress = Mac48Address::ConvertFrom (tag.GetSourceAddress ());

  std::vector<double> tempVector;
  tempVector.push_back (Now ().GetDouble());
  tempVector.push_back (rxParams->m_rxPower_W / rxParams->m_carrierFreq_hz);

  switch (m_channelType)
  {
    case SatEnums::FORWARD_FEEDER_CH:
    case SatEnums::FORWARD_USER_CH:
      {
        SatHelper::m_satRxPowerOutputTraceContainer->AddToContainer (std::make_pair (destAddress, m_channelType), tempVector);
        break;
      }
    case SatEnums::RETURN_FEEDER_CH:
    case SatEnums::RETURN_USER_CH:
      {
        SatHelper::m_satRxPowerOutputTraceContainer->AddToContainer (std::make_pair (sourceAddress, m_channelType), tempVector);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatChannel::StartRx - Invalid channel type");
        break;
      }
  }
}

void
SatChannel::DoRxPowerInputTrace (Ptr<SatSignalParameters> rxParams)
{
  SatMacTag tag;

  /// TODO get rid of peeking
  rxParams->m_packet->PeekPacketTag (tag);

  Mac48Address destAddress = Mac48Address::ConvertFrom (tag.GetDestAddress ());
  Mac48Address sourceAddress = Mac48Address::ConvertFrom (tag.GetSourceAddress ());

  switch (m_channelType)
  {
    case SatEnums::FORWARD_FEEDER_CH:
    case SatEnums::FORWARD_USER_CH:
      {
        rxParams->m_rxPower_W = rxParams->m_carrierFreq_hz * SatHelper::m_satRxPowerInputTraceContainer->GetRxPowerDensity (std::make_pair (destAddress, m_channelType));
        break;
      }
    case SatEnums::RETURN_FEEDER_CH:
    case SatEnums::RETURN_USER_CH:
      {
        rxParams->m_rxPower_W = rxParams->m_carrierFreq_hz * SatHelper::m_satRxPowerInputTraceContainer->GetRxPowerDensity (std::make_pair (sourceAddress, m_channelType));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatChannel::StartRx - Invalid channel type");
        break;
      }
  }
}

void
SatChannel::DoRxPowerCalculation (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx)
{
  Ptr<MobilityModel> txMobility = rxParams->m_phyTx->GetMobility ();
  Ptr<MobilityModel> rxMobility = phyRx->GetMobility ();

  double txAntennaGain_W = 0.0;
  double rxAntennaGain_W = 0.0;
  double fading = 0.0;

  // use always UT's or GW's position when getting antenna gain
  switch (m_channelType)
  {
    case SatEnums::RETURN_FEEDER_CH:
    case SatEnums::FORWARD_USER_CH:
      {
        txAntennaGain_W = rxParams->m_phyTx->GetAntennaGain (rxMobility);
        rxAntennaGain_W = phyRx->GetAntennaGain (rxMobility);
        fading = phyRx->GetFadingValue (m_channelType);
        break;
      }
    case SatEnums::RETURN_USER_CH:
    case SatEnums::FORWARD_FEEDER_CH:
      {
        txAntennaGain_W = rxParams->m_phyTx->GetAntennaGain (txMobility);
        rxAntennaGain_W = phyRx->GetAntennaGain (txMobility);
        fading = rxParams->m_phyTx->GetFadingValue (m_channelType);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatChannel::StartRx - Invalid channel type");
        break;
      }
  }

  // get (calculate) free space loss and RX power and set it to RX params
  double rxPower_W = (rxParams->m_txPower_W * txAntennaGain_W) / m_freeSpaceLoss->GetFsl (txMobility, rxMobility, rxParams->m_carrierFreq_hz);
  rxParams->m_rxPower_W = rxPower_W * rxAntennaGain_W / phyRx->GetLosses () * fading;
}

void
SatChannel::SetChannelType (SatEnums::ChannelType_t chType)
{
  NS_LOG_FUNCTION (this << chType);
  NS_ASSERT (chType != SatEnums::UNKNOWN_CH);

  m_channelType = chType;
}

void
SatChannel::SetFrequencyId (uint32_t fregId)
{
  NS_LOG_FUNCTION (this << fregId);

  m_freqId = fregId;
}

void
SatChannel::SetFrequencyConverter (CarrierFreqConverter converter)
{
  NS_LOG_FUNCTION (this << &converter);

  m_carrierFreqConverter = converter;
}

SatEnums::ChannelType_t
SatChannel::GetChannelType ()
{
  NS_LOG_FUNCTION (this);

  return m_channelType;
}

void
SatChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  NS_LOG_FUNCTION (this << delay);
  NS_ASSERT (m_propagationDelay == 0);
  m_propagationDelay = delay;
}

Ptr<PropagationDelayModel>
SatChannel::GetPropagationDelayModel ()
{
  NS_LOG_FUNCTION (this);

  return m_propagationDelay;
}

void
SatChannel::SetFreeSpaceLoss (Ptr<SatFreeSpaceLoss> loss)
{
  NS_LOG_FUNCTION (this << loss);
  NS_ASSERT (m_freeSpaceLoss == 0);
  m_freeSpaceLoss = loss;
}

uint32_t
SatChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyList.size ();
}

Ptr<NetDevice>
SatChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_phyList.at (i)->GetDevice ()->GetObject<NetDevice> ();
}

} // namespace ns3
