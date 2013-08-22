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
 * \brief Free space loss (FSL) model
 */
class SatFreeSpaceLoss : public Object
{
public:
  static TypeId GetTypeId (void);

  SatFreeSpaceLoss ();
  ~SatFreeSpaceLoss () {}

  /**
   * \return the free space loss as Watts.
   */
  virtual double GetFslW (Ptr<MobilityModel> a, Ptr<MobilityModel> b, double frequency) const;

  /**
   * \return the free space loss as dBs.
   */
  virtual double GetFsldB (Ptr<MobilityModel> a, Ptr<MobilityModel> b, double frequency) const;

private:
  virtual int64_t DoAssignStreams (int64_t stream);

  // the speed of light in m/s
  static const double C = 299792458.0;
};


} // namespace ns3

#endif /* SATELLITE_FREE_SPACE_LOSS_H */
