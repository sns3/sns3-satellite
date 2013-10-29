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

  static TypeId GetTypeId (void);

  /** Create Loo model. */
  SatLooModel ();

  SatLooModel (Ptr<SatLooConf> looConf, uint32_t set, uint32_t state);

  /** Destroy the Loo model. */
  ~SatLooModel ();

  double GetChannelGainDb ();

  double GetChannelGain ();

  void UpdateParameters (uint32_t set, uint32_t state);

private:
  uint32_t m_setId;
  uint32_t m_stateId;
  double m_mean;                       // mean in dB
  double m_stdDev;                     // std in dB
  double m_multipathPower;             // rms squared value of diffuse multipath dB
  double m_sigma;                      // convert multipath to linear units
  double m_slowFadingOmegaDopplerMax;
  double m_fastFadingOmegaDopplerMax;

  uint32_t m_nFastOscillators;
  uint32_t m_nSlowOscillators;

  Ptr<SatLooConf> m_looConf;
  std::vector<std::vector<double> > m_looParameters;

  Ptr<NormalRandomVariable> m_normalRandomVariable;
  Ptr<UniformRandomVariable> m_uniformVariable;

  /// Vectors of oscillators:
  std::vector< Ptr<SatFadingOscillator> > m_slowFadingOscillators;
  std::vector< Ptr<SatFadingOscillator> > m_fastFadingOscillators;

  void ConstructSlowFadingOscillators ();
  void ConstructFastFadingOscillators ();

  std::complex<double> GetOscillatorCosineWaveSum (std::vector< Ptr<SatFadingOscillator> > oscillator);
  std::complex<double> GetOscillatorComplexSum (std::vector< Ptr<SatFadingOscillator> > oscillator);

  void ChangeState (uint32_t state);
  void ChangeSet (uint32_t set);
};

} // namespace ns3

#endif /* SATELLITE_LOO_MODEL_H */
