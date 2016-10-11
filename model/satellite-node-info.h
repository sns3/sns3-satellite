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


#ifndef SATELLITE_NODE_INFO_H_
#define SATELLITE_NODE_INFO_H_

#include <ns3/simple-ref-count.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-enums.h>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief The SatNodeInfo implements a container for all needed node related information.
 * It may be passed to all entities which need node specific information, e.g. LLC,
 * MAC and PHY layers.
 */

class SatNodeInfo : public SimpleRefCount<SatNodeInfo>
{
public:

  /**
   * Default constructor.
   */
  SatNodeInfo ();

  /**
   * Constructor with initialization parameters.
   * \param nodeType 
   * \param nodeId 
   * \param macAddress 
   */
  SatNodeInfo (SatEnums::SatNodeType_t nodeType, uint32_t nodeId, Mac48Address macAddress);

  /**
   * Destructor for SatNodeInfo
   */
  virtual ~SatNodeInfo ();

  /**
   * \brief Get node identifier
   * \return Node identifier
   */
  uint32_t GetNodeId () const;

  /**
   * \brief Get Node type (UT, SAT, GW, NCC, TER)
   * \return Node type enum
   */
  SatEnums::SatNodeType_t GetNodeType () const;

  /**
   * \brief Get MAC address
   * \return Node (SatNetDevice) MAC address
   */
  Mac48Address GetMacAddress () const;

private:
  uint32_t m_nodeId;
  SatEnums::SatNodeType_t m_nodeType;
  Mac48Address m_macAddress;

};

}

#endif /* SATELLITE_NODE_INFO_H_ */
