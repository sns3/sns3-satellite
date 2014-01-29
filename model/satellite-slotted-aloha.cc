/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
#include "satellite-slotted-aloha.h"

NS_LOG_COMPONENT_DEFINE ("SatSlottedAloha");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatSlottedAloha);

TypeId 
SatSlottedAloha::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSlottedAloha")
    .SetParent<Object> ();
  return tid;
}

SatSlottedAloha::SatSlottedAloha () :
  m_min (0.0),
  m_max (0.0)
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatSlottedAloha::SatSlottedAloha - Constructor not in use");
}

SatSlottedAloha::SatSlottedAloha (Ptr<SatRandomAccessConf> randomAccessConf) :
  m_randomAccessConf (randomAccessConf),
  m_min (randomAccessConf->GetSlottedAlohaDefaultMin ()),
  m_max (randomAccessConf->GetSlottedAlohaDefaultMax ())
{
  NS_LOG_FUNCTION (this);

  m_uniformVariable = CreateObject<UniformRandomVariable> ();

  DoVariableSanityCheck ();

  NS_LOG_INFO ("SatSlottedAloha::SatSlottedAloha - Module created");
}

SatSlottedAloha::~SatSlottedAloha ()
{
  NS_LOG_FUNCTION (this);
}

void
SatSlottedAloha::DoVariableSanityCheck ()
{
  NS_LOG_FUNCTION (this);

  if (m_min < 0 || m_max < 0)
    {
      NS_FATAL_ERROR ("SatSlottedAloha::DoVariableSanityCheck - min < 0 || max < 0");
    }

  if (m_min > m_max)
    {
      NS_FATAL_ERROR ("SatSlottedAloha::DoVariableSanityCheck - min > max");
    }

  NS_LOG_INFO ("SatSlottedAloha::DoVariableSanityCheck - Variable sanity check done");
}

void
SatSlottedAloha::UpdateVariables (double min, double max)
{
  NS_LOG_FUNCTION (this << " new min: " << min << " new max: " << max);

  m_min = min;
  m_max = max;

  DoVariableSanityCheck ();

  NS_LOG_INFO ("SatSlottedAloha::UpdateVariables - new min: " << min << " new max: " << max);
}

/// TODO: implement this
bool
SatSlottedAloha::IsDamaAvailable ()
{
  NS_LOG_FUNCTION (this);

  bool isDamaAvailable = false;

  NS_LOG_INFO ("SatSlottedAloha::IsDamaAvailable: " << isDamaAvailable);

  return isDamaAvailable;
}

double
SatSlottedAloha::DoSlottedAloha ()
{
  NS_LOG_FUNCTION (this);

  /// TODO: what to return in the case SA is not used?
  double time = 0;

  NS_LOG_INFO ("SatSlottedAloha::DoSlottedAloha - Checking if we have DAMA allocations");

  /// Check if we have known DAMA allocations
  if (!IsDamaAvailable ())
    {
      NS_LOG_INFO ("SatSlottedAloha::DoSlottedAloha - No DAMA -> Running Slotted ALOHA");

      /// Evaluate Slotted ALOHA
      time = RandomizeReleaseTime ();
    }
  return time;
}

double
SatSlottedAloha::RandomizeReleaseTime ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatSlottedAloha::RandomizeReleaseTime - Randomizing the release time...");

  double releaseTime = Now ().GetSeconds () + m_uniformVariable->GetValue (m_min, m_max);

  NS_LOG_INFO ("SatSlottedAloha::RandomizeReleaseTime - TX opportunity in the next slot after the release time at: " << releaseTime << " seconds");

  return releaseTime;
}

} // namespace ns3
