/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef SATELLITE_LOOK_UP_TABLE_H
#define SATELLITE_LOOK_UP_TABLE_H

#include <fstream>
#include <vector>

#include "ns3/object.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Loads a link result file and provide query service for BLER.
 */
class SatLookUpTable : public Object
{
public:

  /**
   * Constructor with initialization parameters.
   * \param linkResultPath 
   */
  SatLookUpTable (std::string linkResultPath);

  /**
   * Destructor for SatLookUpTable
   */
  virtual ~SatLookUpTable ();

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the BLER corresponding to a given SINR
   * \param sinrDb SINR in logarithmic scale
   * \return BLER
   */
  double GetBler (double sinrDb) const;

  /**
   * \brief Get Es/No in dB for a given BLER target
   * \param blerTarget BLER target (0-1)
   * \return Es/No target in dB
   */
  double GetEsNoDb (double blerTarget) const;

private:
  virtual void DoDispose ();

  /**
   * \brief Load the link results
   * \param linkResultsPath Path to a link results file.
   */
  void Load (std::string linkResultPath);

  std::vector<double> m_esNoDb;
  std::vector<double> m_bler;
  std::ifstream *m_ifs;
};

} // end of namespace ns3

#endif /* SATELLITE_LOOK_UP_TABLE_H */
