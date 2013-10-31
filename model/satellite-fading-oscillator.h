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

  /**
   *
   */
  SatFadingOscillator ();

  /**
   *
   * \param amplitude
   * \param initialPhase
   * \param omega
   */
  SatFadingOscillator (std::complex<double> amplitude, double initialPhase, double omega);

  /**
   *
   * \param amplitude
   * \param initialPhase
   * \param omega
   */
  SatFadingOscillator (double amplitude, double initialPhase, double omega);

  /**
   *
   */
  ~SatFadingOscillator ();

  /**
   *
   * \param t
   * \return
   */
  std::complex<double> GetComplexValueAt (Time t) const;

  /**
   *
   * \param t
   * \return
   */
  std::complex<double> GetCosineWaveValueAt (Time t) const;

private:

  /**
   *
   */
  std::complex<double> m_complexAmplitude;
  double m_amplitude;
  double m_phase;
  double m_omega;

};

} // namespace ns3

#endif /* SATELLITE_FADING_OSCILLATOR_H */
