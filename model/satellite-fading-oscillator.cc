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
  static TypeId tid = TypeId ("ns3::SatFadingOscillator").SetParent<Object> ().AddConstructor<SatFadingOscillator> ();
  return tid;
}

//// Default constructor
SatFadingOscillator::SatFadingOscillator () :
  m_amplitude (0,0),
  m_phase (0),
  m_omega (0)
{
  NS_ASSERT(0);
}

/// Represents a single oscillator
SatFadingOscillator::SatFadingOscillator (std::complex<double> amplitude, double initialPhase, double omega) :
  m_amplitude (amplitude),
  m_phase (initialPhase),
  m_omega (omega)
{}

std::complex<double>
SatFadingOscillator::GetValueAt (Time at) const
{
  return (m_amplitude * std::cos (at.GetSeconds () * m_omega + m_phase));
}

SatFadingOscillator::~SatFadingOscillator ()
{
}

} // namespace ns3
