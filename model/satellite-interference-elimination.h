/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_INTERFERENCE_ELIMINATION_H
#define SATELLITE_INTERFERENCE_ELIMINATION_H

#include <ns3/object.h>


namespace ns3 {

class SatSignalParameters;

/**
 * \ingroup satellite
 * \brief Abstract class defining interface for interference elimination calculation objects
 */
class SatInterferenceElimination : public Object
{
public:
  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * Constructor for Satellite interference elimination base class
   */
  SatInterferenceElimination ();

  /**
   * Destructor for Satellite interference elimination base class
   */
  virtual ~SatInterferenceElimination ();

  /**
   * Calculate residual power of interference for the given packets
   * \param packetInterferedWith Parameters of the packet whose interference level should be lowered
   * \param processedPacket Parameters of the packet we want to remove interference from
   * \param isRegenerative Tell if we are in regenerative configuration. Depending on this value, the computations are made on the GW or on the satellite
   */
  virtual void EliminateInterferences (Ptr<SatSignalParameters> packetInterferedWith, Ptr<SatSignalParameters> processedPacket, double EsNo, bool isRegenerative) = 0;

  /**
   * Calculate residual power of interference for the given packets
   * \param packetInterferedWith Parameters of the packet whose interference level should be lowered
   * \param processedPacket Parameters of the packet we want to remove interference from
   * \param isRegenerative Tell if we are in regenerative configuration. Depending on this value, the computations are made on the GW or on the satellite
   * \param startTime Normalized start time of the interference, with respect to packetInterferedWith
   * \param startTime Normalized end time of the interference, with respect to packetInterferedWith
   */
  virtual void EliminateInterferences (Ptr<SatSignalParameters> packetInterferedWith, Ptr<SatSignalParameters> processedPacket, double EsNo, bool isRegenerative, double startTime, double endTime) = 0;

  /**
   * \brief Calculate the residual power of interference for a given packet
   */
  virtual double GetResidualPower (Ptr<SatSignalParameters> processedPacket, double EsNo) = 0;

};

} // namespace ns3

#endif /* SATELLITE_INTERFERENCE_ELIMINATION_H */
