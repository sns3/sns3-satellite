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

#ifndef SATELLITE_UT_HELPER_H
#define SATELLITE_UT_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/traced-callback.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-link-results.h"
#include "ns3/satellite-ncc.h"
#include "ns3/satellite-superframe-sequence.h"
#include "ns3/satellite-mac.h"
#include "ns3/satellite-random-access-container.h"
#include "ns3/satellite-random-access-container-conf.h"
#include "ns3/satellite-typedefs.h"

namespace ns3 {

/**
 * \brief Creates needed objects for UT nodes like SatGeoNetDevice objects.
 *        Handles needed configuration for the UT nodes.
 *
 */
class SatUtHelper : public Object
{
public:

  /**
   * Define RandomAccessSettings as a struct
   */
  typedef struct
  {
    SatEnums::RandomAccessModel_t m_randomAccessModel;
    SatPhyRxCarrierConf::InterferenceModel m_raInterferenceModel;
    SatPhyRxCarrierConf::RandomAccessCollisionModel m_raCollisionModel;
  } RandomAccessSettings_s;

  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor
   */
  SatUtHelper ();

  /**
   * Create a SatUtHelper to make life easier when creating Satellite point to
   * point network connections.
   */
  SatUtHelper (SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
               uint32_t rtnLinkCarrierCount,
               Ptr<SatSuperframeSeq> seq,
               SatMac::ReadCtrlMsgCallback readCb,
               SatMac::ReserveCtrlMsgCallback reserveCb,
               SatMac::SendCtrlMsgCallback sendCb,
               RandomAccessSettings_s randomAccessSettings);

  /**
   * Destructor
   */
  virtual ~SatUtHelper ()
  {
  }

  /*
   * Initializes the UT helper based on attributes
   * \param lrS2 DVB-S2 link results
   */
  void Initialize (Ptr<SatLinkResultsDvbS2> lrS2);

  /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::SatNetDevice created
   * by SatUtHelper::Install
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each Channel created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attribute on each ns3::SatChannel created
   * by SatUtHelper::Install
   */
  void SetChannelAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each Phy created by the helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::SatNetDevice created
   * by SatUtHelper::Install
   */
  void SetPhyAttribute (std::string name, const AttributeValue &value);

  /**
   * \param c a set of nodes
   * \param beamId  id of the beam
   * \param fCh forward channel
   * \param rCh return channel
   * \param gwNd satellite netdevice of the GW
   * \param ncc NCC (Network Control Center)
   * \return Container of installed net devices
   *
   * This method creates a ns3::SatChannel with the
   * attributes configured by SatUtHelper::SetChannelAttribute,
   * then, for each node in the input container, we create a
   * ns3::SatNetDevice with the requested attributes,
   * a queue for this ns3::NetDevice, and associate the resulting
   * ns3::NetDevice with the ns3::Node and ns3::SatChannel.
   */
  NetDeviceContainer Install (NodeContainer c, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc);

  /**
   * \param n node
   * \param beamId  id of the beam
   * \param fCh forward channel
   * \param rCh return channel
   * \param gwNd satellite netdevice of the GW
   * \param ncc NCC (Network Control Center)
   * \return Net device installed to node
   *
   * Saves you from having to construct a temporary NodeContainer.
   */
  Ptr<NetDevice> Install (Ptr<Node> n, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc);

  /**
   * Enables creation traces to be written in given file
   * \param stream  stream for creation trace outputs
   * \param cb  callback to connect traces
   */
  void EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb);

private:
  SatTypedefs::CarrierBandwidthConverter_t m_carrierBandwidthConverter;
  uint32_t m_fwdLinkCarrierCount;
  Ptr<SatSuperframeSeq> m_superframeSeq;

  // Control message container callbacks
  SatMac::ReadCtrlMsgCallback     m_readCtrlCb;
  SatMac::ReserveCtrlMsgCallback  m_reserveCtrlCb;
  SatMac::SendCtrlMsgCallback     m_sendCtrlCb;

  ObjectFactory m_channelFactory;
  ObjectFactory m_deviceFactory;

  /*
   * Configured dedicated access interference model for the forward link. Set as an attribute.
   */
  SatPhy::InterferenceModel m_daInterferenceModel;

  /*
   * Configured error model for the forward link. Set as an attribute.
   */
  SatPhy::ErrorModel m_errorModel;

  /*
   * Constant error rate for dedicated access in the FWD link.
   */
  double m_daConstantErrorRate;

  /*
   * Forward channel link results (DVB-S2) are created if ErrorModel
   * is configured to be AVI. Note, that only one instance of the
   * link results is needed for all UTs.
   */
  Ptr<SatLinkResults> m_linkResults;

  /**
   * \brief Trace callback for creation traces
   */
  TracedCallback<std::string> m_creationTrace;

  /**
   * Configured lower layer service configuration.
   */
  Ptr<SatLowerLayerServiceConf> m_llsConf;

  /**
   * Enable channel estimation error modeling at forward link
   * receiver (= UT).
   */
  bool m_enableChannelEstimationError;

  /**
   * Planned CRDSA usage:
   * - true -> only for control
   * - false -> for control and user data
   */
  bool m_crdsaOnlyForControl;

  /**
   * The used random access model settings
   */
  RandomAccessSettings_s m_raSettings;
};

} // namespace ns3

#endif /* SATELLITE_UT_HELPER_H */
