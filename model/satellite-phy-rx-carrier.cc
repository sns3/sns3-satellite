/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#include <algorithm>
#include <ostream>
#include <limits>
#include <utility>

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/boolean.h>
#include <ns3/singleton.h>
#include <ns3/address.h>

#include "satellite-utils.h"
#include "satellite-constant-interference.h"
#include "satellite-per-fragment-interference.h"
#include "satellite-per-packet-interference.h"
#include "satellite-traced-interference.h"
#include "satellite-perfect-interference-elimination.h"
#include "satellite-residual-interference-elimination.h"
#include "satellite-mac-tag.h"
#include "satellite-composite-sinr-output-trace-container.h"
#include "satellite-rtn-link-time.h"
#include "satellite-crdsa-replica-tag.h"
#include "satellite-const-variables.h"
#include "../stats/satellite-phy-rx-carrier-packet-probe.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

#include "satellite-phy-rx-carrier.h"


NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrier");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrier);

SatPhyRxCarrier::SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf, Ptr<SatWaveformConf> waveformConf, bool isRandomAccessEnabled)
  : m_randomAccessEnabled (isRandomAccessEnabled),
  m_state (IDLE),
  m_satId (),
  m_beamId (),
  m_carrierId (carrierId),
  m_receivingDedicatedAccess (false),
  m_satInterference (),
  m_satInterferenceElimination (),
  m_enableCompositeSinrOutputTrace (false),
  m_numOfOngoingRx (0),
  m_rxPacketCounter (0),
  m_waveformConf (waveformConf)
{
  NS_LOG_FUNCTION (this << carrierId);

  m_rxBandwidthHz = carrierConf->GetCarrierBandwidthHz (carrierId, SatEnums::EFFECTIVE_BANDWIDTH);

  // Set channel type
  SetChannelType (carrierConf->GetChannelType ());

  // Set link regeneration mode
  SetLinkRegenerationMode (carrierConf->GetLinkRegenerationMode ());

  // Create proper interference object for carrier i
  DoCreateInterferenceModel (carrierConf, carrierId, m_rxBandwidthHz);
  DoCreateInterferenceEliminationModel (carrierConf, carrierId, waveformConf);

  m_rxExtNoisePowerW = carrierConf->GetExtPowerDensityWhz () * m_rxBandwidthHz;

  m_errorModel = carrierConf->GetErrorModel ();

  if (m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      NS_LOG_INFO (this << " link results in use in carrier: " << carrierId);
      m_linkResults = carrierConf->GetLinkResults ();
    }

  m_rxTemperatureK = carrierConf->GetRxTemperatureK ();

  // calculate RX noise
  m_rxNoisePowerW = SatConstVariables::BOLTZMANN_CONSTANT * m_rxTemperatureK * m_rxBandwidthHz;

  // calculate RX ACI power
  m_rxAciIfPowerW = m_rxNoisePowerW * carrierConf->GetRxAciInterferenceWrtNoiseFactor ();

  m_additionalInterferenceCallback = carrierConf->GetAdditionalInterferenceCb ();

  // Constant error rate for dedicated access.
  m_constantErrorRate = carrierConf->GetConstantDaErrorRate ();

  /**
   * Uniform random variable used for checking whether a packet
   * was received successfully or not
   */
  m_uniformVariable = CreateObject<UniformRandomVariable> ();

  // Configured channel estimation error
  m_channelEstimationError = carrierConf->GetChannelEstimatorErrorContainer ();

  NS_LOG_INFO ("Carrier ID: " << m_carrierId <<
               ", channel type: " << SatEnums::GetChannelTypeName (GetChannelType ()));
}


void
SatPhyRxCarrier::DoCreateInterferenceModel (Ptr<SatPhyRxCarrierConf> carrierConf,
                                            uint32_t carrierId,
                                            double rxBandwidthHz)
{
  switch (carrierConf->GetInterferenceModel (m_randomAccessEnabled))
    {
    case SatPhyRxCarrierConf::IF_CONSTANT:
      {
        NS_LOG_INFO (this << " Constant interference model created for carrier: " << carrierId);
        m_satInterference = CreateObject<SatConstantInterference> ();
        break;
      }
    case SatPhyRxCarrierConf::IF_PER_PACKET:
      {
        NS_LOG_INFO (this << " Per packet interference model created for carrier: " << carrierId);
        if (carrierConf->IsIntfOutputTraceEnabled ())
          {
            m_satInterference = CreateObject<SatPerPacketInterference> (GetChannelType (), rxBandwidthHz);
          }
        else
          {
            m_satInterference = CreateObject<SatPerPacketInterference> ();
          }
        break;
      }
    case SatPhyRxCarrierConf::IF_PER_FRAGMENT:
      {
        NS_LOG_INFO (this << " Per fragment interference model created for carrier: " << carrierId);
        if (carrierConf->IsIntfOutputTraceEnabled ())
          {
            m_satInterference = CreateObject<SatPerFragmentInterference> (GetChannelType (), rxBandwidthHz);
          }
        else
          {
            m_satInterference = CreateObject<SatPerFragmentInterference> ();
          }
        break;
      }
    case SatPhyRxCarrierConf::IF_TRACE:
      {
        NS_LOG_INFO (this << " Traced interference model created for carrier: " << carrierId);
        m_satInterference = CreateObject<SatTracedInterference> (GetChannelType (), rxBandwidthHz);
        break;
      }
    default:
      {
        NS_LOG_ERROR (this << " Not a valid interference model!");
        break;
      }
    }
}


void
SatPhyRxCarrier::DoCreateInterferenceEliminationModel (
  Ptr<SatPhyRxCarrierConf> carrierConf,
  uint32_t carrierId,
  Ptr<SatWaveformConf> waveformConf)
{
  switch (carrierConf->GetInterferenceEliminationModel (m_randomAccessEnabled))
    {
    case SatPhyRxCarrierConf::SIC_PERFECT:
      {
        NS_LOG_INFO (this << " Perfect interference elimination model created for carrier: " << carrierId);
        m_satInterferenceElimination = CreateObject<SatPerfectInterferenceElimination> ();
        break;
      }
    case SatPhyRxCarrierConf::SIC_RESIDUAL:
      {
        NS_LOG_INFO (this << " Residual interference elimination model created for carrier: " << carrierId);
        m_satInterferenceElimination = CreateObject<SatResidualInterferenceElimination> (waveformConf);
        break;
      }
    default:
      {
        NS_LOG_ERROR (this << " Not a valid interference elimination model!");
        break;
      }
    }
}


SatPhyRxCarrier::~SatPhyRxCarrier ()
{
  NS_LOG_FUNCTION (this);
}

void
SatPhyRxCarrier::BeginEndScheduling (void)
{
}

TypeId
SatPhyRxCarrier::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrier")
    .SetParent<Object> ()
    .AddAttribute ( "EnableCompositeSinrOutputTrace",
                    "Enable composite SINR output trace.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatPhyRxCarrier::m_enableCompositeSinrOutputTrace),
                    MakeBooleanChecker ())
    .AddTraceSource ("LinkBudgetTrace",
                     "The trace for link budget related quantities",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_linkBudgetTrace),
                     "ns3::SatPhyRxCarrier::LinkBudgetTraceCallback")
    .AddTraceSource ("RxPowerTrace",
                     "The trace for received signal power in dBW",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_rxPowerTrace),
                     "ns3::SatPhyRxCarrier::RxPowerTraceCallback")
    .AddTraceSource ("Sinr",
                     "The trace for composite SINR in dB",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_sinrTrace),
                     "ns3::SatPhyRxCarrier::SinrTraceCallback")
    .AddTraceSource ("LinkSinr",
                     "The trace for link specific SINR in dB",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_linkSinrTrace),
                     "ns3::SatPhyRxCarrier::LinkSinrTraceCallback")
    .AddTraceSource ("DaRx",
                     "Received a packet burst through Dedicated Channel",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_daRxTrace),
                     "ns3::SatPhyRxCarrierPacketProbe::RxStatusCallback")
    .AddTraceSource ("DaRxCarrierId",
                     "Received a packet burst though DAMA",
                     MakeTraceSourceAccessor (&SatPhyRxCarrier::m_daRxCarrierIdTrace),
                     "ns3::SatTypedefs::DataSenderAddressCallback")
  ;
  return tid;
}


void
SatPhyRxCarrier::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_rxCallback.Nullify ();
  m_cnoCallback.Nullify ();
  m_additionalInterferenceCallback.Nullify ();
  m_avgNormalizedOfferedLoadCallback.Nullify ();
  m_satInterference = NULL;
  m_satInterferenceElimination = NULL;
  m_uniformVariable = NULL;

  Object::DoDispose ();
}


std::ostream& operator<< (std::ostream& os, SatPhyRxCarrier::State s)
{
  switch (s)
    {
    case SatPhyRxCarrier::IDLE:
      os << "IDLE";
      break;
    case SatPhyRxCarrier::RX:
      os << "RX";
      break;
    default:
      os << "UNKNOWN";
      break;
    }
  return os;
}

void
SatPhyRxCarrier::SetReceiveCb (SatPhyRx::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_rxCallback = cb;
}

void
SatPhyRxCarrier::SetCnoCb (SatPhyRx::CnoCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_cnoCallback = cb;
}

void
SatPhyRxCarrier::ChangeState (State newState)
{
  NS_LOG_FUNCTION (this << newState);
  NS_LOG_INFO (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}


std::pair<bool, SatPhyRxCarrier::rxParams_s>
SatPhyRxCarrier::GetReceiveParams (Ptr<SatSignalParameters> rxParams)
{
  SatPhyRxCarrier::rxParams_s params;
  params.rxParams = rxParams;
  // Receive packet by default in satellite, discard in UT
  bool receivePacket = GetDefaultReceiveMode ();
  bool ownAddressFound = false;

  // If satellite and regeneration_phy -> do not check MAC address, but store it for stat purposes
  if ((rxParams->m_channelType == SatEnums::FORWARD_FEEDER_CH
    || rxParams->m_channelType == SatEnums::RETURN_USER_CH)
    && m_linkRegenerationMode == SatEnums::REGENERATION_PHY)
    {
      if (!rxParams->m_packetsInBurst.empty ())
        {
          SatMacTag mTag;
          rxParams->m_packetsInBurst[0]->PeekPacketTag (mTag);
          SatAddressE2ETag addressE2ETag;
          rxParams->m_packetsInBurst[0]->PeekPacketTag (addressE2ETag);

          params.destAddress = mTag.GetDestAddress ();
          params.sourceAddress = mTag.GetSourceAddress ();
          params.finalDestAddress = addressE2ETag.GetE2EDestAddress ();
          params.finalSourceAddress = addressE2ETag.GetE2ESourceAddress ();
        }
      receivePacket = true;
    }
  else
    {
      for (SatSignalParameters::PacketsInBurst_t::const_iterator i = rxParams->m_packetsInBurst.begin ();
           ((i != rxParams->m_packetsInBurst.end ()) && (ownAddressFound == false) ); i++)
        {
          SatMacTag mTag;
          (*i)->PeekPacketTag (mTag);
          SatAddressE2ETag addressE2ETag;
          (*i)->PeekPacketTag (addressE2ETag);

          params.destAddress = mTag.GetDestAddress ();
          params.sourceAddress = mTag.GetSourceAddress ();
          params.finalDestAddress = addressE2ETag.GetE2EDestAddress ();
          params.finalSourceAddress = addressE2ETag.GetE2ESourceAddress ();

          if (( params.destAddress == GetOwnAddress () ))
            {
              NS_LOG_INFO ("Packet intended for this specific receiver: " << params.destAddress);

              receivePacket = true;
              ownAddressFound = true;
            }
          else if ( params.destAddress.IsBroadcast () )
            {
              NS_LOG_INFO ("Destination is broadcast address: " << params.destAddress);
              receivePacket = true;
            }
          else if ( params.destAddress.IsGroup () )
            {
              NS_LOG_INFO ("Destination is multicast address: " << params.destAddress);
              receivePacket = true;
            }
        }
    }
  return std::make_pair (receivePacket, params);
}


bool
SatPhyRxCarrier::StartRx (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);
  NS_LOG_INFO ("State: " << m_state);
  NS_ASSERT (rxParams->m_carrierId == m_carrierId);

  uint32_t key;

  NS_LOG_INFO ("Node: " << m_nodeInfo->GetMacAddress ()
                        << " starts receiving packet at: " << Simulator::Now ().GetSeconds ()
                        << " in carrier: " << rxParams->m_carrierId);
  NS_LOG_INFO ("Sender: " << rxParams->m_phyTx);

  switch (m_state)
    {
    case IDLE:
    case RX:
      {
        auto receiveParamTuple = GetReceiveParams (rxParams);

        bool receivePacket = receiveParamTuple.first;
        rxParams_s rxParamsStruct = receiveParamTuple.second;

        // add interference in any case
        rxParamsStruct.interferenceEvent = CreateInterference (rxParams, rxParamsStruct.sourceAddress);

        // Check whether the packet is sent to our beam.
        // In case that RX mode is something else than transparent
        // additionally check that whether the packet was intended for this specific receiver

        bool isGoodBeam = (rxParams->m_beamId == GetBeamId ());
        if ((GetChannelType () == SatEnums::RETURN_FEEDER_CH)
            && (m_linkRegenerationMode == SatEnums::REGENERATION_LINK || m_linkRegenerationMode == SatEnums::REGENERATION_NETWORK))
          {
            isGoodBeam = true;
          }

        if ( receivePacket && isGoodBeam )
          {
            if (IsReceivingDedicatedAccess () && rxParams->m_txInfo.packetType == SatEnums::PACKET_TYPE_DEDICATED_ACCESS)
              {
                NS_FATAL_ERROR ("Starting reception of a packet when receiving DA transmission! This may be due to a clock drift in UTs too important, or an update period for SGP4 too important.");
              }

            GetInterferenceModel ()->NotifyRxStart (rxParamsStruct.interferenceEvent);

            key = m_rxPacketCounter;
            m_rxPacketCounter++;

            StoreRxParams (key, rxParamsStruct);

            NS_LOG_INFO (this << " scheduling EndRx with delay " << rxParams->m_duration.GetSeconds () << "s");

            // Update link specific received signal power
            if (GetChannelType () == SatEnums::FORWARD_FEEDER_CH || GetChannelType () == SatEnums::FORWARD_USER_CH)
              {
                m_rxPowerTrace (SatUtils::LinearToDb (rxParams->m_rxPower_W), rxParamsStruct.finalDestAddress);
              }
            else if (GetChannelType () == SatEnums::RETURN_FEEDER_CH || GetChannelType () == SatEnums::RETURN_USER_CH)
              {
                m_rxPowerTrace (SatUtils::LinearToDb (rxParams->m_rxPower_W), rxParamsStruct.finalSourceAddress);
              }
            else
              {
                NS_FATAL_ERROR ("Incorrect channel for satPhyRxCarrierUplink: " << SatEnums::GetChannelTypeName (GetChannelType ()));
              }

            Simulator::Schedule (rxParams->m_duration, &SatPhyRxCarrier::EndRxData, this, key);

            IncreaseNumOfRxState (rxParams->m_txInfo.packetType);

            return true;
          }
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatPhyRxCarrier::StartRx - Unknown state");
        break;
      }
    }

  return false;
}


void
SatPhyRxCarrier::DoCompositeSinrOutputTrace (double cSinr)
{
  NS_LOG_FUNCTION (this);

  std::vector<double> tempVector;
  tempVector.push_back (Now ().GetSeconds ());
  tempVector.push_back (cSinr);

  Singleton<SatCompositeSinrOutputTraceContainer>::Get ()->AddToContainer (std::make_pair (GetOwnAddress (), GetChannelType ()), tempVector);
}


bool
SatPhyRxCarrier::CheckAgainstLinkResults (double cSinr, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);

  /// Initialize with no errors
  bool error = false;

  switch (m_errorModel)
    {
    case SatPhyRxCarrierConf::EM_AVI:
      {
        error = CheckAgainstLinkResultsErrorModelAvi (cSinr, rxParams);
        break;
      }
    case SatPhyRxCarrierConf::EM_CONSTANT:
      {
        double r = GetUniformRandomValue (0, 1);
        if (r <  m_constantErrorRate)
          {
            error = true;
          }
        break;
      }
    case SatPhyRxCarrierConf::EM_NONE:
      {
        /// No errors i.e. error = false;
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatPhyRxCarrier::EndRxData - Error model not defined");
        break;
      }
    }
  return error;
}


bool
SatPhyRxCarrier::CheckAgainstLinkResultsErrorModelAvi (double cSinr, Ptr<SatSignalParameters> rxParams)
{
  bool error = false;

  switch (GetChannelType ())
    {
    case SatEnums::FORWARD_FEEDER_CH:
    case SatEnums::FORWARD_USER_CH:
      {
        /**
         * In forward link the link results are in Es/No format, thus here we need
         * to convert the SINR into Es/No:
         *
         * C/No = C/N * fs
         * Es/No = C/N * B/fs = (C/No / fs) * B/fs
         * Es/No = (C*Ts)/No = C/No * (1/fs) = C/N
         *
         * where
         * C/No = carrier to noise density
         * C/N = signal to noise ratio (= SINR in the simulator)
         * Es/No = energy per symbol per noise density
         * B = channel bandwidth in Hz
         * fs = symbol rate in baud
        */

        double ber = (GetLinkResults ()->GetObject <SatLinkResultsFwd> ())->GetBler (rxParams->m_txInfo.modCod,
                                                                                     rxParams->m_txInfo.frameType,
                                                                                     SatUtils::LinearToDb (cSinr));
        double r = GetUniformRandomValue (0, 1);

        if ( r < ber )
          {
            error = true;
          }

        NS_LOG_INFO ("FORWARD cSinr (dB): " << SatUtils::LinearToDb (cSinr)
                                            << " esNo (dB): " << SatUtils::LinearToDb (cSinr)
                                            << " rand: " << r
                                            << " ber: " << ber
                                            << " error: " << error);
        break;
      }

    case SatEnums::RETURN_FEEDER_CH:
    case SatEnums::RETURN_USER_CH:
      {
        /**
         * In return link the link results are in Eb/No format, thus here we need
         * to convert the SINR into Eb/No:
         * Eb/No = C/N * B/fb = (C/No / fs) * B/fb
         * Eb/No = (Es/log2M)/No = (Es/No)*(1/log2M)  = C/N * (1/log2M) = C/No * (1/fs) * (1/log2M)
         *
         * where
         * C/No = carrier to noise density
         * C/N = signal to noise ratio (= SINR in the simulator)
         * Es/No = energy per symbol per noise density
         * Eb/No = energy per bit per noise density
         * B = channel bandwidth in Hz
         * fs = symbol rate in baud
         * fb = channel bitrate (after FEC) in bps (i.e. burst payloadInBits / burstDurationInSec)
        */

        double ebNo = cSinr / (SatUtils::GetCodingRate (rxParams->m_txInfo.modCod) *
                               SatUtils::GetModulatedBits (rxParams->m_txInfo.modCod));

        double ber;
        if ((m_linkRegenerationMode == SatEnums::REGENERATION_LINK || m_linkRegenerationMode == SatEnums::REGENERATION_NETWORK)
             && GetChannelType () == SatEnums::RETURN_FEEDER_CH)
          {
            ber = (GetLinkResults ()->GetObject <SatLinkResultsFwd> ())->GetBler (rxParams->m_txInfo.modCod,
                                                                                  rxParams->m_txInfo.frameType,
                                                                                  SatUtils::LinearToDb (cSinr));
          }
        else
          {
            ber = (GetLinkResults ()->GetObject <SatLinkResultsRtn> ())->GetBler (rxParams->m_txInfo.waveformId,
                                                                                           SatUtils::LinearToDb (ebNo));
          }
        double r = GetUniformRandomValue (0, 1);

        if ( r < ber )
          {
            error = true;
          }

        NS_LOG_INFO ("RETURN cSinr (dB): " << SatUtils::LinearToDb (cSinr)
                                           << " ebNo (dB): " << SatUtils::LinearToDb (ebNo)
                                           << " modulated bits: " << SatUtils::GetModulatedBits (rxParams->m_txInfo.modCod)
                                           << " rand: " << r
                                           << " ber: " << ber
                                           << " error: " << error);
        break;
      }
    case SatEnums::UNKNOWN_CH:
    default:
      {
        NS_FATAL_ERROR ("SatPhyRxCarrier::CheckAgainstLinkResultsErrorModelAvi - Invalid channel type!");
        break;
      }

    }
  return error;
}


void
SatPhyRxCarrier::SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);

  m_ownAddress = nodeInfo->GetMacAddress ();
  m_nodeInfo = nodeInfo;
}

double
SatPhyRxCarrier::CalculateSinr (double rxPowerW,
                                double ifPowerW,
                                double rxNoisePowerW,
                                double rxAciIfPowerW,
                                double rxExtNoisePowerW,
                                double additionalInterference)
{
  NS_LOG_FUNCTION (this << rxPowerW <<  ifPowerW);

  if (rxNoisePowerW <= 0.0)
    {
      NS_FATAL_ERROR ("Noise power must be greater than zero!!! Current value is " << rxNoisePowerW);
    }

  // Calculate first SINR based on co-channel interference, Adjacent channel interference, noise and external noise
  // NOTE! ACI noise power and Ext noise power are set 0 by default and given as attributes by PHY object when used.
  double sinr = rxPowerW / (ifPowerW +  rxNoisePowerW + rxAciIfPowerW + rxExtNoisePowerW);

  // Call PHY calculator to composite C over I interference configured to PHY.
  double finalSinr = (1 / ( 1/sinr + 1/additionalInterference ));

  return finalSinr;
}

double
SatPhyRxCarrier::CalculateSinr (double sinr, double otherInterference)
{
  NS_LOG_FUNCTION (this << sinr << otherInterference);

  if ( sinr <= 0  )
    {
      NS_FATAL_ERROR ( "Calculated own SINR is expected to be greater than zero!!!");
    }

  if ( otherInterference <= 0  )
    {
      NS_FATAL_ERROR ( "Interference is expected to be greater than zero!!!");
    }

  double finalSinr = 1 / ( (1 / sinr) + (1 / otherInterference) );

  return finalSinr;
}

double
SatPhyRxCarrier::CalculateCompositeSinr (double sinr1, double sinr2)
{
  NS_LOG_FUNCTION (this << sinr1 << sinr2 );

  if (sinr1 <= 0.0)
    {
      NS_FATAL_ERROR ("SINR 1 must be greater than zero!!!");
    }

  if (sinr2 <= 0.0)
    {
      NS_FATAL_ERROR ("SINR 2 must be greater than zero!!!");
    }

  return 1.0 / ( (1.0 / sinr1) + (1.0 / sinr2) );
}

double
SatPhyRxCarrier::GetWorstSinr (double sinr1, double sinr2)
{
  NS_LOG_FUNCTION (this << sinr1 << sinr2 );

  if (sinr1 <= 0.0)
    {
      NS_FATAL_ERROR ("SINR 1 must be greater than zero!!! Current value is " << sinr1);
    }

  if (sinr2 <= 0.0)
    {
      NS_FATAL_ERROR ("SINR 2 must be greater than zero!!! Current value is " << sinr2);
    }

  return std::min (sinr1, sinr2);
}

void
SatPhyRxCarrier::IncreaseNumOfRxState (SatEnums::PacketType_t packetType)
{
  NS_LOG_FUNCTION (this);

  m_numOfOngoingRx++;
  ChangeState (RX);

  if (packetType == SatEnums::PACKET_TYPE_DEDICATED_ACCESS)
    {
      m_receivingDedicatedAccess = true;
    }

  CheckRxStateSanity ();
}

void
SatPhyRxCarrier::DecreaseNumOfRxState (SatEnums::PacketType_t packetType)
{
  NS_LOG_FUNCTION (this);

  if (m_numOfOngoingRx > 0)
    {
      m_numOfOngoingRx--;
    }

  if (m_numOfOngoingRx < 1)
    {
      ChangeState (IDLE);
    }

  if (packetType == SatEnums::PACKET_TYPE_DEDICATED_ACCESS)
    {
      m_receivingDedicatedAccess = false;
    }

  CheckRxStateSanity ();
}

void
SatPhyRxCarrier::CheckRxStateSanity ()
{
  NS_LOG_FUNCTION (this);

  if (m_numOfOngoingRx > 0 && m_state == IDLE)
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::CheckStateSanity - State mismatch");
    }

  if (m_numOfOngoingRx < 1 && m_state == RX)
    {
      NS_FATAL_ERROR ("SatPhyRxCarrier::CheckStateSanity - State mismatch");
    }
}


void
SatPhyRxCarrier::SetAverageNormalizedOfferedLoadCallback (SatPhyRx::AverageNormalizedOfferedLoadCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);

  m_avgNormalizedOfferedLoadCallback = callback;
}


}
