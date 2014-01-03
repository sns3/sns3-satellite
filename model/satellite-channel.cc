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
#include "ns3/enum.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac-tag.h"
#include "ns3/singleton.h"
#include "ns3/boolean.h"
#include "satellite-rx-power-output-trace-container.h"
#include "satellite-rx-power-input-trace-container.h"
#include "satellite-fading-output-trace-container.h"

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
   m_rxPowerCalculationMode (SatEnums::RX_PWR_CALCULATION), /// TODO tie this properly to e.g. fading (do not create fading objects!)
   m_enableRxPowerOutputTrace (false),
   m_enableFadingOutputTrace (false)
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
    .AddAttribute( "EnableRxPowerOutputTrace",
                   "Enable Rx power output trace.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatChannel::m_enableRxPowerOutputTrace),
                    MakeBooleanChecker ())
    .AddAttribute( "EnableFadingOutputTrace",
                   "Enable fading output trace.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatChannel::m_enableFadingOutputTrace),
                    MakeBooleanChecker ())
    .AddAttribute ("RxPowerCalculationMode",
                   "Rx Power calculation mode",
                    EnumValue (SatEnums::RX_PWR_CALCULATION),
                    MakeEnumAccessor (&SatChannel::m_rxPowerCalculationMode),
                    MakeEnumChecker (SatEnums::RX_PWR_CALCULATION, "RxPowerCalculation",
                                     SatEnums::RX_PWR_INPUT_TRACE, "RxPowerInputTrace"))
    .AddAttribute ("RxMode",
                   "Channel receiving mode",
                   EnumValue (SatChannel::MULTI_RX),
                   MakeEnumAccessor (&SatChannel::m_rxMode),
                   MakeEnumChecker (SatChannel::SINGLE_RX, "SingleRx",
                                    SatChannel::MULTI_RX, "MultiRx"))
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

  PhyList::iterator phyIter = std::find (m_phyList.begin (), m_phyList.end (), phyRx);

  if (phyIter != m_phyList.end ()) // == vector.end() means the element was not found
    {
      m_phyList.erase (phyIter);
    }
}

void
SatChannel::StartTx (Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this << txParams);
  NS_ASSERT_MSG (txParams->m_phyTx, "NULL phyTx");

  switch (m_rxMode)
  {
    /**
     * The packet shall be received by only the intended receiver. The purpose is
     * to be able to speed-up the simulations. Note, that with SINLGE_RX mode,
     * the PerPacket interference may not be used, since the will be no interference.
    */
    case SatChannel::SINGLE_RX:
      {
        for (PhyList::const_iterator rxPhyIterator = m_phyList.begin ();
            rxPhyIterator != m_phyList.end ();
            ++rxPhyIterator)
          {
            /**
             * Currently SINGLE_RX mode restricts only the transmissions to the
             * intended beam id. When the SatSignalParameters includes the
             * packet target MAC addresses and information whether the PHY
             * transmission contains broadcast data, this may be enhanced to
             * pass data to only the intended receivers within the spot-beam
             * TODO: Add the target MAC address container and a flag indicating
             * whether the PHY transmission contains broadcast data to the
             * SatSignalParameters. Implement the functionality to pass the PHY
             * transmission to only for the proper receivers within the beam.
             */
            if ( (*rxPhyIterator)->GetBeamId() == txParams->m_beamId )
              {
                ScheduleRx (txParams, *rxPhyIterator);
              }
          }
        break;
      }
      /**
       * The packet shall be received by all the receivers in the channel. The
       * intended receiver shall receive the packet, while other receivers in the
       * channel see the packet as co-channel interference. Note, that MULTI_RX mode
       * is needed by the PerPacket interference.
      */
    case SatChannel::MULTI_RX:
      {
        for (PhyList::const_iterator rxPhyIterator = m_phyList.begin ();
            rxPhyIterator != m_phyList.end ();
            ++rxPhyIterator)
          {
            ScheduleRx (txParams, *rxPhyIterator);
          }
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported SatChannel RxMode!");
        break;
      }
  }
}

void
SatChannel::ScheduleRx (Ptr<SatSignalParameters> txParams, Ptr<SatPhyRx> receiver)
{
  NS_LOG_FUNCTION (this << txParams << receiver);

  Time delay = Seconds (0);

  Ptr<MobilityModel> senderMobility = txParams->m_phyTx->GetMobility ();
  Ptr<MobilityModel> receiverMobility = receiver->GetMobility ();

  NS_LOG_LOGIC ("copying signal parameters " << txParams);
  Ptr<SatSignalParameters> rxParams = txParams->Copy ();

  if (m_propagationDelay)
    {
      delay = m_propagationDelay->GetDelay (senderMobility, receiverMobility);

      // TODO: This still needed to check
      // Transmission time is needed to decrease from second link delay
      // to prevent overlapping receiving and in second hand this closer
      // to real receiving time (because sending start already when first bit arrives)
      switch (m_channelType)
      {
        case SatEnums::RETURN_FEEDER_CH:
        case SatEnums::FORWARD_USER_CH:
          {
            if ( delay > txParams->m_duration)
            {
              delay -= txParams->m_duration;
            }
          else
            {
              delay = Seconds (0);
            }
          break;
        }

      default:
        {
          break;
        }
      }

      NS_LOG_LOGIC("Time: " << Simulator::Now ().GetSeconds () << ": setting propagation delay: " << delay);
    }

  Ptr<NetDevice> netDev = receiver->GetDevice ();
  uint32_t dstNode =  netDev->GetNode ()->GetId ();
  Simulator::ScheduleWithContext (dstNode, delay, &SatChannel::StartRx, this, rxParams, receiver);

  // Call the tx anim callback on the channel (check net devices from virtual channel)
  // Note: this is only needed for NetAnim. By default, the NetDevice does not have a channel
  // pointer.
  /*
  Ptr<Channel> ch = netDev->GetChannel();
  m_txrxPointToPoint(txParams->m_packet, ch->GetDevice(0), ch->GetDevice(1), Seconds(0), delay );
   */
}

void
SatChannel::StartRx (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << rxParams << phyRx);

  rxParams->m_channelType = m_channelType;

  double frequency_hz = m_carrierFreqConverter (m_channelType, m_freqId, rxParams->m_carrierId);
  rxParams->m_carrierFreq_hz = frequency_hz;

  switch (m_rxPowerCalculationMode)
  {
    case SatEnums::RX_PWR_CALCULATION:
      {
        DoRxPowerCalculation (rxParams, phyRx);

        if (m_enableRxPowerOutputTrace)
          {
            DoRxPowerOutputTrace (rxParams, phyRx);
          }
        break;
      }
    case SatEnums::RX_PWR_INPUT_TRACE:
      {
        DoRxPowerInputTrace (rxParams, phyRx);
        break;
      }
    default :
      {
        NS_FATAL_ERROR ("SatChannel::StartRx - Invalid Rx power calculation mode");
        break;
      }
  }

  phyRx->StartRx (rxParams);
}

void
SatChannel::DoRxPowerOutputTrace (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx)
{
  std::vector<double> tempVector;
  tempVector.push_back (Now ().GetSeconds ());
  tempVector.push_back (rxParams->m_rxPower_W / rxParams->m_carrierFreq_hz);

  switch (m_channelType)
    {
      case SatEnums::RETURN_FEEDER_CH:
      case SatEnums::FORWARD_USER_CH:
        {
          Singleton<SatRxPowerOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (phyRx->GetDevice ()->GetAddress (), m_channelType), tempVector);
          break;
        }
      case SatEnums::FORWARD_FEEDER_CH:
      case SatEnums::RETURN_USER_CH:
        {
          Singleton<SatRxPowerOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (GetSourceAddress (rxParams), m_channelType), tempVector);
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("SatChannel::DoRxPowerOutputTrace - Invalid channel type");
          break;
        }
    }
}

void
SatChannel::DoRxPowerInputTrace (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx)
{
  switch (m_channelType)
    {
      case SatEnums::RETURN_FEEDER_CH:
      case SatEnums::FORWARD_USER_CH:
        {
          rxParams->m_rxPower_W = rxParams->m_carrierFreq_hz * Singleton<SatRxPowerInputTraceContainer>::Get ()->GetRxPowerDensity (std::make_pair (phyRx->GetDevice ()->GetAddress (), m_channelType));
          break;
        }
      case SatEnums::FORWARD_FEEDER_CH:
      case SatEnums::RETURN_USER_CH:
        {
          rxParams->m_rxPower_W = rxParams->m_carrierFreq_hz * Singleton<SatRxPowerInputTraceContainer>::Get ()->GetRxPowerDensity (std::make_pair (GetSourceAddress (rxParams), m_channelType));
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("SatChannel::DoRxPowerInputTrace - Invalid channel type");
          break;
        }
    }
}

void
SatChannel::DoFadingOutputTrace (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx, double fadingValue)
{
  std::vector<double> tempVector;
  tempVector.push_back (Now ().GetSeconds ());
  tempVector.push_back (fadingValue);

  switch (m_channelType)
    {
      case SatEnums::RETURN_FEEDER_CH:
      case SatEnums::FORWARD_USER_CH:
        {
          Singleton<SatFadingOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (phyRx->GetDevice ()->GetAddress (), m_channelType), tempVector);
          break;
        }
      case SatEnums::FORWARD_FEEDER_CH:
      case SatEnums::RETURN_USER_CH:
        {
          Singleton<SatFadingOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (GetSourceAddress (rxParams), m_channelType), tempVector);
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("SatChannel::DoFadingOutputTrace - Invalid channel type");
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
        fading = phyRx->GetFadingValue (phyRx->GetDevice ()->GetAddress (),m_channelType);
        break;
      }
    case SatEnums::RETURN_USER_CH:
    case SatEnums::FORWARD_FEEDER_CH:
      {
        txAntennaGain_W = rxParams->m_phyTx->GetAntennaGain (txMobility);
        rxAntennaGain_W = phyRx->GetAntennaGain (txMobility);
        fading = rxParams->m_phyTx->GetFadingValue (GetSourceAddress (rxParams),m_channelType);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatChannel::DoRxPowerCalculation - Invalid channel type");
        break;
      }
  }

  if (m_enableFadingOutputTrace)
    {
      DoFadingOutputTrace (rxParams, phyRx, fading);
    }

  // get (calculate) free space loss and RX power and set it to RX params
  double rxPower_W = (rxParams->m_txPower_W * txAntennaGain_W) / m_freeSpaceLoss->GetFsl (txMobility, rxMobility, rxParams->m_carrierFreq_hz);
  rxParams->m_rxPower_W = rxPower_W * rxAntennaGain_W / phyRx->GetLosses () * fading;
}

/// TODO get rid of source MAC address peeking
Mac48Address
SatChannel::GetSourceAddress (Ptr<SatSignalParameters> rxParams)
{
  SatMacTag tag;

  SatSignalParameters::TransmitBuffer_t::const_iterator i = rxParams->m_packetBuffer.begin ();

  if (*i == NULL)
  {
    NS_FATAL_ERROR ("SatChannel::DoFadingOutputTrace - Empty packet list");
  }

  (*i)->PeekPacketTag (tag);

  return Mac48Address::ConvertFrom (tag.GetSourceAddress ());
}

void
SatChannel::SetChannelType (SatEnums::ChannelType_t chType)
{
  NS_LOG_FUNCTION (this << chType);
  NS_ASSERT (chType != SatEnums::UNKNOWN_CH);

  m_channelType = chType;
}

void
SatChannel::SetFrequencyId (uint32_t freqId)
{
  NS_LOG_FUNCTION (this << freqId);

  m_freqId = freqId;
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
