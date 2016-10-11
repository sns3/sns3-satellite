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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */
#ifndef SATELLITE_FREE_SPACE_LOSS_H
#define SATELLITE_FREE_SPACE_LOSS_H

#include "ns3/object.h"
#include "ns3/mobility-model.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief This class is responsible of calculating the free space loss (FSL)
 * based on two given node positions and used (center) frequency.
 */
class SatFreeSpaceLoss : public Object
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatFreeSpaceLoss ();
  ~SatFreeSpaceLoss ()
  {
  }

  /**
   * \brief Calculate the free-space loss in linear format
   * \param a Mobility model of node a
   * \param b Mobility model of node b
   * \param frequencyHz Frequency in Hertz
   * \return the free space loss as ratio.
   */
  virtual double GetFsl (Ptr<MobilityModel> a, Ptr<MobilityModel> b, double frequencyHz) const;

  /**
   * \brief Calculate the free-space loss in dB
   * \param a Mobility model of node a
   * \param b Mobility model of node b
   * \param frequencyHz Frequency in Hertz
   * \return the free space loss as dBs.
   */
  virtual double GetFsldB (Ptr<MobilityModel> a, Ptr<MobilityModel> b, double frequencyHz) const;
};


} // namespace ns3

#endif /* SATELLITE_FREE_SPACE_LOSS_H */
