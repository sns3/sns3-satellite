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

NS_OBJECT_ENSURE_REGISTERED (SatRayleighModel);
NS_LOG_COMPONENT_DEFINE ("SatRayleighModel");

TypeId SatRayleighModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRayleighModel")
    .SetParent<SatBaseFader> ()
    .AddConstructor<SatRayleighModel> ();
  return tid;
}

SatRayleighModel::SatRayleighModel ()
  : m_currentSet (),
    m_currentState (),
    m_rayleighConf ()
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatRayleighModel - Constructor not in use");
}

SatRayleighModel::SatRayleighModel (Ptr<SatRayleighConf> rayleighConf, uint32_t initialSet, uint32_t initialState)
  : m_currentSet (initialSet),
    m_currentState (initialState),
    m_rayleighConf (rayleighConf)
{
  NS_LOG_FUNCTION (this);

  m_uniformVariable = CreateObject<UniformRandomVariable> ();
  m_uniformVariable->SetAttribute ("Min", DoubleValue (-1.0 * M_PI));
  m_uniformVariable->SetAttribute ("Max", DoubleValue (M_PI));

  m_rayleighParameters = m_rayleighConf->GetParameters (m_currentSet);

  ConstructOscillators ();
}

SatRayleighModel::~SatRayleighModel ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatRayleighModel::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
  SatBaseFader::DoDispose ();
}

void
SatRayleighModel::Reset ()
{
  NS_LOG_FUNCTION (this);

  m_rayleighConf = NULL;
  m_oscillators.clear ();
  m_uniformVariable = NULL;
}

void
SatRayleighModel::ConstructOscillators ()
{
  NS_LOG_FUNCTION (this);

  ///Initial phase is common for all oscillators:
  double phi = m_uniformVariable->GetValue ();
  /// Theta is common for all oscillators:
  double theta = m_uniformVariable->GetValue ();
  for (uint32_t i = 0; i < m_rayleighParameters[0][1]; i++)
    {
      uint32_t n = i + 1;
      /// 1. Rotation speed
      /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
      double alpha = (2.0 * M_PI * n - M_PI + theta) / (4.0 * m_rayleighParameters[0][1]);
      /// 1b. Initiate rotation speed:
      double omega = 2.0 * m_rayleighParameters[0][0] * M_PI * std::cos (alpha);
      /// 2. Initiate complex amplitude:
      double psi = m_uniformVariable->GetValue ();
      std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_rayleighParameters[0][1]);
      /// 3. Construct oscillator:
      m_oscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
    }
}

std::complex<double>
SatRayleighModel::GetComplexGain ()
{
  NS_LOG_FUNCTION (this);

  double timeInSeconds = Now ().GetSeconds ();

  std::complex<double> sumAmplitude = std::complex<double> (0,0);
  for (uint32_t i = 0; i < m_oscillators.size (); i++)
    {
      sumAmplitude += m_oscillators[i]->GetComplexValueAt (timeInSeconds);
    }
  return sumAmplitude;
}

double
SatRayleighModel::GetChannelGainDb ()
{
  NS_LOG_FUNCTION (this);

  std::complex<double> complexGain = GetComplexGain ();
  double tempChannelGainDb = (10 * std::log10 ((std::pow (complexGain.real (), 2) + std::pow (complexGain.imag (), 2)) / 2));
  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " " << tempChannelGainDb);
  return tempChannelGainDb;
}

double
SatRayleighModel::GetChannelGain ()
{
  NS_LOG_FUNCTION (this);

  std::complex<double> complexGain = GetComplexGain ();

  double tempChannelGain = ((std::pow (complexGain.real (), 2) + std::pow (complexGain.imag (), 2)) / 2);
  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " " << tempChannelGain);
  return tempChannelGain;
}

void
SatRayleighModel::UpdateParameters (uint32_t newSet, uint32_t newState)
{
  NS_LOG_FUNCTION (this << newSet << " " << newState);
  m_currentSet = newSet;
  m_currentState = newState;
}

} // namespace ns3
