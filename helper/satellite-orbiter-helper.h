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

#ifndef SAT_ORBITER_HELPER_H
#define SAT_ORBITER_HELPER_H

#include "ns3/error-model.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/satellite-bbframe-conf.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-fwd-link-scheduler.h"
#include "ns3/satellite-ncc.h"
#include "ns3/satellite-orbiter-feeder-mac.h"
#include "ns3/satellite-orbiter-net-device.h"
#include "ns3/satellite-phy.h"
#include "ns3/satellite-scpc-scheduler.h"
#include "ns3/satellite-superframe-sequence.h"
#include "ns3/satellite-typedefs.h"
#include "ns3/traced-callback.h"

#include <map>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

namespace ns3
{

/**
 * @brief Creates needed objects for Satellite node like SatorbiterNetDevice objects.
 *        Handles needed configuration for the Satellite node.
 *
 */
class SatOrbiterHelper : public Object
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
     * @brief Get the type ID
     * @return the object TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * Default constructor.
     */
    SatOrbiterHelper();

    /**
     * Create a SatOrbiterHelper to make life easier when creating Satellite point to
     * point network connections.
     */
    SatOrbiterHelper(SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                     uint32_t rtnLinkCarrierCount,
                     uint32_t fwdLinkCarrierCount,
                     Ptr<SatSuperframeSeq> seq,
                     SatMac::ReadCtrlMsgCallback fwdReadCb,
                     SatMac::ReadCtrlMsgCallback rtnReadCb,
                     RandomAccessSettings_s randomAccessSettings);

    virtual ~SatOrbiterHelper()
    {
    }

    /*
     * Initializes the orbiter helper based on attributes.
     * Link results are used only if satellite is regenerative.
     * @param lrFwd DVB-S2 or DVB-S2X link results
     * @param lrRcs2 return link results
     */
    void Initialize(Ptr<SatLinkResultsFwd> lrFwd, Ptr<SatLinkResultsRtn> lrRcs2);

    /**
     * Set an attribute value to be propagated to each NetDevice created by the
     * helper.
     *
     * @param name the name of the attribute to set
     * @param value the value of the attribute to set
     *
     * Set these attributes on each ns3::SatOrbiterNetDevice created
     * by SatOrbiterHelper::Install
     */
    void SetDeviceAttribute(std::string name, const AttributeValue& value);

    /**
     * Set an attribute value to be propagated to each User Phy created by the helper.
     *
     * @param name the name of the attribute to set
     * @param value the value of the attribute to set
     *
     * Set these attributes on each ns3::SatPhy (user) created
     * by SatOrbiterHelper::Install
     */
    void SetUserPhyAttribute(std::string name, const AttributeValue& value);

    /**
     * Set an attribute value to be propagated to each Feeder Phy created by the helper.
     *
     * @param name the name of the attribute to set
     * @param value the value of the attribute to set
     *
     * Set these attributes on each ns3::SatPhy (feeder) created
     * by SatOrbiterHelper::Install
     */
    void SetFeederPhyAttribute(std::string name, const AttributeValue& value);

    /**
     * This method creates a ns3::SatOrbiterNetDevices with the requested attributes
     * and associate the resulting ns3::NetDevices with the ns3::Nodes corresponding to the
     * orbiters.
     *
     * @return container to the created devices
     */
    NetDeviceContainer InstallAllOrbiters();

    /**
     * @param n a node
     *
     * This method creates a ns3::SatOrbiterNetDevice with the requested attributes
     * and associate the resulting ns3::NetDevice with the ns3::Node.
     *
     * @return pointer to the created device
     */
    Ptr<NetDevice> Install(Ptr<Node> n);

    /**
     * @param aName name of a node
     *
     * This method creates a ns3::SatOrbiterNetDevice with the requested attributes
     * and associate the resulting ns3::NetDevice with the ns3::Node.
     *
     * @return pointer to the created device
     */
    Ptr<NetDevice> Install(std::string aName);

    /**
     * Create a SatOrbiterNetDevice instance, with correct type infered from child classes.
     *
     * @return SatOrbiterNetDevice instance
     */
    virtual Ptr<SatOrbiterNetDevice> CreateOrbiterNetDevice() = 0;

    /*
     * Attach the SatChannels for the beam to NetDevice
     * @param dev NetDevice to attach channels
     * @param fr feeder return channel
     * @param uf user forward channel
     * @param uf user return channel
     * @param userAgp user beam antenna gain pattern
     * @param feederAgp feeder beam antenna gain pattern
     * @param ncc NCC (Network Control Center)
     * @param satId ID of satellite associated to this channel
     * @param gwId ID of GW associated to this channel
     * @param userBeamId Id of the user beam
     */
    void AttachChannels(Ptr<NetDevice> dev,
                        Ptr<SatChannel> ff,
                        Ptr<SatChannel> fr,
                        Ptr<SatChannel> uf,
                        Ptr<SatChannel> ur,
                        Ptr<SatAntennaGainPattern> userAgp,
                        Ptr<SatAntennaGainPattern> feederAgp,
                        Ptr<SatNcc> ncc,
                        uint32_t satId,
                        uint32_t gwId,
                        uint32_t userBeamId);

    /*
     * Attach the SatChannels for the beam to NetDevice
     * @param dev NetDevice to attach channels
     * @param fr feeder forward channel
     * @param fr feeder return channel
     * @param feederAgp feeder beam antenna gain pattern
     * @param ncc NCC (Network Control Center)
     * @param satId ID of satellite associated to this channel
     * @param gwId ID of GW associated to this channel
     * @param userBeamId Id of the user beam
     */
    void AttachChannelsFeeder(Ptr<SatOrbiterNetDevice> dev,
                              Ptr<SatChannel> ff,
                              Ptr<SatChannel> fr,
                              Ptr<SatAntennaGainPattern> feederAgp,
                              Ptr<SatNcc> ncc,
                              uint32_t satId,
                              uint32_t gwId,
                              uint32_t userBeamId);

    /*
     * Attach the SatChannels for the beam to NetDevice
     * @param dev NetDevice to attach channels
     * @param uf user forward channel
     * @param uf user return channel
     * @param userAgp user beam antenna gain pattern
     * @param ncc NCC (Network Control Center)
     * @param satId ID of satellite associated to this channel
     * @param userBeamId Id of the beam
     */
    virtual void AttachChannelsUser(Ptr<SatOrbiterNetDevice> dev,
                                    Ptr<SatChannel> uf,
                                    Ptr<SatChannel> ur,
                                    Ptr<SatAntennaGainPattern> userAgp,
                                    Ptr<SatNcc> ncc,
                                    uint32_t satId,
                                    uint32_t userBeamId) = 0;

    /**
     * Enables creation traces to be written in given file
     * @param stream  stream for creation trace outputs
     * @param cb  callback to connect traces
     */
    void EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase& cb);

    /**
     * Set ISL routes
     *
     * @param isls List of all ISLs
     */
    void SetIslRoutes(std::vector<std::pair<uint32_t, uint32_t>> isls);

  protected:
    /**
     * Satellites node id
     */
    std::vector<uint32_t> m_nodeIds;

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
     * @brief Trace callback for creation traces
     */
    TracedCallback<std::string> m_creationTrace;

    /**
     * @brief Superframe sequence
     */
    Ptr<SatSuperframeSeq> m_superframeSeq;

    /**
     * @brief The used random access model settings
     */
    RandomAccessSettings_s m_raSettings;

    /**
     * Forward channel link results (DVB-S2) are created if ErrorModel
     * is configured to be AVI.
     */
    Ptr<SatLinkResults> m_fwdLinkResults;

    /**
     * Return channel link results (DVB-RCS2) are created if ErrorModel
     * is configured to be AVI.
     */
    Ptr<SatLinkResults> m_rtnLinkResults;

    /**
     * Map used in regenerative mode to store if MAC already created for a given pair SAT ID / GW ID
     */
    std::map<std::pair<uint32_t, uint32_t>, Ptr<SatOrbiterFeederMac>> m_gwMacMap;

    /**
     * Arbiter in use to route packets on ISLs
     */
    SatEnums::IslArbiterType_t m_islArbiterType;

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

#endif /* SAT_ORBITER_HELPER_H */
