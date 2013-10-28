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
 * \brief
 */
class SatFadingOscillator : public Object
{
public:
  static TypeId GetTypeId (void);

  //// Default constructor
  SatFadingOscillator ();

  /// Initiate oscillator with complex amplitude, initial phase and rotation speed
  SatFadingOscillator (std::complex<double> amplitude, double initialPhase, double omega);

  /// Initiate oscillator with amplitude, initial phase and rotation speed
  SatFadingOscillator (double amplitude, double initialPhase, double omega);

  ~SatFadingOscillator ();

  // Get the complex amplitude at moment \param t
  std::complex<double> GetComplexValueAt (Time t) const;

  // Get the complex amplitude at moment \param t
  std::complex<double> GetCosineWaveValueAt (Time t) const;

private:

  /// Complex number \f[Re=\cos(\psi_n), Im = i\sin(\psi_n)]
  std::complex<double> m_complexAmplitude;

  double m_amplitude;

  /// Phase \f[\phi_n] of the oscillator
  double m_phase;

  /// Rotation speed of the oscillator \f[\omega_d \cos(\alpha_n)]
  double m_omega;

};

} // namespace ns3

#endif /* SATELLITE_FADING_OSCILLATOR_H */
