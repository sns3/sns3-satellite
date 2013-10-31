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
#ifndef SATELLITE_FADING_H
#define SATELLITE_FADING_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-channel.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Base class for fading
 */
class SatFading : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatFading ();

  /**
   * \brief Destructor
   */
  virtual ~SatFading ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for getting the fading value
   * \param channelType channel type
   * \return fading value
   */
  virtual double GetFading (SatChannel::ChannelType_t channelType) = 0;

private:

};

} // namespace ns3

#endif /* SATELLITE_FADING_H */
