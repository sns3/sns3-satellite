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

#include "satellite-rayleigh-model.h"

namespace ns3 {

const double SatRayleighModel::PI = 3.14159265358979323846;

NS_OBJECT_ENSURE_REGISTERED (SatRayleighModel);
NS_LOG_COMPONENT_DEFINE ("SatRayleighModel");

TypeId SatRayleighModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRayleighModel")
      .SetParent<Object> ()
      .AddConstructor<SatRayleighModel> ();
  return tid;
}

SatRayleighModel::SatRayleighModel () :
  m_omegaDopplerMax (0),
  m_nOscillators (0)
{
  NS_ASSERT(0);
}

SatRayleighModel::SatRayleighModel (double dopplerFrequencyHz, uint32_t numOfOscillators) :
  m_omegaDopplerMax (2.0 * dopplerFrequencyHz * SatRayleighModel::PI),
  m_nOscillators (numOfOscillators)
{
  NS_ASSERT (m_nOscillators != 0);
  NS_ASSERT (m_omegaDopplerMax != 0);

  m_uniformVariable = CreateObject<UniformRandomVariable> ();
  m_uniformVariable->SetAttribute ("Min", DoubleValue (-1.0 * PI));
  m_uniformVariable->SetAttribute ("Max", DoubleValue (PI));

  ConstructOscillators ();
}

SatRayleighModel::~SatRayleighModel()
{
  m_oscillators.clear ();
  m_uniformVariable = 0;
}

void
SatRayleighModel::ConstructOscillators ()
{
  // Initial phase is common for all oscillators:
  double phi = m_uniformVariable->GetValue ();
  // Theta is common for all oscillators:
  double theta = m_uniformVariable->GetValue ();
  for (uint32_t i = 0; i < m_nOscillators; i++)
    {
      uint32_t n = i + 1;
      /// 1. Rotation speed
      /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
      double alpha = (2.0 * SatRayleighModel::PI * n - SatRayleighModel::PI + theta) / (4.0 * m_nOscillators);
      /// 1b. Initiate rotation speed:
      double omega = m_omegaDopplerMax * std::cos (alpha);
      /// 2. Initiate complex amplitude:
      double psi = m_uniformVariable->GetValue ();
      std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_nOscillators);
      /// 3. Construct oscillator:
      m_oscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
    }
}

std::complex<double>
SatRayleighModel::GetComplexGain () const
{
  std::complex<double> sumAmplitude = std::complex<double> (0, 0);
  for (uint32_t i = 0; i < m_oscillators.size (); i++)
    {
      sumAmplitude += m_oscillators[i]->GetComplexValueAt (Now ());
    }
  return sumAmplitude;
}

double
SatRayleighModel::GetChannelGainDb () const
{
  std::complex<double> complexGain = GetComplexGain ();
  double tempChannelGainDb = (10 * std::log10 ((std::pow (complexGain.real (), 2) + std::pow (complexGain.imag (), 2)) / 2));
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " " << tempChannelGainDb);
  return tempChannelGainDb;
}

double
SatRayleighModel::GetChannelGain () const
{
  std::complex<double> complexGain = GetComplexGain ();

  double tempChannelGain = ((std::pow (complexGain.real (), 2) + std::pow (complexGain.imag (), 2)) / 2);
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " " << tempChannelGain);
  return tempChannelGain;
}

} // namespace ns3
