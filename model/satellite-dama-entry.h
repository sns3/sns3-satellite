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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SAT_DAMA_ENTRY_H
#define SAT_DAMA_ENTRY_H

#include <map>

#include "satellite-control-message.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief class for module Beam Scheduler.
 *
 * This SatDamaEntry class implements scheduler used to for one specific beam.
 * It's is created and used by NCC.
 *
 */
class SatDamaEntry
{
public:
  typedef uint32_t Cra_t;
  typedef uint32_t Rc_t;

  /**
   * Construct a SatDamaEntry
   *
   * This is the constructor for the SatDamaEntry
   *
   */
  SatDamaEntry ();

  /**
   * Destroy a SatDamaEntry
   *
   * This is the destructor for the SatDamaEntry.
   */
  ~SatDamaEntry ();

private:

  /**
   * UT identifier: MAC address currently TODO: Something else?
   */
  Address m_macAddress;
  /**
   * Negotiated guaranteed bitrate
   */
  Cra_t m_cra;

  /**
   * Dynamic SRs
   */
  std::map<Rc_t, Ptr<SatCapacityReqHeader> > m_capacityRequests;

  /**
   * Throughput in this superframe
   */
  double m_instantaneousThroughput;

  /**
   * Average throughput in the near past per RC
   */
  std::vector<double> m_pastAverageTput;

  /**
   * Maximum bitrate for this UT (UT class dependent)
   */
  double m_maxTput;
};

} // namespace ns3

#endif /* SAT_DAMA_ENTRY_H */
