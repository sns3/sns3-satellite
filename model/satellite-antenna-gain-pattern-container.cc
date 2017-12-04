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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <sstream>
#include "ns3/log.h"
#include "satellite-antenna-gain-pattern-container.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"

NS_LOG_COMPONENT_DEFINE ("SatAntennaGainPatternContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatAntennaGainPatternContainer);


TypeId
SatAntennaGainPatternContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatAntennaGainPatternContainer")
    .SetParent<Object> ()
    .AddConstructor<SatAntennaGainPatternContainer> ();
  return tid;
}

SatAntennaGainPatternContainer::SatAntennaGainPatternContainer ()
{
  /**
   * TODO: To change the reference system, these hard coded paths
   * and filenames may have to be changed! One way could be to hard
   * code the antenna pattern names, but change the input folder
   * according to the wanted reference system.
   */
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->LocateDataDirectory ();
  std::string path = dataPath + "/antennapatterns/SatAntennaGain72Beams_";

  // Note, that the beam ids start from 1
  for (uint32_t i = 1; i <= NUMBER_OF_BEAMS; ++i)
    {
      std::ostringstream ss;
      ss << i;
      std::string filePathName = path + ss.str () + ".txt";
      Ptr<SatAntennaGainPattern> gainPattern = CreateObject<SatAntennaGainPattern> (filePathName);

      std::pair<std::map<uint32_t,Ptr<SatAntennaGainPattern> >::iterator, bool> ret;
      ret = m_antennaPatternMap.insert (std::pair<uint32_t, Ptr<SatAntennaGainPattern> > (i, gainPattern));

      if (ret.second == false)
        {
          NS_FATAL_ERROR (this << " an antenna pattern for beam " << i << " already exists!");
        }
    }
}

Ptr<SatAntennaGainPattern>
SatAntennaGainPatternContainer::GetAntennaGainPattern (uint32_t beamId) const
{
  NS_LOG_FUNCTION (this << beamId);

  if (beamId > m_antennaPatternMap.size ())
    {
      NS_FATAL_ERROR ("SatAntennaGainPatternContainer::GetAntennaGainPattern - unvalid beam id: " << beamId);
    }

  // Note, that now we assume that all the antenna patterns are created
  // regardless of how many beams are actually simulated.
  return m_antennaPatternMap.at (beamId);
}

uint32_t
SatAntennaGainPatternContainer::GetBestBeamId (GeoCoordinate coord) const
{
  NS_LOG_FUNCTION (this << coord.GetLatitude () << coord.GetLongitude ());

  double bestGain (-100.0);
  uint32_t bestId (0);

  for (uint32_t i = 1; i <= NUMBER_OF_BEAMS; ++i)
    {
      double gain = m_antennaPatternMap.at (i)->GetAntennaGain_lin (coord);

      // The antenna pattern has returned a NAN gain. This means
      // that this position is not valid. Return 0, which is not a valid beam id.
      if (std::isnan (gain))
        {
          NS_FATAL_ERROR (this << " returned a NAN antenna gain value!");
        }
      else if (gain > bestGain)
        {
          bestGain = gain;
          bestId = i;
        }
    }

  return bestId;
}

} // namespace ns3
