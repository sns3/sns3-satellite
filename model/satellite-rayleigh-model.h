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
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief
 */
class SatRayleighModel : public Object
{
public:

  /**
   * \brief Value for PI
   */
  static const double PI;

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
   * \param omegaDopplerMax maximum Doppler
   * \param numOfOscillators number of oscillators
   */
  SatRayleighModel (double omegaDopplerMax, uint32_t numOfOscillators);

  /**
   * \brief Destructor
   */
  ~SatRayleighModel ();

  /**
   * \brief Function for calculating the oscillator complex gain
   * \return complex gain
   */
  std::complex<double> GetComplexGain () const;

  /**
   * \brief Function for returning the channel gain in dB
   * \return channel gain in dB
   */
  double GetChannelGainDb () const;

  /**
   * \brief Function for returning the channel gain
   * \return channel gain
   */
  double GetChannelGain () const;

private:

  /**
   * \brief Function for constructing the oscillators
   */
  void ConstructOscillators ();

  /**
   * \brief Vector of oscillators
   */
  std::vector< Ptr<SatFadingOscillator> > m_oscillators;

  /**
   * \brief Maximum omega Doppler
   */
  double m_omegaDopplerMax;

  /**
   * \brief Number of oscillators
   */
  uint32_t m_nOscillators;

  /**
   * \brief Uniform distribution random variable
   */
  Ptr<UniformRandomVariable> m_uniformVariable;
};

} // namespace ns3

#endif /* SATELLITE_RAYLEIGH_MODEL_H */
