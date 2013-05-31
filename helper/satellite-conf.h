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
   std::vector <uint32_t> GetBeamConfiguration (uint32_t beamId) const;

private:

  /*
   *  Columns:
   *  1. Beam id
   *  2. User frequency id
   *  3. GW id
   *  4. Feeder frequency id
   */
   static const uint32_t m_conf[99][4];

};


} // namespace ns3


#endif /* SAT_CONF_H */
