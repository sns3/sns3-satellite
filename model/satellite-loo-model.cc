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

NS_OBJECT_ENSURE_REGISTERED (SatLooModel);

TypeId SatLooModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLooModel")
    .SetParent<SatBaseFader> ()
    .AddConstructor<SatLooModel> ();
  return tid;
}

SatLooModel::SatLooModel ()
  : m_numOfStates (0),
    m_currentSet (0),
    m_currentState (0),
    m_looConf (NULL),
    m_normalRandomVariable (NULL),
    m_uniformVariable (NULL)
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatLooModel::SatLooModel - Constructor not in use");
}

SatLooModel::SatLooModel (Ptr<SatLooConf> looConf, uint32_t numOfStates, uint32_t initialSet, uint32_t initialState)
  : m_numOfStates (numOfStates),
    m_currentSet (initialSet),
    m_currentState (initialState),
    m_looConf (looConf),
    m_normalRandomVariable (NULL),
    m_uniformVariable (NULL)
{
  NS_LOG_FUNCTION (this << numOfStates << " " << initialSet << " " << initialState);

  /// initialize random number generators
  m_normalRandomVariable = CreateObject<NormalRandomVariable> ();
  m_uniformVariable = CreateObject<UniformRandomVariable> ();
  m_uniformVariable->SetAttribute ("Min", DoubleValue (-1.0 * M_PI));
  m_uniformVariable->SetAttribute ("Max", DoubleValue (M_PI));

  /// initialize parameters for this set and state, construct oscillators
  ChangeSet (m_currentSet, m_currentState);
}

SatLooModel::~SatLooModel ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatLooModel::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
  SatBaseFader::DoDispose ();
}

void
SatLooModel::Reset ()
{
  NS_LOG_FUNCTION (this);

  m_looConf = NULL;
  m_normalRandomVariable = NULL;
  m_uniformVariable = NULL;

  if (!m_directSignalOscillators.empty ())
    {
      for (uint32_t i = 0; i < m_numOfStates; i++)
        {
          if (!m_directSignalOscillators[i].empty ())
            {
              m_directSignalOscillators[i].clear ();
            }
        }
      m_directSignalOscillators.clear ();
    }

  if (!m_multipathOscillators.empty ())
    {
      for (uint32_t i = 0; i < m_numOfStates; i++)
        {
          if (!m_multipathOscillators[i].empty ())
            {
              m_multipathOscillators[i].clear ();
            }
        }
      m_multipathOscillators.clear ();
    }

  m_looParameters.clear ();
  m_sigma.clear ();
}

void
SatLooModel::ConstructDirectSignalOscillators ()
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < m_numOfStates; i++)
    {
      std::vector< Ptr<SatFadingOscillator> > oscillators;

      /// Initial phase is common for all oscillators:
      double phi = m_uniformVariable->GetValue ();
      /// Theta is common for all oscillators:
      double theta = m_uniformVariable->GetValue ();
      for (uint32_t j = 0; j < m_looParameters[i][3]; j++)
        {
          uint32_t n = j + 1;
          /// 1. Rotation speed
          /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
          double alpha = (2.0 * M_PI * n - M_PI + theta) / (4.0 * m_looParameters[i][3]);
          /// 1b. Initiate rotation speed:
          double omega = 2.0 * M_PI * m_looParameters[i][5] * std::cos (alpha);
          /// 2. Initiate amplitude:

          /// TODO: Direct signal amplitude calculations will need to be verified,
          /// especially the target std. deviation with combined oscillators.
          /// Currently the std. dev is applied to individual oscillators. Combining
          /// these averages these and may result in too small std. dev with the combined
          /// value.
          double amplitude = m_normalRandomVariable->GetValue (m_looParameters[i][0], m_looParameters[i][1]);
          amplitude = pow (10,amplitude / 10) / m_looParameters[i][3];

          /// 3. Construct oscillator:
          oscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
        }
      m_directSignalOscillators.push_back (oscillators);
    }
}

void
SatLooModel::ConstructMultipathOscillators ()
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < m_numOfStates; i++)
    {
      std::vector< Ptr<SatFadingOscillator> > oscillators;

      /// Initial phase is common for all oscillators:
      double phi = m_uniformVariable->GetValue ();
      /// Theta is common for all oscillators:
      double theta = m_uniformVariable->GetValue ();
      for (uint32_t j = 0; j < m_looParameters[i][4]; j++)
        {
          uint32_t n = j + 1;
          /// 1. Rotation speed
          /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is oscillatorNumber, M is m_nOscillators
          double alpha = (2.0 * M_PI * n - M_PI + theta) / (4.0 * m_looParameters[i][4]);
          /// 1b. Initiate rotation speed:
          double omega = 2.0 * M_PI * m_looParameters[i][6] * std::cos (alpha);
          /// 2. Initiate complex amplitude:
          double psi = m_normalRandomVariable->GetValue ();
          std::complex<double> amplitude = std::complex<double> (std::cos (psi), std::sin (psi)) * 2.0 / std::sqrt (m_looParameters[i][4]);
          /// 3. Construct oscillator:
          oscillators.push_back (CreateObject<SatFadingOscillator> (amplitude, phi, omega));
        }
      m_multipathOscillators.push_back (oscillators);
    }
}

double
SatLooModel::GetChannelGainDb ()
{
  NS_LOG_FUNCTION (this);

  double tempChannelGainDb = 10.0 * std::log10 (GetChannelGain ());
  NS_LOG_INFO ("Time " << Now ().GetSeconds () << " " << tempChannelGainDb);
  return tempChannelGainDb;
}

double
SatLooModel::GetChannelGain ()
{
  NS_LOG_FUNCTION (this);

  double timeInSeconds = Now ().GetSeconds ();

  /// Direct signal
  std::complex<double> directComplexGain = GetOscillatorCosineWaveSum (m_directSignalOscillators[m_currentState], timeInSeconds);

  /// Multipath
  std::complex<double> multipathComplexGain = GetOscillatorComplexSum (m_multipathOscillators[m_currentState], timeInSeconds);
  multipathComplexGain = multipathComplexGain * m_sigma[m_currentState];

  /// Combining
  std::complex<double> fadingGain = directComplexGain + multipathComplexGain;
  return sqrt ((pow (fadingGain.real (), 2) + pow (fadingGain.imag (), 2)));
}

std::complex<double>
SatLooModel::GetOscillatorCosineWaveSum (std::vector< Ptr<SatFadingOscillator> > oscillator, double timeInSeconds)
{
  NS_LOG_FUNCTION (this);

  std::complex<double> complexSum = std::complex<double> (0, 0);

  for (uint32_t i = 0; i < oscillator.size (); i++)
    {
      complexSum += oscillator[i]->GetCosineWaveValueAt (timeInSeconds);
    }

  return complexSum;
}

std::complex<double>
SatLooModel::GetOscillatorComplexSum (std::vector< Ptr<SatFadingOscillator> > oscillator, double timeInSeconds)
{
  NS_LOG_FUNCTION (this);

  std::complex<double> complexSum = std::complex<double> (0, 0);

  for (uint32_t i = 0; i < oscillator.size (); i++)
    {
      complexSum += oscillator[i]->GetComplexValueAt (timeInSeconds);
    }

  return complexSum;
}

void
SatLooModel::UpdateParameters (uint32_t newSet, uint32_t newState)
{
  NS_LOG_FUNCTION (this << newSet << " " << newState);

  if (m_currentSet != newSet)
    {
      ChangeSet (newSet, newState);
    }

  if (m_currentSet == newSet && m_currentState != newState)
    {
      ChangeState (newState);
    }
}

void
SatLooModel::ChangeSet (uint32_t newSet, uint32_t newState)
{
  NS_LOG_FUNCTION (this << newSet << " " << newState);

  m_looParameters.clear ();
  m_looParameters = m_looConf->GetParameters (newSet);
  m_currentSet = newSet;

  ChangeState (newState);

  if (!m_directSignalOscillators.empty ())
    {
      for (uint32_t i = 0; i < m_numOfStates; i++)
        {
          if (!m_directSignalOscillators[i].empty ())
            {
              m_directSignalOscillators[i].clear ();
            }
        }
      m_directSignalOscillators.clear ();
    }

  if (!m_multipathOscillators.empty ())
    {
      for (uint32_t i = 0; i < m_numOfStates; i++)
        {
          if (!m_multipathOscillators[i].empty ())
            {
              m_multipathOscillators[i].clear ();
            }
        }
      m_multipathOscillators.clear ();
    }

  m_sigma.clear ();

  ConstructDirectSignalOscillators ();
  ConstructMultipathOscillators ();
  CalculateSigma ();
}

void
SatLooModel::ChangeState (uint32_t newState)
{
  NS_LOG_FUNCTION (this << newState);

  m_currentState = newState;
}

void
SatLooModel::CalculateSigma ()
{
  NS_LOG_FUNCTION (this);

  for (uint32_t i = 0; i < m_numOfStates; i++)
    {
      m_sigma.push_back (sqrt (0.5 * pow (10,(m_looParameters[i][2] / 10))));
    }
}

} // namespace ns3
