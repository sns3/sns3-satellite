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
#ifndef SATELLITE_LOO_MODEL_H
#define SATELLITE_LOO_MODEL_H

#include "ns3/vector.h"
#include "satellite-base-fader.h"
#include "satellite-fading-oscillator.h"
#include "satellite-loo-conf.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for Loo's model fader. The class implements Loo's
 * model fader for the Markov -based fading model. The model
 * uses multiple oscillators to form the fading (inspired by Jakes model).
 */
class SatLooModel : public SatBaseFader
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
  SatLooModel ();

  /**
   * \brief Constructor
   * \param looConf Loo's model configuration
   * \param numOfStates number of states
   * \param initialSet initial parameter set
   * \param initialState initial state
   */
  SatLooModel (Ptr<SatLooConf> looConf, uint32_t numOfStates, uint32_t initialSet, uint32_t initialState);

  /**
   * \brief Destructor
   */
  ~SatLooModel ();

  /**
   *  \brief Do needed dispose actions.
   */
  void DoDispose ();

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
   * \brief Number of states
   */
  uint32_t m_numOfStates;

  /**
   * \brief Current parameter set
   */
  uint32_t m_currentSet;

  /**
   * \brief Current state
   */
  uint32_t m_currentState;

  /**
   * \brief Multipath power converted to linear units
   */
  std::vector<double> m_sigma;

  /**
   * \brief Loo's model configuration object
   */
  Ptr<SatLooConf> m_looConf;

  /**
   * \brief Loo's model parameters
   */
  std::vector<std::vector<double> > m_looParameters;

  /**
   * \brief Normal distribution random variable
   */
  Ptr<NormalRandomVariable> m_normalRandomVariable;

  /**
   * \brief Uniform distribution random variable
   */
  Ptr<UniformRandomVariable> m_uniformVariable;

  /**
   * \brief Direct signal oscillators
   */
  std::vector< std::vector< Ptr<SatFadingOscillator> > > m_directSignalOscillators;

  /**
   * \brief Multipath oscillators
   */
  std::vector< std::vector< Ptr<SatFadingOscillator> > > m_multipathOscillators;

  /**
   * \brief Function for constructing direct signal oscillators
   */
  void ConstructDirectSignalOscillators ();

  /**
   * \brief Function for constructing multipath oscillators
   */
  void ConstructMultipathOscillators ();

  /**
   * \brief Function for calculating cosine wave oscillator complex sum
   * \param oscillator oscillator
   * \param timeInSeconds current time in seconds
   * \return sum
   */
  std::complex<double> GetOscillatorCosineWaveSum (std::vector< Ptr<SatFadingOscillator> > oscillator, double timeInSeconds);

  /**
   * \brief Function for calculating oscillator complex sum
   * \param oscillator oscillator
   * \param timeInSeconds current time in seconds
   * \return sum
   */
  std::complex<double> GetOscillatorComplexSum (std::vector< Ptr<SatFadingOscillator> > oscillator, double timeInSeconds);

  /**
   * \brief Function for setting the state
   * \param newState new state
   */
  void ChangeState (uint32_t newState);

  /**
   * \brief Function for setting the parameter set and state
   * \param newSet new set
   * \param newState new state
   */
  void ChangeSet (uint32_t newSet, uint32_t newState);

  /**
   * \brief Function for calculating sigma for different states
   */
  void CalculateSigma ();

  /**
   * \brief Clear used variables
   */
  void Reset ();
};

} // namespace ns3

#endif /* SATELLITE_LOO_MODEL_H */
