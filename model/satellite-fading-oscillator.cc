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

#include "satellite-fading-oscillator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFadingOscillator);
NS_LOG_COMPONENT_DEFINE ("SatFadingOscillator");

TypeId SatFadingOscillator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFadingOscillator")
    .SetParent<Object> ()
    .AddConstructor<SatFadingOscillator> ();
  return tid;
}

SatFadingOscillator::SatFadingOscillator ()
  : m_complexAmplitude (0,0),
    m_amplitude (0),
    m_phase (0),
    m_omega (0)
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatFadingOscillator::SatFadingOscillator - Constructor not in use");
}

SatFadingOscillator::SatFadingOscillator (std::complex<double> amplitude, double initialPhase, double omega)
  : m_complexAmplitude (amplitude),
    m_amplitude (0),
    m_phase (initialPhase),
    m_omega (omega)
{
  NS_LOG_FUNCTION (this << amplitude << " " << initialPhase << " " << omega);
}

SatFadingOscillator::SatFadingOscillator (double amplitude, double initialPhase, double omega)
  : m_complexAmplitude (0,0),
    m_amplitude (amplitude),
    m_phase (initialPhase),
    m_omega (omega)
{
  NS_LOG_FUNCTION (this << amplitude << " " << initialPhase << " " << omega);
}

std::complex<double>
SatFadingOscillator::GetComplexValueAt (double timeInSeconds) const
{
  NS_LOG_FUNCTION (this << timeInSeconds);

  return (m_complexAmplitude * std::cos (timeInSeconds * m_omega + m_phase));
}

std::complex<double>
SatFadingOscillator::GetCosineWaveValueAt (double timeInSeconds) const
{
  NS_LOG_FUNCTION (this << timeInSeconds);

  std::complex<double> complexPhase ( std::cos (timeInSeconds * m_omega + m_phase), std::sin (timeInSeconds * m_omega + m_phase) );
  return (m_amplitude * std::exp (complexPhase));
}

SatFadingOscillator::~SatFadingOscillator ()
{
  NS_LOG_FUNCTION (this);
}

void
SatFadingOscillator::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Object::DoDispose ();
}

} // namespace ns3
