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
#include "satellite-fading-oscillator.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Loo model
 */
class SatLooModel : public Object
{
public:
  static const double PI;

  static TypeId GetTypeId (void);

  /** Create Loo model. */
  SatLooModel ();

  SatLooModel (double mean, double stdDev, double multipathPower, double dopplerFrequencyHz, uint32_t numOfOscillators);

  /** Destroy the Loo model. */
  ~SatLooModel ();

  double GetChannelGainDb ();

  double GetChannelGain ();

  void UpdateParameters (double mean, double stdDev, double multipathPower);

private:
  double m_mean;
  double m_stdDev;
  double m_multipathPower;

  // Log standard deviation
  double m_sigma;
  //
  double m_mu;

  double m_omegaDopplerMax;
  uint32_t m_nOscillators;

  /// Vector of oscillators:
  std::vector< Ptr<SatFadingOscillator> > m_uniformOscillators;
  std::vector< Ptr<SatFadingOscillator> > m_logNormalOscillators;

  Ptr<UniformRandomVariable> m_uniformVariable;
  Ptr<LogNormalRandomVariable> m_logNormalVariable;

  void ConstructLogNormalOscillators ();
  void ConstructUniformOscillators ();
  std::complex<double> GetUniformComplexGain ();
  std::complex<double> GetLogNormalComplexGain ();
};

} // namespace ns3

#endif /* SATELLITE_LOO_MODEL_H */
