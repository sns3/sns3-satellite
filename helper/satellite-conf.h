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

#ifndef SAT_CONF_H
#define SAT_CONF_H

#include "ns3/uinteger.h"
#include <vector>

namespace ns3 {

/**
 * \brief A configuration class for the 98-beam GEO satellite reference system
 *
 */
class SatConf
{
public:

  SatConf ();
  virtual ~SatConf () {}

  /**
   * Get the configuration vector for a given satellite beam id
   *
   * \param beam id
   */
   uint32_t GetBeamCount () const;


  /**
   * Get the configuration vector for a given satellite beam id
   *
   * \param beam id
   */
   std::vector <uint32_t> GetBeamConfiguration (uint32_t beamId) const;

   /**
    * Definition for beam ID index (column) in m_conf
    */
   static const uint32_t BEAM_ID_INDEX = 0;

   /**
    * Definition for user frequency ID index (column) in m_conf
    */
   static const uint32_t U_FREQ_ID_INDEX = 1;

   /**
    * Definition for GW ID index (column) in m_conf
    */
   static const uint32_t GW_ID_INDEX = 2;

   /**
    * Definition for feeder frequency ID index (column) in m_conf
    */
   static const uint32_t F_FREQ_ID_INDEX = 3;
   static const uint32_t DEFAULT_BEAM_COUNT = 98;
   static const uint32_t BEAM_ELEM_COUNT = 4;

private:

  /*
   *  Columns:
   *  1. Beam id
   *  2. User frequency id
   *  3. GW id
   *  4. Feeder frequency id
   */
   std::vector <std::vector <uint32_t> > m_conf;

   /**
    * Beam count.
    */
   uint32_t m_beamCount;

};


} // namespace ns3


#endif /* SAT_CONF_H */
