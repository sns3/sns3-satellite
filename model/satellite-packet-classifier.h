/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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


#ifndef SATELLITE_PACKET_CLASSIFIER_H_
#define SATELLITE_PACKET_CLASSIFIER_H_

#include "ns3/simple-ref-count.h"
#include "satellite-enums.h"
#include "satellite-control-message.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * SatPacketClassifier is responsible of classifying packets into certain
 * flow identifier. Classification is done based on IP header DSCP field
 * value.
 * Control -> flow index 0 (Note, that this is not using IP header DSCP field)
 * Default (0) -> flow index 3
 * Assured forwarding -> flow index 2
 * Expedited forwarding -> flow index 1
 */

class SatPacketClassifier : public SimpleRefCount<SatPacketClassifier>
{
public:

  /**
   * Default constructor.
   */
  SatPacketClassifier ();

  /**
   * Destructor for SatPacketClassifier
   */
  virtual ~SatPacketClassifier ();

  /**
   * \brief Classify control message
   * \param type Control msg type
   * \param dest Destination address
   * \return uint8_t Flow identifier
   */
  uint8_t Classify (SatControlMsgTag::SatControlMsgType_t type, const Address& dest) const;

  /**
   * \brief Classify a packet based on IP header DSCP to a lower layer
   * flow id.
   * \param packet Packet to be classified
   * \param dest Destination address
   * \param protocolNumber Protocol number
   * \return uint8_t Flow identifier
   */
  uint8_t Classify (const Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) const;

private:
};

}

#endif /* SATELLITE_PACKET_CLASSIFIER_H_ */
