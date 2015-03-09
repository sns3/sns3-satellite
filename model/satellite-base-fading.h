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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#ifndef SATELLITE_BASE_FADING_H
#define SATELLITE_BASE_FADING_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-enums.h"
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Base class for fading models such as Markov-based fading
 * or fading trace. This class defines the functions which all the
 * different fading models must implement for the fading
 * interface. This base class itself is abstract and will not
 * implement any real functionality.
 */
class SatBaseFading : public Object
{
public:
  /**
   * \brief Gets elevation angle in degrees
   */
  typedef Callback<double> ElevationCallback;

  /**
   * \brief Gets velocity in m/s
   */
  typedef Callback<double> VelocityCallback;

  /**
   * \brief Constructor
   */
  SatBaseFading ();

  /**
   * \brief Destructor
   */
  virtual ~SatBaseFading ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for getting the fading value.
   * \param macAddress
   * \param channelType channel type
   * \return fading value
   */
  double GetFading (Address macAddress, SatEnums::ChannelType_t channelType);

  /**
   * \brief Function for getting the fading value. This is an abstract method,
   * thus it is implemented in the inherited fading classes.
   * \param macAddress
   * \param channelType channel type
   * \return fading value
   */
  virtual double DoGetFading (Address macAddress, SatEnums::ChannelType_t channelType) = 0;

private:
};

} // namespace ns3

#endif /* SATELLITE_BASE_FADING_H */
