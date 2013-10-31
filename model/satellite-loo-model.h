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
#include "satellite-fader.h"
#include "satellite-fading-oscillator.h"
#include "satellite-loo-conf.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Loo model
 */
class SatLooModel : public SatFader
{
public:
  static const double PI;

  /**
   *
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   *
   */
  SatLooModel ();

  /**
   *
   * \param looConf
   * \param numOfStates
   * \param initialSet
   * \param initialState
   */
  SatLooModel (Ptr<SatLooConf> looConf, uint32_t numOfStates, uint32_t initialSet, uint32_t initialState);

  /**
   *
   */
  ~SatLooModel ();

  /**
   *
   * \return
   */
  double GetChannelGainDb ();

  /**
   *
   * \return
   */
  double GetChannelGain ();

  /**
   *
   * \param set
   * \param state
   */
  void UpdateParameters (uint32_t set, uint32_t state);

private:
  uint32_t m_numOfStates;              // number of Markov states
  uint32_t m_currentSet;               // current set
  uint32_t m_currentState;             // current state
  double m_sigma;                      // multipath power converted to linear units

  Ptr<SatLooConf> m_looConf;
  std::vector<std::vector<double> > m_looParameters;

  /**
   *
   */
  Ptr<NormalRandomVariable> m_normalRandomVariable;
  Ptr<UniformRandomVariable> m_uniformVariable;

  /**
   *
   */
  std::vector< std::vector< Ptr<SatFadingOscillator> > > m_slowFadingOscillators;
  std::vector< std::vector< Ptr<SatFadingOscillator> > > m_fastFadingOscillators;

  /**
   *
   */
  void ConstructSlowFadingOscillators ();

  /**
   *
   */
  void ConstructFastFadingOscillators ();

  /**
   *
   * \param oscillator
   * \return
   */
  std::complex<double> GetOscillatorCosineWaveSum (std::vector< Ptr<SatFadingOscillator> > oscillator);

  /**
   *
   * \param oscillator
   * \return
   */
  std::complex<double> GetOscillatorComplexSum (std::vector< Ptr<SatFadingOscillator> > oscillator);

  /**
   *
   * \param state
   */
  void ChangeState (uint32_t state);

  /**
   *
   * \param set
   * \param state
   */
  void ChangeSet (uint32_t set, uint32_t state);
};

} // namespace ns3

#endif /* SATELLITE_LOO_MODEL_H */
