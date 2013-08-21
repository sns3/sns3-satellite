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

/*
 * Beam id, User frequency id, GW id, Feeder frequency id
 */
static const uint32_t g_DefaultConf[SatConf::DEFAULT_BEAM_COUNT][SatConf::BEAM_ELEM_COUNT] = {
                                        {1, 1, 4, 1,},
                                        {2, 2, 6, 1,},
                                        {3, 4, 6, 2,},
                                        {4, 1, 5, 1,},
                                        {5, 2, 5, 2,},
                                        {6, 3, 5, 3,},
                                        {7, 3, 4, 2,},
                                        {8, 4, 4, 3,},
                                        {9, 2, 4, 4,},
                                        {10, 3, 4, 5,},
                                        {11, 4, 6, 3,},
                                        {12, 1, 6, 4,},
                                        {13, 2, 6, 5,},
                                        {14, 3, 6, 6,},
                                        {15, 4, 6, 7,},
                                        {16, 1, 6, 8,},
                                        {17, 3, 5, 4,},
                                        {18, 4, 5, 5,},
                                        {19, 1, 5, 6,},
                                        {20, 2, 5, 7,},
                                        {21, 3, 5, 8,},
                                        {22, 4, 4, 6,},
                                        {23, 1, 4, 7,},
                                        {24, 2, 4, 8,},
                                        {25, 3, 4, 9,},
                                        {26, 4, 4, 10,},
                                        {27, 1, 4, 11,},
                                        {28, 2, 6, 9,},
                                        {29, 3, 6, 10,},
                                        {30, 4, 6, 11,},
                                        {31, 1, 6, 12,},
                                        {32, 2, 6, 13,},
                                        {33, 3, 6, 14,},
                                        {34, 1, 5, 9,},
                                        {35, 2, 5, 10,},
                                        {36, 3, 5, 11,},
                                        {37, 4, 5, 12,},
                                        {38, 1, 5, 13,},
                                        {39, 2, 4, 12,},
                                        {40, 3, 4, 13,},
                                        {41, 4, 4, 14,},
                                        {42, 1, 4, 15,},
                                        {43, 2, 4, 16,},
                                        {44, 3, 1, 1,},
                                        {45, 4, 1, 2,},
                                        {46, 1, 1, 3,},
                                        {47, 2, 1, 4,},
                                        {48, 3, 6, 15,},
                                        {49, 4, 6, 16,},
                                        {50, 4, 3, 1,},
                                        {51, 1, 3, 2,},
                                        {52, 2, 3, 3,},
                                        {53, 3, 2, 1,},
                                        {54, 4, 2, 2,},
                                        {55, 1, 2, 3,},
                                        {56, 2, 2, 4,},
                                        {57, 3, 2, 5,},
                                        {58, 4, 1, 5,},
                                        {59, 1, 1, 6,},
                                        {60, 2, 1, 7,},
                                        {61, 3, 1, 8,},
                                        {62, 2, 3, 4,},
                                        {63, 3, 3, 5,},
                                        {64, 4, 3, 6,},
                                        {65, 1, 2, 6,},
                                        {66, 2, 2, 7,},
                                        {67, 3, 2, 8,},
                                        {68, 4, 2, 9,},
                                        {69, 1, 2, 10,},
                                        {70, 2, 1, 9,},
                                        {71, 3, 1, 10,},
                                        {72, 4, 1, 11,},
                                        {73, 1, 1, 12,},
                                        {74, 4, 3, 7,},
                                        {75, 1, 3, 8,},
                                        {76, 2, 3, 9,},
                                        {77, 3, 2, 11,},
                                        {78, 4, 2, 12,},
                                        {79, 1, 2, 13,},
                                        {80, 2, 2, 14,},
                                        {81, 3, 2, 15,},
                                        {82, 4, 1, 13,},
                                        {83, 1, 1, 14,},
                                        {84, 2, 1, 15,},
                                        {85, 3, 3, 10,},
                                        {86, 4, 3, 11,},
                                        {87, 1, 7, 1,},
                                        {88, 2, 7, 2,},
                                        {89, 3, 7, 3,},
                                        {90, 4, 7, 4,},
                                        {91, 1, 7, 5,},
                                        {92, 2, 7, 6,},
                                        {93, 3, 7, 7,},
                                        {94, 4, 7, 8,},
                                        {95, 1, 7, 9,},
                                        {96, 2, 7, 10,},
                                        {97, 3, 7, 11,},
                                        {98, 4, 7, 12,} };

static const GeoCoordinate g_defaultGwPos[SatConf::DEFAULT_GW_COUNT] = {
                                          GeoCoordinate(54.689444, 25.28, 0.00),      // GW id: 1; Vilna, Lithuania; Position: 54.41.22 N / 25.16.48 E
                                          GeoCoordinate(49.447778, 11.068056, 0.00),  // GW id: 2; Nurnberg, Germany; Position: 49.26.52 N / 11.4.5 E
                                          GeoCoordinate(52.25, -0.883333, 0.00),      // GW id: 3; Northampton, Great Britain; Position: 52.15.0 N / 0.53.0 W
                                          GeoCoordinate(41.9, 12.483333, 0.00),       // GW id: 4; Roma, Italy; Position: 41.54.0 N / 12.29.0 E
                                          GeoCoordinate(37.883333, -4.766667, 0.00),  // GW id: 5; Coroba, Spain; Position: 37.53.0 N / 4.46.0 W
                                          GeoCoordinate(39.927222, 32.864167, 0.00),  // GW id: 6; Ankara, Turkey; Position: 39.55.38 N / 32.51.51 E
                                          GeoCoordinate(63.183333, 14.65, 0.00) };    // GW id: 7; Östersund, Sweden; Position: 63.11.0 N / 14.39.0 E


static const GeoCoordinate g_defaultGeoSatPos = GeoCoordinate(33.0, 0, 35786);

SatConf::SatConf()
  :m_beamCount(DEFAULT_BEAM_COUNT),
   m_gwCount(SatConf::DEFAULT_GW_COUNT)
{
  for (uint32_t i = 0; i < DEFAULT_BEAM_COUNT; i++)
    {
      std::vector <uint32_t> beamConf;

      beamConf.push_back (g_DefaultConf[i][BEAM_ID_INDEX]);
      beamConf.push_back (g_DefaultConf[i][U_FREQ_ID_INDEX]);
      beamConf.push_back (g_DefaultConf[i][GW_ID_INDEX]);
      beamConf.push_back (g_DefaultConf[i][F_FREQ_ID_INDEX]);

      m_conf.push_back(beamConf);
    }

  for ( uint32_t j = 0; j < DEFAULT_GW_COUNT; j++ )
    {
      m_gwPositions.push_back(g_defaultGwPos[j]);
    }

  m_geoSatPosition = g_defaultGeoSatPos;
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



