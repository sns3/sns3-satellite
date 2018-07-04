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
 * \brief Static beam switching time plan configuration
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
   * Try to open a file from a given path
   * \param filePathName
   */
  std::ifstream* OpenFile (std::string filePathName);

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

private:

  std::vector <std::vector <uint32_t> > m_bstp;
  mutable uint32_t m_currentIterator;

};


} // namespace ns3


#endif /* SAT_STATIC_BSTP_H */
