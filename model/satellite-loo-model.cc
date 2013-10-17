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

NS_LOG_COMPONENT_DEFINE ("SatLooModel");

namespace ns3 {

const double SatLooModel::PI = 3.14159265358979323846;

NS_OBJECT_ENSURE_REGISTERED (SatLooModel);

TypeId SatLooModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLooModel")
      .SetParent<SatFader> ()
      .AddConstructor<SatLooModel> ();
  return tid;
}

SatLooModel::SatLooModel () :
  m_setId (0),
  m_stateId (0),
  m_mean (0),
  m_stdDev (0),
  m_multipathPower (0),
  m_sigma (0)
{
  NS_ASSERT(0);
}

SatLooModel::SatLooModel (Ptr<SatLooConf> looConf, uint32_t set, uint32_t state) :
    m_setId (set),
    m_stateId (state)
{

  m_looConf = looConf;

  // initialize parameters for this set and state
  ChangeSet (m_setId);
  ChangeState (m_stateId);

  // === TESTING === TODO: remove these, make parameters
  //m_mean = -8.0;
  //m_stdDev = 3.0;
  //m_multipathPower = -10.0;
  //m_sigma = sqrt(0.5 * pow(10,(m_multipathPower / 10)));

  //m_nFastOscillators = 4;
  //m_nSlowOscillators = 4;

  //m_slowFadingOmegaDopplerMax = 0.15;
  //m_slowFadingFrequencyOmegaDopplerMax = 0.15;
  //m_fastFadingOmegaDopplerMax = 100;
  // === TESTING ===

  m_normalRandomVariable = CreateObject<NormalRandomVariable> ();

  m_uniformVariable = CreateObject<UniformRandomVariable> ();
  m_uniformVariable->SetAttribute ("Min", DoubleValue (-1.0 * PI));
  m_uniformVariable->SetAttribute ("Max", DoubleValue (PI));

  ConstructSlowFadingOscillators ();
  ConstructFastFadingOscillators ();
  ConstructSlowFadingFrequencyOscillators ();
}

SatLooModel::~SatLooModel ()
{
}

void
SatLooModel::ConstructSlowFadingOscillators ()
{
  // Initial phase is common for all oscillators:
  double phi = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
  // Theta is common for all oscillators:
  double theta = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
  for (uint32_t i = 0; i < m_nSlowOscillators; i++)
    {
      uint32_t n = i + 1;
      /// 1. Rotation speed
      /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
      double alpha = (2.0 * SatLooModel::PI * n - SatLooModel::PI + theta) / (4.0 * m_nSlowOscillators);
      /// 1b. Initiate rotation speed:
      double omega = m_slowFadingOmegaDopplerMax * std::cos (alpha);
      /// 2. Initiate complex amplitude:
      double psi = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
      std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_nSlowOscillators);
      /// 3. Construct oscillator:
      m_slowFadingOscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
    }
}

void
SatLooModel::ConstructSlowFadingFrequencyOscillators ()
{
  // Initial phase is common for all oscillators:
  double phi = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
  // Theta is common for all oscillators:
  double theta = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
  for (uint32_t i = 0; i < m_nSlowOscillators; i++)
    {
      uint32_t n = i + 1;
      /// 1. Rotation speed
      /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
      double alpha = (2.0 * SatLooModel::PI * n - SatLooModel::PI + theta) / (4.0 * m_nSlowOscillators);
      /// 1b. Initiate rotation speed:
      double omega = m_slowFadingFrequencyOmegaDopplerMax * std::cos (alpha);
      /// 2. Initiate complex amplitude:
      double psi = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
      std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_nSlowOscillators);
      /// 3. Construct oscillator:
      m_slowFadingFrequencyOscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
    }
}

void
SatLooModel::ConstructFastFadingOscillators ()
{
  // Initial phase is common for all oscillators:
  double phi = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
  // Theta is common for all oscillators:
  double theta = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
  for (uint32_t i = 0; i < m_nFastOscillators; i++)
    {
      uint32_t n = i + 1;
      /// 1. Rotation speed
      /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
      double alpha = (2.0 * SatLooModel::PI * n - SatLooModel::PI + theta) / (4.0 * m_nFastOscillators);
      /// 1b. Initiate rotation speed:
      double omega = m_fastFadingOmegaDopplerMax * std::cos (alpha);
      /// 2. Initiate complex amplitude:
      double psi = m_normalRandomVariable->GetValue (m_mean, m_stdDev);
      std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_nFastOscillators);
      /// 3. Construct oscillator:
      m_fastFadingOscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
    }
}


double
SatLooModel::GetChannelGainDb ()
{
  double tempChannelGainDb = 10.0 * std::log10 (GetChannelGain ());
  NS_LOG_INFO("Time " << Now ().GetSeconds () << " " << tempChannelGainDb);
  return tempChannelGainDb;
}

double
SatLooModel::GetChannelGain () // TODO: check validity
{
  //// Direct signal
  std::complex<double> slowComplexGain = GetOscillatorComplexSum (m_slowFadingOscillators);
  std::complex<double> slowComplexFrequency = GetOscillatorComplexSum (m_slowFadingFrequencyOscillators);

  // check the / 2, should it be sqrt?
  double slowGain = ((pow (slowComplexGain.real (), 2) + pow (slowComplexGain.imag (), 2)) / 2);

  // check the / 2, should it be sqrt?
  double slowFrequency = ((pow (slowComplexFrequency.real (), 2) + pow (slowComplexFrequency.imag (), 2)) / 2);

  // Verify this! Does it require normalization? Add stdDev!
  double slowFading = m_mean + slowGain + std::cos (Now ().GetSeconds () * 2.0 * SatLooModel::PI * m_slowFadingOmegaDopplerMax + slowFrequency);

  //// Multipath
  std::complex<double> fastComplexGain = GetOscillatorComplexSum (m_fastFadingOscillators);
  fastComplexGain = fastComplexGain * m_sigma;

  // check the sqrt, should it be / 2 instead!!
  double fastFading = sqrt((pow (fastComplexGain.real (), 2) + pow (fastComplexGain.imag (), 2)));
  fastFading = 10 * log10(fastFading);

  // TESTING BEGINS

  //double direct = m_normalRandomVariable->GetValue(m_mean,m_stdDev);

  //std::complex<double> rayleighGain = (m_normalRandomVariable->GetValue(), m_normalRandomVariable->GetValue());
  //rayleighGain = rayleighGain * m_sigma;
  //double rayleighFading = sqrt(pow (rayleighGain.real (), 2) + pow (rayleighGain.imag (), 2));
  //rayleighFading = 20 * log10(rayleighFading);

  return (slowFading + fastFading) / 2; // is divide by 2 correct?
  //return slowFading;
  //return fastFading;

}

double
SatLooModel::GetOscillatorSum (std::vector< Ptr<SatFadingOscillator> > oscillator)
{
  double sum = 0;

  for (uint32_t i = 0; i < oscillator.size (); i++)
    {
      sum += oscillator[i]->GetValueAt (Now ());
    }

  return sum;
}

std::complex<double>
SatLooModel::GetOscillatorComplexSum (std::vector< Ptr<SatFadingOscillator> > oscillator)
{
  std::complex<double> complexSum = std::complex<double> (0, 0);

  for (uint32_t i = 0; i < oscillator.size (); i++)
    {
      complexSum += oscillator[i]->GetComplexValueAt (Now ());
    }

  return complexSum;
}

void
SatLooModel::UpdateParameters (uint32_t set, uint32_t state)
{
  if (m_setId != set)
    {
      ChangeSet (set);
      ChangeState (state);
    }

  if (m_setId == set && m_stateId != state)
    {
      ChangeState (state);
    }
}

void
SatLooModel::ChangeSet (uint32_t set)
{
  m_looParameters.clear();
  m_looParameters = m_looConf->GetLooParameters (set);
  m_setId = set;
}

void
SatLooModel::ChangeState (uint32_t state)
{
  m_mean = m_looParameters[state][0];
  m_stdDev = m_looParameters[state][1];
  m_multipathPower = m_looParameters[state][2];

  m_nFastOscillators = m_looParameters[state][3];
  m_nSlowOscillators = m_looParameters[state][4];

  m_slowFadingOmegaDopplerMax = m_looParameters[state][5];
  m_slowFadingFrequencyOmegaDopplerMax = m_looParameters[state][6];
  m_fastFadingOmegaDopplerMax = m_looParameters[state][7];

  m_sigma = sqrt(pow(((m_multipathPower / 10.0) / 2.0),10.0));
  m_stateId = state;
}

} // namespace ns3
