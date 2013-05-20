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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SAT_GEO_HELPER_H
#define SAT_GEO_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/deprecated.h"

#include "ns3/trace-helper.h"

namespace ns3 {

class NetDevice;
class Node;
class SatGeoNetDevice;

/**
 * \brief Build a set SatGeoNetDevice object and configuring it
 *
 * Normally we eschew multiple inheritance, however, the classes 
 * PcapUserHelperForDevice and AsciiTraceUserHelperForDevice are
 * "mixins".
 */
class SatGeoHelper : public PcapHelperForDevice, public AsciiTraceHelperForDevice
{
public:
  /**
   * Create a SatGeoHelper to make life easier when creating Satellite point to
   * point network connections.
   */
  SatGeoHelper ();
  virtual ~SatGeoHelper () {}

  /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::SatNetDevice created
   * by SatGeoHelper::Install
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * \param c a set of nodes
   * \param beamId  id of the beam
   *
   * This method creates a ns3::SatGeoNetDevices with the requested attributes
   * and associate the resulting ns3::NetDevices with the ns3::Nodes.
   */
  NetDeviceContainer Install (NodeContainer c, uint16_t beamId);

  /**
   * \param n a node
   * \param beamId  id of the beam
   *
   * This method creates a ns3::SatGeoNetDevice with the requested attributes
   * and associate the resulting ns3::NetDevice with the ns3::Node.
   */
  Ptr<NetDevice> Install (Ptr<Node> n, uint16_t beamId);

  /**
   * \param nName name of a node
   * \param beamId  id of the beam
   *
   * This method creates a ns3::SatGeoNetDevice with the requested attributes
   * and associate the resulting ns3::NetDevice with the ns3::Node.
   */
  Ptr<NetDevice> Install (std::string nName, uint16_t beamId);

  /*
   * Attach the SatChannels for the beam to NetDevice
   * \param dev SatGeoNetDevice to attact channels
   * \param fr feeder return channel
   * \param uf user forward channel
   * \param uf user return channel
   * \param beamId beam Id of the attachment
   */
  void AttachChannels ( Ptr<SatGeoNetDevice> dev, Ptr<SatChannel> ff, Ptr<SatChannel> fr,
                        Ptr<SatChannel> uf, Ptr<SatChannel> ur, uint16_t beamId );


private:
  /**
   * \brief Enable pcap output the indicated net device.
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

  /**
   * \brief Enable ascii trace output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnableAsciiInternal (
    Ptr<OutputStreamWrapper> stream,
    std::string prefix,
    Ptr<NetDevice> nd,
    bool explicitFilename);

    // count for devices. Currently only one device supported by helper.
    uint16_t m_deviceCount;

    ObjectFactory m_deviceFactory;
};

} // namespace ns3

#endif /* SAT_GEO_HELPER_H */
