/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Inspired and adapted from Hypatia: https://github.com/snkas/hypatia
 *
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_ISL_ARBITER_H
#define SATELLITE_ISL_ARBITER_H

#include <ns3/mac48-address.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/satellite-geo-net-device.h>

#include <string>

namespace ns3
{

class SatIslArbiter : public Object
{
  public:
    static TypeId GetTypeId(void);

    /**
     * Default constructor.
     */
    SatIslArbiter();

    /**
     * Constructor.
     * \param node The satellite node this arbiter is attached
     */
    SatIslArbiter(Ptr<Node> node);

    /**
     * Base decide how to forward
     *
     * \param pkt                               Packet
     * \param destination                       IP header of the packet
     *
     * \return ISL interface index, or -1 if routing failed
     */
    int32_t BaseDecide(Ptr<Packet> pkt, Mac48Address destination);

    /**
     * Decide how to forward. Implemented in subclasses
     *
     * \param sourceSatId                       Satellite ID where the packet originated from
     * \param targetSatId                       Satellite ID where the packet has to go to
     * \param pkt                               Packet
     *
     * \return ISL interface index, or -1 if routing failed
     */
    virtual int32_t Decide(int32_t sourceSatId, int32_t targetSatId, Ptr<Packet> pkt) = 0;

    /**
     * Convert the forwarding state (i.e., routing table) to a string representation.
     *
     * @return String representation
     */
    virtual std::string StringReprOfForwardingState() = 0;

  protected:
    uint32_t m_nodeId; // ID of node where this arbiter is installed.
};

} // namespace ns3

#endif // SATELLITE_ISL_ARBITER_H
