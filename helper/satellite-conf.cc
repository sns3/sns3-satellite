/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#include "satellite-conf.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("SatConf");

namespace ns3 {


SatConf::SatConf()
{
  // Nothing done here
}

void SatConf::Initialize (std::string path, std::string satConf, std::string gwPos, std::string satPos)
{
  // Load satellite configuration file
  LoadSatConf (path + satConf);

  // Load GW positions
  LoadGwPos (path + gwPos);

  // Load satellite position
  LoadGeoSatPos (path + satPos);
}


std::ifstream* SatConf::OpenFile (std::string filePathName)
{
  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

  if (!ifs->is_open ())
    {
      // script might be launched by test.py, try a different base path
      delete ifs;
      filePathName = "../../" + filePathName;
      ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

      if (!ifs->is_open ())
        {
          NS_FATAL_ERROR ("The file " << filePathName << " is not found.");
        }
    }
    return ifs;
}

void
SatConf::LoadSatConf (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = OpenFile (filePathName);

  uint32_t beamId, userChannelId, gwId, feederChannelId;
  *ifs >> beamId >> userChannelId >> gwId >> feederChannelId;

  while (ifs->good ())
    {
      NS_LOG_DEBUG (this <<
                    " beamId = " << beamId <<
                    ", userChannelId = " << userChannelId <<
                    ", gwId = " << gwId <<
                    ", feederChannelId = " << feederChannelId);

      // Store the values
      std::vector <uint32_t> beamConf;

      beamConf.push_back (beamId);
      beamConf.push_back (userChannelId);
      beamConf.push_back (gwId);
      beamConf.push_back (feederChannelId);

      m_conf.push_back(beamConf);

      // get next row
      *ifs >> beamId >> userChannelId >> gwId >> feederChannelId;
    }

  m_beamCount = m_conf.size ();

  ifs->close ();
  delete ifs;
}

void
SatConf::LoadGwPos (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = OpenFile (filePathName);

  double lat, lon, alt;
  *ifs >> lat >> lon >> alt;

  while (ifs->good ())
    {
      NS_LOG_DEBUG (this <<
                    " latitude [deg] = " << lat <<
                    ", longitude [deg] = " << lon <<
                    ", altitude [m] = ");

      // Store the values
      GeoCoordinate coord(lat, lon, alt);
      m_gwPositions.push_back (coord);

      // get next row
      *ifs >> lat >> lon >> alt;
    }

  m_gwCount = m_gwPositions.size ();

  ifs->close ();
  delete ifs;
}

void
SatConf::LoadGeoSatPos (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = OpenFile (filePathName);

  double lat, lon, alt;
  *ifs >> lat >> lon >> alt;

  if (ifs->good ())
    {
      NS_LOG_DEBUG (this <<
                    " latitude [deg] = " << lat <<
                    ", longitude [deg] = " << lon <<
                    ", altitude [m] = ");

      m_geoSatPosition = GeoCoordinate (lat,lon,alt);
    }

  ifs->close ();
  delete ifs;
}

uint32_t SatConf::GetBeamCount() const
{
  return m_beamCount;
}

uint32_t SatConf::GetGwCount() const
{
  return m_gwCount;
}


std::vector <uint32_t> SatConf::GetBeamConfiguration (uint32_t beamId) const
{
    NS_ASSERT((beamId > 0) && (beamId <=  m_beamCount));

    return m_conf[beamId - 1];
}

GeoCoordinate SatConf::GetGwPosition (uint32_t gwId) const
{
    NS_ASSERT((gwId > 0) && (gwId <=  m_gwCount));

    return m_gwPositions[gwId - 1];
}

GeoCoordinate SatConf::GetGeoSatPosition () const
{
    return m_geoSatPosition;
}

} // namespace ns3



