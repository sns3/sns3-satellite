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

#include "satellite-loo-model.h"
#include "satellite-utils.h"

namespace ns3 {

const double SatLooModel::PI = 3.14159265358979323846;

NS_OBJECT_ENSURE_REGISTERED (SatLooModel);
NS_LOG_COMPONENT_DEFINE ("SatLooModel");

TypeId SatLooModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLooModel")
      .SetParent<Object> ()
      .AddConstructor<SatLooModel> ();
  return tid;
}

SatLooModel::SatLooModel () :
  m_mean (0),
  m_stdDev (0),
  m_multipathPower (0),
  m_sigma (0),
  m_mu (0),
  m_omegaDopplerMax (0),
  m_nOscillators (0)
{
  NS_ASSERT(0);
}

SatLooModel::SatLooModel (double mean, double stdDev, double multipathPower, double dopplerFrequencyHz, uint32_t numOfOscillators) :
    m_mean(mean),
    m_stdDev(stdDev),
    m_multipathPower(multipathPower),
    m_omegaDopplerMax (2.0 * dopplerFrequencyHz * SatLooModel::PI),
    m_nOscillators (numOfOscillators)
{
  NS_ASSERT (m_nOscillators != 0);
  NS_ASSERT (m_omegaDopplerMax != 0);

  m_sigma = sqrt( log( 1 + (m_stdDev / pow(m_mean,2))));

  //m_sigma = sqrt(pow(((m_multipathPower / 10.0) / 2.0),10.0));

  m_mu = log(  pow(m_mean,2) / sqrt(m_stdDev + pow(m_mean,2)));

  m_uniformVariable = CreateObject<UniformRandomVariable> ();
  m_uniformVariable->SetAttribute ("Min", DoubleValue (-1.0 * PI));
  m_uniformVariable->SetAttribute ("Max", DoubleValue (PI));

  m_logNormalVariable = CreateObject<LogNormalRandomVariable> ();
  m_logNormalVariable->SetAttribute ("Mu", DoubleValue (m_mu));
  m_logNormalVariable->SetAttribute ("Sigma", DoubleValue (m_sigma));

  ConstructUniformOscillators ();
  ConstructLogNormalOscillators ();
}

SatLooModel::~SatLooModel ()
{
  m_uniformOscillators.clear ();
  m_logNormalOscillators.clear ();
  m_uniformVariable = 0;
  m_logNormalVariable = 0;
}

void
SatLooModel::ConstructUniformOscillators ()
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
      double alpha = (2.0 * SatLooModel::PI * n - SatLooModel::PI + theta) / (4.0 * m_nOscillators);
      /// 1b. Initiate rotation speed:
      double omega = m_omegaDopplerMax * std::cos (alpha);
      /// 2. Initiate complex amplitude:
      double psi = m_uniformVariable->GetValue ();
      std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_nOscillators);
      /// 3. Construct oscillator:
      m_uniformOscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
    }
}

void
SatLooModel::ConstructLogNormalOscillators ()
{
  // Initial phase is common for all oscillators:
  double phi = m_logNormalVariable->GetValue ();
  // Theta is common for all oscillators:
  double theta = m_logNormalVariable->GetValue ();
  for (uint32_t i = 0; i < m_nOscillators; i++)
    {
      uint32_t n = i + 1;
      /// 1. Rotation speed
      /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
      double alpha = (2.0 * SatLooModel::PI * n - SatLooModel::PI + theta) / (4.0 * m_nOscillators);
      /// 1b. Initiate rotation speed:
      double omega = m_omegaDopplerMax * std::cos (alpha);
      /// 2. Initiate complex amplitude:
      double psi = m_logNormalVariable->GetValue ();
      std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_nOscillators);
      /// 3. Construct oscillator:
      m_logNormalOscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
    }
}

std::complex<double>
SatLooModel::GetUniformComplexGain ()
{
  std::complex<double> sumUniformAmplitude = std::complex<double> (0, 0);

  for (uint32_t i = 0; i < m_uniformOscillators.size (); i++)
    {
      sumUniformAmplitude += m_uniformOscillators[i]->GetValueAt (Now ());
    }

  return sumUniformAmplitude;
}

std::complex<double>
SatLooModel::GetLogNormalComplexGain ()
{
  std::complex<double> sumLogNormalAmplitude = std::complex<double> (0, 0);

  for (uint32_t i = 0; i < m_logNormalOscillators.size (); i++)
    {
      sumLogNormalAmplitude += m_logNormalOscillators[i]->GetValueAt (Now ());
    }

  return sumLogNormalAmplitude;
}

double
SatLooModel::GetChannelGainDb ()
{
  double tempChannelGainDb = SatUtils::LinearToDb(GetChannelGain ());
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " " << tempChannelGainDb);
  return tempChannelGainDb;
}

double
SatLooModel::GetChannelGain ()
{
  std::complex<double> uniformComplexGain = GetUniformComplexGain ();
  std::complex<double> logNormalComplexGain = GetLogNormalComplexGain ();

  double tempUniformChannelGain = ((std::pow (uniformComplexGain.real (), 2) + std::pow (uniformComplexGain.imag (), 2)) / 2);
  double tempLogNormalChannelGain = ((std::pow (logNormalComplexGain.real (), 2) + std::pow (logNormalComplexGain.imag (), 2)) / 2);

  double tempTotalChannelGain = tempUniformChannelGain + (SatUtils::DbToLinear(m_multipathPower) * tempLogNormalChannelGain);

  //NS_LOG_INFO("Time " << Now ().GetSeconds () << " " << tempUniformChannelGain << " " << tempLogNormalChannelGain << " " << tempTotalChannelGain);

  return tempTotalChannelGain;
}

void
SatLooModel::UpdateParameters (double mean, double stdDev, double multipathPower)
{
  m_sigma = sqrt( log( 1 + (m_stdDev / pow(m_mean,2))));
  m_mu = log(  pow(m_mean,2) / sqrt(m_stdDev + pow(m_mean,2)));
  m_multipathPower = multipathPower;

  m_logNormalVariable->SetAttribute ("Mu", DoubleValue (m_mu));
  m_logNormalVariable->SetAttribute ("Sigma", DoubleValue (m_sigma));
}

} // namespace ns3
