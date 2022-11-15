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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 * Author: Mathias Ettinger <mettinger@viveris.toulouse.fr>
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
#include "ns3/satellite-phy.h"
#include "ns3/satellite-geo-feeder-mac.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-superframe-sequence.h"
#include "ns3/satellite-typedefs.h"
#include "ns3/satellite-bbframe-conf.h"
#include "ns3/satellite-fwd-link-scheduler.h"
#include "ns3/satellite-scpc-scheduler.h"

namespace ns3 {

/**
 * \brief Creates needed objects for Geo Satellite node like SatGeoNetDevice objects.
 *        Handles needed configuration for the Geo Satellite node.
 *
 */
class SatGeoHelper : public Object
{
public:
  /**
   * Random access setting options.
   */
  typedef struct
  {
    SatEnums::RandomAccessModel_t m_randomAccessModel;
    SatPhyRxCarrierConf::InterferenceModel m_raFwdInterferenceModel;
    SatPhyRxCarrierConf::InterferenceModel m_raRtnInterferenceModel;
    SatPhyRxCarrierConf::InterferenceEliminationModel m_raInterferenceEliminationModel;
    SatPhyRxCarrierConf::RandomAccessCollisionModel m_raCollisionModel;
  } RandomAccessSettings_s;

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor.
   */
  SatGeoHelper ();

  /**
   * Create a SatGeoHelper to make life easier when creating Satellite point to
   * point network connections.
   */
  SatGeoHelper (SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                uint32_t rtnLinkCarrierCount,
                uint32_t fwdLinkCarrierCount,
                Ptr<SatSuperframeSeq> seq,
                SatMac::ReadCtrlMsgCallback fwdReadCb,
                SatMac::ReadCtrlMsgCallback rtnReadCb,
                RandomAccessSettings_s randomAccessSettings);

  virtual ~SatGeoHelper ()
  {
  }

  /*
   * Initializes the GEO helper based on attributes.
   * Link results are used only if satellite is regenerative.
   * \param lrFwd DVB-S2 or DVB-S2X link results
   * \param lrRcs2 return link results
   */
  void Initialize (Ptr<SatLinkResultsFwd> lrFwd, Ptr<SatLinkResultsRtn> lrRcs2);

  /**
   * Set an attribute value to be propagated to each NetDevice created by the
   * helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::SatGeoNetDevice created
   * by SatGeoHelper::Install
   */
  void SetDeviceAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each User Phy created by the helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::SatPhy (user) created
   * by SatGeoHelper::Install
   */
  void SetUserPhyAttribute (std::string name, const AttributeValue &value);

  /**
   * Set an attribute value to be propagated to each Feeder Phy created by the helper.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   *
   * Set these attributes on each ns3::SatPhy (feeder) created
   * by SatGeoHelper::Install
   */
  void SetFeederPhyAttribute (std::string name, const AttributeValue &value);

  /**
   * \param c a set of nodes
   *
   * This method creates a ns3::SatGeoNetDevices with the requested attributes
   * and associate the resulting ns3::NetDevices with the ns3::Nodes.
   *
   * \return container to the created devices
   */
  NetDeviceContainer Install (NodeContainer c);

  /**
   * \param n a node
   *
   * This method creates a ns3::SatGeoNetDevice with the requested attributes
   * and associate the resulting ns3::NetDevice with the ns3::Node.
   *
   * \return pointer to the created device
   */
  Ptr<NetDevice> Install (Ptr<Node> n);

  /**
   * \param aName name of a node
   *
   * This method creates a ns3::SatGeoNetDevice with the requested attributes
   * and associate the resulting ns3::NetDevice with the ns3::Node.
   *
   * \return pointer to the created device
   */
  Ptr<NetDevice> Install (std::string aName);

  /*
   * Attach the SatChannels for the beam to NetDevice
   * \param dev NetDevice to attach channels
   * \param fr feeder return channel
   * \param uf user forward channel
   * \param uf user return channel
   * \param userAgp user beam antenna gain pattern
   * \param feederAgp feeder beam antenna gain pattern
   * \param gwId ID of GW associated to this channel
   * \param userBeamId Id of the beam
   * \param forwardLinkRegenerationMode Regeneration mode on forward
   * \param returnLinkRegenerationMode Regeneration mode on return
   */
  void AttachChannels ( Ptr<NetDevice> dev,
                        Ptr<SatChannel> ff,
                        Ptr<SatChannel> fr,
                        Ptr<SatChannel> uf,
                        Ptr<SatChannel> ur,
                        Ptr<SatAntennaGainPattern> userAgp,
                        Ptr<SatAntennaGainPattern> feederAgp,
                        uint32_t gwId,
                        uint32_t userBeamId,
                        SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                        SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /*
   * Attach the SatChannels for the beam to NetDevice
   * \param dev NetDevice to attach channels
   * \param fr feeder forward channel
   * \param fr feeder return channel
   * \param feederAgp feeder beam antenna gain pattern
   * \param gwId ID of GW associated to this channel
   * \param userBeamId Id of the beam
   * \param forwardLinkRegenerationMode Regeneration mode on forward
   * \param returnLinkRegenerationMode Regeneration mode on return
   */
  void AttachChannelsFeeder ( Ptr<SatGeoNetDevice> dev,
                              Ptr<SatChannel> ff,
                              Ptr<SatChannel> fr,
                              Ptr<SatAntennaGainPattern> feederAgp,
                              uint32_t gwId,
                              uint32_t userBeamId,
                              SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                              SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /*
   * Attach the SatChannels for the beam to NetDevice
   * \param dev NetDevice to attach channels
   * \param uf user forward channel
   * \param uf user return channel
   * \param userAgp user beam antenna gain pattern
   * \param userBeamId Id of the beam
   * \param forwardLinkRegenerationMode Regeneration mode on forward
   * \param returnLinkRegenerationMode Regeneration mode on return
   */
  void AttachChannelsUser ( Ptr<SatGeoNetDevice> dev,
                            Ptr<SatChannel> uf,
                            Ptr<SatChannel> ur,
                            Ptr<SatAntennaGainPattern> userAgp,
                            uint32_t userBeamId,
                            SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                            SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /**
   * Enables creation traces to be written in given file
   * \param stream  stream for creation trace outputs
   * \param cb  callback to connect traces
   */
  void EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb);


private:
  /**
   * GEO satellite node id
   */
  uint32_t m_nodeId;

  SatTypedefs::CarrierBandwidthConverter_t m_carrierBandwidthConverter;
  uint32_t m_fwdLinkCarrierCount;
  uint32_t m_rtnLinkCarrierCount;

  // count for devices for each node ID. Currently only one device supported by helper.
  std::map<uint32_t, uint16_t> m_deviceCount;

  ObjectFactory m_deviceFactory;

  /*
   * Configured forward link interference model for dedicated access
   */
  SatPhy::InterferenceModel m_daFwdLinkInterferenceModel;

  /*
   * Configured return link interference model for dedicated access
   */
  SatPhy::InterferenceModel m_daRtnLinkInterferenceModel;

  /*
   * Configured error model for the forward feeder link. Set as an attribute.
   */
  SatPhy::ErrorModel m_fwdErrorModel;

  /*
   * Constant error rate for dedicated access in the FWD feeder link.
   */
  double m_fwdDaConstantErrorRate;

  /*
   * Configured error model for the return user link. Set as an attribute.
   */
  SatPhy::ErrorModel m_rtnErrorModel;

  /*
   * Constant error rate for dedicated access in the RTN user link.
   */
  double m_rtnDaConstantErrorRate;

  double m_symbolRateRtn;

  Ptr<SatBbFrameConf> m_bbFrameConfRtn;

  double m_symbolRateFwd;

  Ptr<SatBbFrameConf> m_bbFrameConfFwd;

  /**
   * \brief Trace callback for creation traces
   */
  TracedCallback<std::string> m_creationTrace;

  /**
   * \brief Superframe sequence
   */
  Ptr<SatSuperframeSeq> m_superframeSeq;

  /**
   * \brief The used random access model settings
   */
  RandomAccessSettings_s m_raSettings;

  /*
   * Forward channel link results (DVB-S2) are created if ErrorModel
   * is configured to be AVI.
   */
  Ptr<SatLinkResults> m_fwdLinkResults;

  /*
   * Return channel link results (DVB-RCS2) are created if ErrorModel
   * is configured to be AVI.
   */
  Ptr<SatLinkResults> m_rtnLinkResults;

  /*
   * Map used in regenerative mode to store if MAC already created for a given GW ID
   */
  std::map<uint32_t, Ptr<SatGeoFeederMac>> m_gwMacMap;

  /**
   * Control forward link messages callback
   */
  SatMac::ReadCtrlMsgCallback m_fwdReadCtrlCb;

  /**
   * Control return link messages callback
   */
  SatMac::ReadCtrlMsgCallback m_rtnReadCtrlCb;
};

} // namespace ns3

#endif /* SAT_GEO_HELPER_H */
