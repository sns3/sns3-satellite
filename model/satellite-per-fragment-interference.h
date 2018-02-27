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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_PER_FRAGMENT_INTERFERENCE_H
#define SATELLITE_PER_FRAGMENT_INTERFERENCE_H

#include "satellite-per-packet-interference.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Packet fragment by packet fragment interference. Interference
 * is calculated separately for each fragment of a packet depending of
 * how other interferent packets overlaps.
 */
class SatPerFragmentInterference : public SatPerPacketInterference
{
public:
  /**
   * Inherited from ObjectBase base class.
   */
  static TypeId GetTypeId ();

  /**
   * Inherited from ObjectBase base class.
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor
   */
  SatPerFragmentInterference ();

  /**
   * Constructor to listen on a specific SatChannel.
   */
  SatPerFragmentInterference (SatEnums::ChannelType_t channelType, double rxBandwidthHz);

  /**
   * Destructor
   */
  ~SatPerFragmentInterference ();
};

}  // namespace ns3

#endif  // SATELLITE_PER_FRAGMENT_INTERFERENCE_H
