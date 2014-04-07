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

#ifndef SATELLITE_PHY_RX_CARRIER_CONF_H_
#define SATELLITE_PHY_RX_CARRIER_CONF_H_

#include "ns3/object.h"

#include "satellite-channel.h"
#include "satellite-link-results.h"
#include "satellite-channel-estimation-error-container.h"

namespace ns3 {

/**
 * \brief Information of beam users liken UTs and their users.
 * Used e.g. when creating beams.
 *
 */
class SatPhyRxCarrierConf : public Object
{
public:

  /**
   * \param Calculated C/NI
   * \return Calculated Final SINR in linear
   */
  typedef Callback<double, double> SinrCalculatorCallback;

  /**
   * \param channelType     The type of the channel
   * \param carrierId       The id of the carrier
   * \param bandwidthType   The type of the bandwidth
   *
   * \return The bandwidth of the carrier.
   */
  typedef Callback<double, SatEnums::ChannelType_t, uint32_t, SatEnums::CarrierBandwidthType_t > CarrierBandwidthConverter;

  /**
   *  RX mode.
   *
   *  TRANSPARENT   Only beam Id is checked for receiving
   *  NORMAL        Beam Id and valid address (own or broadcast) is checked for receiving
  */
  enum RxMode
  {
    TRANSPARENT, NORMAL
  };

  /**
   * Used error model
   */
  enum ErrorModel
  {
    EM_NONE, EM_CONSTANT, EM_AVI
  };

  /**
   * Used interference model
   */
  enum InterferenceModel
  {
    IF_PER_PACKET, IF_TRACE, IF_CONSTANT
  };

  /**
   * \brief Struct for storing the information for SatPhyRxCarrierConf.
   * \param rxTemperatureK RX noise temperature in Kelvins
   * \param aciIfWrtNoisePercent Adjacent channel interference wrt noise in percents
   * \param errorModel Used error model
   * \param ifModel Used interference model
   * \param rxMode RX mode used in carrier
   * \param chType RX channel type
   * \param converter Bandwidth converter
   * \param carrierCount carrier count
   * \param cec Channel estimation error container
   */
  typedef struct
  {
    double                                  m_rxTemperatureK;
    double                                  m_aciIfWrtNoisePercent;
    ErrorModel                              m_errorModel;
    InterferenceModel                       m_ifModel;
    RxMode                                  m_rxMode;
    SatEnums::ChannelType_t                 m_chType;
    CarrierBandwidthConverter               m_converter;
    uint32_t                                m_carrierCount;
    Ptr<SatChannelEstimationErrorContainer> m_cec;
  } RxCarrierCreateParams_s;

  /**
   * Default constructor for SatPhyRxCarrierConf.
   */
  SatPhyRxCarrierConf (RxCarrierCreateParams_s p);

  /**
   * Constructor for SatPhyRxCarrierConf.
   * \param
   */
  SatPhyRxCarrierConf ( );

  /**
   * Destructor for SatPhyRxCarrierConf.
   */
  virtual ~SatPhyRxCarrierConf () {}

  static TypeId GetTypeId (void);

  /**
   * \brief
   */
  virtual void DoDispose ();

  /**
   * Set link results for the carrier configuration, either
   * - DVB-RCS2
   * - DVB-S2
   */
  void SetLinkResults (Ptr<SatLinkResults> linkResults);

  /**
   * Get the number of configured carriers
   * \return the number of configured carriers
   */
  uint32_t GetCarrierCount () const;

  /**
   * Get configured error model
   * \return configured error model
   */
  ErrorModel GetErrorModel () const;

  /**
   * Get configured interference model
   * \return configured interference model
   */
  InterferenceModel GetInterferenceModel () const;

  /**
   * Get configured link results
   * \return configured link results
   */
  Ptr<SatLinkResults> GetLinkResults () const;

  /**
   * Get configured bandwidth
   *
   * \param carrierId   Id of the carrier
   * \param bandwidthType Type of the bandwidth
   *
   * \return Bandwidth of the requested carrier.
   */
  double GetCarrierBandwidthHz ( uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType ) const;

  /**
   * Get configured RX noise temperature
   * \return configured RX noise temperature
   */
  double GetRxTemperatureK () const;

  /**
   * Get other system RX noise
   * \return other system RX noise
   */
  double GetExtPowerDensityDbwhz () const;

  /**
   * Get adjacent channel interference wrt noise (percent)
   * \return adjacent channel interference wrt noise (percent)
   */
  double GetRxAciInterferenceWrtNoise () const;

  /**
   * Get configured RX mode
   * \return configured RX mode
   */
  RxMode GetRxMode () const;

  /**
   * \brief Function for getting the error rate for constant error model
   * \return error rate for constant error model
   */
  double GetConstantErrorRate () const;

  /**
   * \brief Get channel type
   * \return channel type
   */
  SatEnums::ChannelType_t GetChannelType () const;

  /**
   * \brief Is interference output trace enabled
   * \return true or false
   */
  bool IsIntfOutputTraceEnabled () const;

  /**
   * \brief Get callback function to calculate final SINR.
   */
  inline SinrCalculatorCallback  GetSinrCalculatorCb () {return m_sinrCalculate;}

  /**
   * \brief Set callback function to calculate final SINR.
   */
  inline void SetSinrCalculatorCb (SinrCalculatorCallback sinrCalculator ) {m_sinrCalculate = sinrCalculator;}

  /**
   *
   * \return
   */
  Ptr<SatChannelEstimationErrorContainer> GetChannelEstimatorErrorContainer () const;

  /**
   *
   * \return
   */
  bool AreCollidingRandomAccessPacketsAlwaysDropped () const;

  /**
   *
   * \return
   */
  bool IsRandomAccessEnabledForThisCarrier () const;

  /**
   *
   * \return
   */
  uint32_t GetRandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize () const;

private:

  /*
   * Note, that different carriers may be different bandwidth (symbol rate).
   * This might be one place to have the frame configuration as well, so that
   * we can pass the bandwidth information for each carrier. The bandwidth may be needed
   * for at least noise and C/No calculation.
   */
  InterferenceModel m_ifModel;
  ErrorModel m_errorModel;
  double m_rxTemperatureK;
  double m_rxAciIfWrtNoise;
  RxMode m_rxMode;
  uint32_t m_carrierCount;
  CarrierBandwidthConverter m_carrierBandwidthConverter;
  SatEnums::ChannelType_t m_channelType;
  Ptr<SatChannelEstimationErrorContainer> m_channelEstimationError;
  SinrCalculatorCallback m_sinrCalculate;
  double m_constantErrorRate;
  Ptr<SatLinkResults> m_linkResults;
  double m_rxExtNoiseDensityDbwhz;
  bool m_enableIntfOutputTrace;
  bool m_alwaysDropCollidingRandomAccessPackets;
  bool m_randomAccessEnabledForThisCarrier;
  uint32_t m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize;
};

} // namespace ns3


#endif /* SATELLITE_PHY_RX_CARRIER_CONF_H_ */
