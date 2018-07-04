/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions Ltd
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

#ifndef SAT_STATIC_BSTP_H
#define SAT_STATIC_BSTP_H

#include <vector>
#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatStaticBstp class models the static beam switching
 * time plan (BSTP) configuration, which is defined in an external
 * file read by this class.
 *
 * The BSTP configuration file holds multiple lines of the following
 * content:
 *   1_col, 2_col, 3_col, 4_col, ... n_col
 *
 * The first column defines the validity of a BSTP configuration
 * (this line) in DVB-S2x superframes. The following columns define
 * which beams are enabled in this BSTP window. The ones not mentioned
 * are disabled. Each line of the BSTP configuration file may hold
 * different amount of enabled spot-beams.
 *
 * The BSTP configuration file may also hold several configuration
 * lines. Each will define different BSTP configuration and may have
 * different validity. When all the lines of a BSTP has been gone through,
 * we start again from the first BSTP configuration file. Thus, the
 * BSTP may be considered as being a pre-defined spot-beam enabling
 * pattern.
 *
 */
class SatStaticBstp : public SimpleRefCount<SatStaticBstp>
{
public:

  /**
   * Default constructor.
   */
  SatStaticBstp ();
  SatStaticBstp (std::string fileName);
  virtual ~SatStaticBstp () { }

  /**
   * \brief Load BSTP configuration from a file
   * \param filePathName
   */
  void LoadBstp (std::string filePathName);

  /**
   * \brief Get the next configuration file
   * \return A unsigned int configuration vector
   */
  std::vector<uint32_t> GetNextConf () const;

  /**
   * \brief Add the information about which spot-beams are enabled
   * in this simulation. This information is stored and used to check
   * the validity of the BSTP.
   * \param beamId Enabled beam identifier
   * \param userFreqId User frequency id of the enabled spot-beam
   * \param feederFreqId Feeder frequency id of the enabled spot-beam
   * \param gwId GW id of the enabled spot-beam
   */
  void AddEnabledBeamInfo (uint32_t beamId,
                           uint32_t userFreqId,
                           uint32_t feederFreqId,
                           uint32_t gwId);

  /**
   * \brief Check validity of the individual BSTP configuration line.
   * The method is not returning anything, but it crashes to a fatal
   * message if the BSTP is not considered to be valid!
   */
  void CheckValidity ();

private:

  std::vector <std::vector <uint32_t> > m_bstp;
  mutable uint32_t m_currentIterator;

  // Helpful containers for checking the validity of the BSTP
  std::map<uint32_t, uint32_t> m_beamGwMap;
  std::map<uint32_t, uint32_t> m_beamFeederFreqIdMap;

  // All enabled spot-beams
  std::vector<uint32_t> m_enabledBeams;
};


} // namespace ns3


#endif /* SAT_STATIC_BSTP_H */
