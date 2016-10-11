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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#ifndef SATELLITE_RAYLEIGH_MODEL_H
#define SATELLITE_RAYLEIGH_MODEL_H

#include "ns3/vector.h"
#include "satellite-fading-oscillator.h"
#include "satellite-base-fader.h"
#include "ns3/random-variable-stream.h"
#include "satellite-rayleigh-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for Rayleigh model fader. The class implements Rayleigh
 * model fader for the Markov -based fading model. The model
 * uses multiple oscillators to form the fading (inspired by Jakes model).
 */
class SatRayleighModel : public SatBaseFader
{
public:
  /**
   * \brief NS-3 function for type id
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  SatRayleighModel ();

  /**
   * \brief Constructor
   * \param rayleighConf Rayleigh configuration.
   * \param initialSet Initial set of parameters for which Rayleigh parameters are requested.
   * \param initialState Initial state of the model.
   */
  SatRayleighModel (Ptr<SatRayleighConf> rayleighConf, uint32_t initialSet, uint32_t initialState);

  /**
   * \brief Destructor
   */
  ~SatRayleighModel ();

  /**
   *  \brief Do needed dispose actions.
   */
  void DoDispose ();

  /**
   * \brief Function for calculating the oscillator complex gain
   * \return complex gain
   */
  std::complex<double> GetComplexGain ();

  /**
   * \brief Function for returning the channel gain in dB
   * \return channel gain in dB
   */
  double GetChannelGainDb ();

  /**
   * \brief Function for returning the channel gain
   * \return channel gain
   */
  double GetChannelGain ();

  /**
   * \brief Function for updating the parameter set and state
   * \param set parameter set
   * \param state state
   */
  void UpdateParameters (uint32_t set, uint32_t state);

private:
  /**
   * \brief Function for constructing the oscillators
   */
  void ConstructOscillators ();

  /**
   * \brief Clear used variables
   */
  void Reset ();

  /**
   * \brief Vector of oscillators
   */
  std::vector< Ptr<SatFadingOscillator> > m_oscillators;

  /**
   * \brief Current parameter set
   */
  uint32_t m_currentSet;

  /**
   * \brief Current state
   */
  uint32_t m_currentState;

  /**
   * \brief Uniform distribution random variable
   */
  Ptr<UniformRandomVariable> m_uniformVariable;

  /**
   * \brief Rayleigh configuration object
   */
  Ptr<SatRayleighConf> m_rayleighConf;

  /**
   * \brief Rayleigh model parameters
   */
  std::vector<std::vector<double> > m_rayleighParameters;
};

} // namespace ns3

#endif /* SATELLITE_RAYLEIGH_MODEL_H */
