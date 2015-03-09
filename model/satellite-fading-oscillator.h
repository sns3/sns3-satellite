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
#ifndef SATELLITE_FADING_OSCILLATOR_H
#define SATELLITE_FADING_OSCILLATOR_H

#include "ns3/object.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <complex>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Oscillator class for faders. This class implements
 * the oscillators required by the faders for Markov-fading.
 * The provided oscillator values are used to form individual
 * fading samples.
 */
class SatFadingOscillator : public Object
{
public:
  /**
   * \brief NS-3 function for type id
   * @return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  SatFadingOscillator ();

  /**
   * \brief Constructor
   * \param amplitude amplitude
   * \param initialPhase intial phase
   * \param omega rotation speed
   */
  SatFadingOscillator (std::complex<double> amplitude, double initialPhase, double omega);

  /**
   * \brief Constructor
   * \param amplitude amplitude
   * \param initialPhase initial phase
   * \param omega rotation speed
   */
  SatFadingOscillator (double amplitude, double initialPhase, double omega);

  /**
   * \brief Destructor
   */
  ~SatFadingOscillator ();

  /**
   * \brief Returns complex value at time t
   * \param timeInSeconds current time in seconds
   * \return complex value
   */
  std::complex<double> GetComplexValueAt (double timeInSeconds) const;

  /**
   * \brief Returns cosine wave complex value at time t
   * \param timeInSeconds current time in seconds
   * \return complex value
   */
  std::complex<double> GetCosineWaveValueAt (double timeInSeconds) const;

  /**
   * \brief Do needed dispose actions
   */
  void DoDispose ();

private:
  /**
   * \brief Complex amplitude
   */
  std::complex<double> m_complexAmplitude;

  /**
   * \brief Amplitude
   */
  double m_amplitude;

  /**
   * \brief Initial phase
   */
  double m_phase;

  /**
   * \brief Rotation speed
   */
  double m_omega;

};

} // namespace ns3

#endif /* SATELLITE_FADING_OSCILLATOR_H */
