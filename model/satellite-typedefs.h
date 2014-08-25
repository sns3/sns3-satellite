/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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

#ifndef SATELLITE_TYPEDEFS_H_
#define SATELLITE_TYPEDEFS_H_

#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatTypedefs class is for general typedefs used in satellite module.
 */
class SatTypedefs
{
public:

  /**
   * \brief Callback for carrier bandwidths
   * \param channelType     The type of the channel
   * \param carrierId       The id of the carrier
   * \param bandwidthType   The type of the bandwidth
   *
   * \return The bandwidth of the carrier.
   */
  typedef Callback<double, SatEnums::ChannelType_t, uint32_t, SatEnums::CarrierBandwidthType_t > CarrierBandwidthConverter_t;


private:

  /**
   * Destructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatTypedefs () = 0;
};


} // namespace ns3


#endif /* SATELLITE_TYPEDEFS_H_ */
