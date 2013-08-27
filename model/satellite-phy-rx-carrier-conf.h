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

#include "satellite-link-results.h"

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
   * Default constructor for SatPhyRxCarrierConf.
   */
  SatPhyRxCarrierConf ();

  /**
   * Constructor for SatPhyRxCarrierConf.
   * \param numCarriers Number of configured carriers
   * \param rxTemperature_K RX noise temperature in Kelvins
   * \param rxBandwidth_Hz RX bandwidth in Hz
   * \param ifModel Used interference model
   */
  SatPhyRxCarrierConf ( uint32_t numCarriers, double rxTemperature_K,
                        double rxBandwidth_Hz, ErrorModel errorModel, InterferenceModel ifModel);

  /**
   * Destructor for SatPhyRxCarrierConf.
   */
  virtual ~SatPhyRxCarrierConf () {}

  static TypeId GetTypeId (void);

  /**
   * Set link results for the carrier configuration, either
   * - DVB-RCS2
   * - DVB-S2
   */
  void SetLinkResults (Ptr<SatLinkResults> linkResults);

  /*
   * Get the number of configured carriers
   */
  uint32_t GetCarriersN () const;

  /*
   * Get configured error model
   */
  ErrorModel GetErrorModel () const;

  /*
   * Get configured interference model
   */
  InterferenceModel GetInterferenceModel () const;

  /*
   * Get configured link results
   */
  Ptr<SatLinkResults> GetLinkResults () const;

  /*
   * Get configured bandwidth
   */
  double GetBandwidth_Hz () const;

  /*
   * Get configured RX noise temperature
   */
  double GetRxTemperature_K () const;

private:

  /*
   * Note, that different carriers may be different bandwidth (symbol rate).
   * This might be one place to have the frame configuration as well, so that
   * we can pass the bandwidth information for each carrier. The bandwidth may be needed
   * for at least noise and C/No calculation.
   */
  uint32_t m_numCarriers;
  InterferenceModel m_ifModel;
  ErrorModel m_errorModel;
  Ptr<SatLinkResults> m_linkResults;
  double m_rxTemperature_K;
  double m_rxBandwidth_Hz;

 };

} // namespace ns3


#endif /* SATELLITE_PHY_RX_CARRIER_CONF_H_ */
