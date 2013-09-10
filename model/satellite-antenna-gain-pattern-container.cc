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
  std::string path = "src/satellite/data/antennapatterns/";
  std::string fileName = "SatAntennaGain72Beams_";

  for (uint32_t i = 1; i <= NUMBER_OF_BEAMS; ++i)
    {
      std::ostringstream ss;
      ss << i;
      std::string filePathName = path + fileName + ss.str() + ".txt";
      Ptr<SatAntennaGainPattern> gainPattern = CreateObject<SatAntennaGainPattern> (filePathName);

      std::pair<std::map<uint32_t,Ptr<SatAntennaGainPattern> >::iterator, bool> ret;
      ret = m_antennaPatternMap.insert(std::pair<uint32_t, Ptr<SatAntennaGainPattern> >(i, gainPattern));

      if (ret.second == false)
        {
          NS_FATAL_ERROR(this << " an antenna pattern for beam " << i << " already exists!");
        }
    }
}


Ptr<SatAntennaGainPattern> SatAntennaGainPatternContainer::GetAntennaGainPattern (uint32_t beamId) const
{
  NS_LOG_FUNCTION (this << beamId);

  NS_ASSERT (beamId > 0 && beamId <= m_antennaPatternMap.size());

  // Note, that now we assume that all the antenna patterns are created
  // regardless of how many beams are actually simulated.
  return m_antennaPatternMap.at(beamId);
}


uint32_t SatAntennaGainPatternContainer::GetBestBeamId (GeoCoordinate coord) const
{
  NS_LOG_FUNCTION (this << coord.GetLatitude() << coord.GetLongitude());

  double bestGain = -100.0;
  uint32_t bestId = 0;

  for (uint32_t i = 1; i <= NUMBER_OF_BEAMS; ++i)
    {
      double gain = m_antennaPatternMap.at(i)->GetAntennaGain_lin (coord);
      if (gain > bestGain)
        {
          bestGain = gain;
          bestId = i;
        }
    }

  NS_ASSERT (bestId > 0);

  return bestId;
}

} // namespace ns3


