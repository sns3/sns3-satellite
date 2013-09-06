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
#include "ns3/output-stream-wrapper.h"
#include "ns3/error-model.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/traced-callback.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-phy-rx-carrier-conf.h"

namespace ns3 {

/**
 * \brief Build a set SatGeoNetDevice object and configuring it
 *
 */
class SatGeoHelper : public Object
{
public:
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
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
   *
   * This method creates a ns3::SatGeoNetDevices with the requested attributes
   * and associate the resulting ns3::NetDevices with the ns3::Nodes.
   */
  NetDeviceContainer Install (NodeContainer c);

  /**
   * \param n a node
   *
   * This method creates a ns3::SatGeoNetDevice with the requested attributes
   * and associate the resulting ns3::NetDevice with the ns3::Node.
   */
  Ptr<NetDevice> Install (Ptr<Node> n);

  /**
   * \param nName name of a node
   *
   * This method creates a ns3::SatGeoNetDevice with the requested attributes
   * and associate the resulting ns3::NetDevice with the ns3::Node.
   */
  Ptr<NetDevice> Install (std::string nName);

  /*
   * Attach the SatChannels for the beam to NetDevice
   * \param dev NetDevice to attach channels
   * \param fr feeder return channel
   * \param uf user forward channel
   * \param uf user return channel
   * \param beamId Id of the beam
   */
  void AttachChannels ( Ptr<NetDevice> dev, Ptr<SatChannel> ff, Ptr<SatChannel> fr,
                        Ptr<SatChannel> uf, Ptr<SatChannel> ur, uint32_t beamId);

  /**
   * Enables creation traces to be written in given file
   * /param stream  stream for creation trace outputs
   * /param cb  callback to connect traces
   */
  void EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb);


private:
    // count for devices. Currently only one device supported by helper.
    uint16_t m_deviceCount;

    ObjectFactory m_deviceFactory;
    ObjectFactory m_userPhyFactory;
    ObjectFactory m_feederPhyFactory;

    /*
     * Configured forward link interference model
     */
    SatPhyRxCarrierConf::InterferenceModel m_fwdLinkInterferenceModel;

    /*
     * Configured return link interference model
     */
    SatPhyRxCarrierConf::InterferenceModel m_rtnLinkInterferenceModel;

    /*
     * Configured RX noise temperature (in Kelvins) for return link.
     */
    double m_rtnLinkRxTemperature_K;

    /*
     * Configured RX noise temperature (in Kelvins) for forward link.
     */
    double m_fwdLinkRxTemperature_K;

    /*
     * Configured other system noise (in Watt) for return link.
     */
    double m_rtnLinkOtherSysNoise_W;

    /*
     * Configured other system noise (in Watt) for forward link.
     */
    double m_fwdLinkOtherSysNoise_W;

    /**
     * Trace callback for creation traces
     */
    TracedCallback<std::string> m_creation;
};

} // namespace ns3

#endif /* SAT_GEO_HELPER_H */
