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

#include "satellite-beam-helper.h"

#include "satellite-gw-helper-dvb.h"
#include "satellite-gw-helper-lora.h"
#include "satellite-ut-helper-dvb.h"
#include "satellite-ut-helper-lora.h"

#include <ns3/config.h>
#include <ns3/enum.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-interface.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/log.h>
#include <ns3/mobility-helper.h>
#include <ns3/pointer.h>
#include <ns3/satellite-antenna-gain-pattern-container.h>
#include <ns3/satellite-arp-cache.h>
#include <ns3/satellite-bstp-controller.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-fading-input-trace-container.h>
#include <ns3/satellite-fading-input-trace.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-gw-llc.h>
#include <ns3/satellite-gw-mac.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-lorawan-net-device.h>
#include <ns3/satellite-mobility-model.h>
#include <ns3/satellite-packet-trace.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-tx.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-point-to-point-isl-helper.h>
#include <ns3/satellite-propagation-delay-model.h>
#include <ns3/satellite-sgp4-mobility-model.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/satellite-ut-llc.h>
#include <ns3/satellite-ut-mac.h>
#include <ns3/satellite-utils.h>
#include <ns3/singleton.h>
#include <ns3/string.h>
#include <ns3/traffic-control-helper.h>

NS_LOG_COMPONENT_DEFINE("SatBeamHelper");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatBeamHelper);

TypeId
SatBeamHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatBeamHelper")
            .SetParent<Object>()
            .AddConstructor<SatBeamHelper>()
            .AddAttribute("CarrierFrequencyConverter",
                          "Callback to convert carrier id to generate frequency.",
                          CallbackValue(),
                          MakeCallbackAccessor(&SatBeamHelper::m_carrierFreqConverter),
                          MakeCallbackChecker())
            .AddAttribute("FadingModel",
                          "Fading model",
                          EnumValue(SatEnums::FADING_OFF),
                          MakeEnumAccessor<SatEnums::FadingModel_t>(&SatBeamHelper::m_fadingModel),
                          MakeEnumChecker(SatEnums::FADING_OFF,
                                          "FadingOff",
                                          SatEnums::FADING_TRACE,
                                          "FadingTrace",
                                          SatEnums::FADING_MARKOV,
                                          "FadingMarkov"))
            .AddAttribute("RandomAccessModel",
                          "Random Access Model",
                          EnumValue(SatEnums::RA_MODEL_OFF),
                          MakeEnumAccessor<SatEnums::RandomAccessModel_t>(&SatBeamHelper::m_randomAccessModel),
                          MakeEnumChecker(SatEnums::RA_MODEL_OFF,
                                          "RaOff",
                                          SatEnums::RA_MODEL_SLOTTED_ALOHA,
                                          "RaSlottedAloha",
                                          SatEnums::RA_MODEL_CRDSA,
                                          "RaCrdsa",
                                          SatEnums::RA_MODEL_RCS2_SPECIFICATION,
                                          "RaRcs2Specification",
                                          SatEnums::RA_MODEL_MARSALA,
                                          "RaMarsala",
                                          SatEnums::RA_MODEL_ESSA,
                                          "RaEssa"))
            .AddAttribute("RaInterferenceModel",
                          "Interference model for random access",
                          EnumValue(SatPhyRxCarrierConf::IF_CONSTANT),
                          MakeEnumAccessor<SatPhyRxCarrierConf::InterferenceModel>(&SatBeamHelper::m_raInterferenceModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::IF_CONSTANT,
                                          "Constant",
                                          SatPhyRxCarrierConf::IF_TRACE,
                                          "Trace",
                                          SatPhyRxCarrierConf::IF_PER_PACKET,
                                          "PerPacket",
                                          SatPhyRxCarrierConf::IF_PER_FRAGMENT,
                                          "PerFragment"))
            .AddAttribute("RaInterferenceEliminationModel",
                          "Interference elimination model for random access",
                          EnumValue(SatPhyRxCarrierConf::SIC_PERFECT),
                          MakeEnumAccessor<SatPhyRxCarrierConf::InterferenceEliminationModel>(&SatBeamHelper::m_raInterferenceEliminationModel),
                          MakeEnumChecker(SatPhyRxCarrierConf::SIC_PERFECT,
                                          "Perfect",
                                          SatPhyRxCarrierConf::SIC_RESIDUAL,
                                          "Residual"))
            .AddAttribute(
                "RaCollisionModel",
                "Collision model for random access",
                EnumValue(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR),
                MakeEnumAccessor<SatPhyRxCarrierConf::RandomAccessCollisionModel>(&SatBeamHelper::m_raCollisionModel),
                MakeEnumChecker(SatPhyRxCarrierConf::RA_COLLISION_NOT_DEFINED,
                                "RaCollisionNotDefined",
                                SatPhyRxCarrierConf::RA_COLLISION_ALWAYS_DROP_ALL_COLLIDING_PACKETS,
                                "RaCollisionAlwaysDropCollidingPackets",
                                SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR,
                                "RaCollisionCheckAgainstSinr",
                                SatPhyRxCarrierConf::RA_CONSTANT_COLLISION_PROBABILITY,
                                "RaCollisionConstantErrorProbability"))
            .AddAttribute("RaConstantErrorRate",
                          "Constant error rate for random access",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&SatBeamHelper::m_raConstantErrorRate),
                          MakeDoubleChecker<double>())
            .AddAttribute("PropagationDelayModel",
                          "Propagation delay model",
                          EnumValue(SatEnums::PD_CONSTANT_SPEED),
                          MakeEnumAccessor<SatEnums::PropagationDelayModel_t>(&SatBeamHelper::m_propagationDelayModel),
                          MakeEnumChecker(SatEnums::PD_CONSTANT_SPEED,
                                          "ConstantSpeed",
                                          SatEnums::PD_CONSTANT,
                                          "Constant"))
            .AddAttribute("ConstantPropagationDelay",
                          "Constant propagation delay",
                          TimeValue(Seconds(0.13)),
                          MakeTimeAccessor(&SatBeamHelper::m_constantPropagationDelay),
                          MakeTimeChecker())
            .AddAttribute(
                "PrintDetailedInformationToCreationTraces",
                "Print detailed information to creation traces",
                BooleanValue(true),
                MakeBooleanAccessor(&SatBeamHelper::m_printDetailedInformationToCreationTraces),
                MakeBooleanChecker())
            .AddAttribute("CtrlMsgStoreTimeInFwdLink",
                          "Time to store a control message in container for forward link.",
                          TimeValue(MilliSeconds(10000)),
                          MakeTimeAccessor(&SatBeamHelper::m_ctrlMsgStoreTimeFwdLink),
                          MakeTimeChecker())
            .AddAttribute("CtrlMsgStoreTimeInRtnLink",
                          "Time to store a control message in container for return link.",
                          TimeValue(MilliSeconds(10000)),
                          MakeTimeAccessor(&SatBeamHelper::m_ctrlMsgStoreTimeRtnLink),
                          MakeTimeChecker())
            .AddAttribute("EnableFwdLinkBeamHopping",
                          "Enable beam hopping in forward link.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatBeamHelper::m_enableFwdLinkBeamHopping),
                          MakeBooleanChecker())
            .AddAttribute("EnableTracesOnUserReturnLink",
                          "Use traces files on the user return channel only",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SatBeamHelper::m_enableTracesOnReturnLink),
                          MakeBooleanChecker())
            .AddAttribute("DvbVersion",
                          "Indicates if using DVB-S2 or DVB-S2X",
                          EnumValue(SatEnums::DVB_S2),
                          MakeEnumAccessor<SatEnums::DvbVersion_t>(&SatBeamHelper::m_dvbVersion),
                          MakeEnumChecker(SatEnums::DVB_S2, "DVB_S2", SatEnums::DVB_S2X, "DVB_S2X"))
            .AddAttribute("ReturnLinkLinkResults",
                          "Protocol used for the return link link results.",
                          EnumValue(SatEnums::LR_RCS2),
                          MakeEnumAccessor<SatEnums::LinkResults_t>(&SatBeamHelper::m_rlLinkResultsType),
                          MakeEnumChecker(SatEnums::LR_RCS2,
                                          "RCS2",
                                          SatEnums::LR_FSIM,
                                          "FSIM",
                                          SatEnums::LR_LORA,
                                          "LORA"))
            .AddTraceSource("Creation",
                            "Creation traces",
                            MakeTraceSourceAccessor(&SatBeamHelper::m_creationTrace),
                            "ns3::SatTypedefs::CreationCallback");
    return tid;
}

TypeId
SatBeamHelper::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatBeamHelper::SatBeamHelper()
    : m_printDetailedInformationToCreationTraces(false),
      m_fadingModel(),
      m_propagationDelayModel(SatEnums::PD_CONSTANT_SPEED),
      m_constantPropagationDelay(Seconds(0.13)),
      m_randomAccessModel(SatEnums::RA_MODEL_OFF),
      m_raInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_raInterferenceEliminationModel(SatPhyRxCarrierConf::SIC_PERFECT),
      m_raCollisionModel(SatPhyRxCarrierConf::RA_COLLISION_NOT_DEFINED),
      m_raConstantErrorRate(0.0),
      m_enableFwdLinkBeamHopping(false),
      m_bstpController(),
      m_standard(SatEnums::DVB),
      m_forwardLinkRegenerationMode(SatEnums::TRANSPARENT),
      m_returnLinkRegenerationMode(SatEnums::TRANSPARENT)
{
    NS_LOG_FUNCTION(this);

    // this default constructor should not be called...
    NS_FATAL_ERROR("SatBeamHelper::SatBeamHelper - Constructor not in use");
}

SatBeamHelper::SatBeamHelper(SatEnums::Standard_t standard,
                             NodeContainer geoNodes,
                             std::vector<std::pair<uint32_t, uint32_t>> isls,
                             SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                             uint32_t rtnLinkCarrierCount,
                             uint32_t fwdLinkCarrierCount,
                             Ptr<SatSuperframeSeq> seq,
                             SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                             SatEnums::RegenerationMode_t returnLinkRegenerationMode)
    : m_carrierBandwidthConverter(bandwidthConverterCb),
      m_superframeSeq(seq),
      m_printDetailedInformationToCreationTraces(false),
      m_fadingModel(SatEnums::FADING_MARKOV),
      m_propagationDelayModel(SatEnums::PD_CONSTANT_SPEED),
      m_constantPropagationDelay(Seconds(0.13)),
      m_randomAccessModel(SatEnums::RA_MODEL_OFF),
      m_raInterferenceModel(SatPhyRxCarrierConf::IF_CONSTANT),
      m_raInterferenceEliminationModel(SatPhyRxCarrierConf::SIC_PERFECT),
      m_raCollisionModel(SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR),
      m_raConstantErrorRate(0.0),
      m_enableFwdLinkBeamHopping(false),
      m_bstpController(),
      m_standard(standard),
      m_forwardLinkRegenerationMode(forwardLinkRegenerationMode),
      m_returnLinkRegenerationMode(returnLinkRegenerationMode),
      m_isls(isls)
{
    NS_LOG_FUNCTION(this << rtnLinkCarrierCount << fwdLinkCarrierCount << seq);

    // uncomment next code line, if attributes are needed already in construction phase.
    // E.g attributes set by object factory affecting object creation
    ObjectBase::ConstructSelf(AttributeConstructionList());

    m_channelFactory.SetTypeId("ns3::SatChannel");

    // create SatChannel containers
    m_ulChannels = Create<SatChannelPair>();
    m_flChannels = Create<SatChannelPair>();

    // create link specific control message containers
    Ptr<SatControlMsgContainer> rtnCtrlMsgContainer =
        Create<SatControlMsgContainer>(m_ctrlMsgStoreTimeRtnLink, true);
    Ptr<SatControlMsgContainer> fwdCtrlMsgContainer =
        Create<SatControlMsgContainer>(m_ctrlMsgStoreTimeFwdLink, false);

    SatMac::ReadCtrlMsgCallback rtnReadCtrlCb =
        MakeCallback(&SatControlMsgContainer::Read, rtnCtrlMsgContainer);
    SatMac::ReserveCtrlMsgCallback rtnReserveCtrlCb =
        MakeCallback(&SatControlMsgContainer::ReserveIdAndStore, rtnCtrlMsgContainer);
    SatMac::SendCtrlMsgCallback rtnSendCtrlCb =
        MakeCallback(&SatControlMsgContainer::Send, rtnCtrlMsgContainer);

    SatMac::ReadCtrlMsgCallback fwdReadCtrlCb =
        MakeCallback(&SatControlMsgContainer::Read, fwdCtrlMsgContainer);
    SatMac::ReserveCtrlMsgCallback fwdReserveCtrlCb =
        MakeCallback(&SatControlMsgContainer::ReserveIdAndStore, fwdCtrlMsgContainer);
    SatMac::SendCtrlMsgCallback fwdSendCtrlCb =
        MakeCallback(&SatControlMsgContainer::Send, fwdCtrlMsgContainer);

    SatGeoHelper::RandomAccessSettings_s geoRaSettings;
    geoRaSettings.m_raFwdInterferenceModel = m_raInterferenceModel;
    geoRaSettings.m_raRtnInterferenceModel = m_raInterferenceModel;
    geoRaSettings.m_raInterferenceEliminationModel = m_raInterferenceEliminationModel;
    geoRaSettings.m_randomAccessModel = m_randomAccessModel;
    geoRaSettings.m_raCollisionModel = m_raCollisionModel;

    SatGwHelper::RandomAccessSettings_s gwRaSettings;
    gwRaSettings.m_raInterferenceModel = m_raInterferenceModel;
    gwRaSettings.m_raInterferenceEliminationModel = m_raInterferenceEliminationModel;
    gwRaSettings.m_randomAccessModel = m_randomAccessModel;
    gwRaSettings.m_raCollisionModel = m_raCollisionModel;

    // Error rate is valid only at the GW for random access
    gwRaSettings.m_raConstantErrorRate = m_raConstantErrorRate;

    SatUtHelper::RandomAccessSettings_s utRaSettings;
    utRaSettings.m_randomAccessModel = m_randomAccessModel;
    utRaSettings.m_raInterferenceModel = m_raInterferenceModel;
    utRaSettings.m_raInterferenceEliminationModel = m_raInterferenceEliminationModel;
    utRaSettings.m_raCollisionModel = m_raCollisionModel;

    if (m_enableTracesOnReturnLink)
    {
        geoRaSettings.m_raRtnInterferenceModel = SatPhyRxCarrierConf::IF_TRACE;
        gwRaSettings.m_raInterferenceModel = SatPhyRxCarrierConf::IF_TRACE;
        Config::SetDefault("ns3::SatGeoHelper::DaRtnLinkInterferenceModel", StringValue("Trace"));
        Config::SetDefault("ns3::SatGwHelper::DaRtnLinkInterferenceModel", StringValue("Trace"));
    }

    // create needed low level satellite helpers
    m_geoHelper = CreateObject<SatGeoHelper>(bandwidthConverterCb,
                                             rtnLinkCarrierCount,
                                             fwdLinkCarrierCount,
                                             seq,
                                             fwdReadCtrlCb,
                                             rtnReadCtrlCb,
                                             geoRaSettings);

    switch (m_standard)
    {
    case SatEnums::DVB: {
        m_gwHelper = CreateObject<SatGwHelperDvb>(bandwidthConverterCb,
                                                  rtnLinkCarrierCount,
                                                  seq,
                                                  rtnReadCtrlCb,
                                                  fwdReserveCtrlCb,
                                                  fwdSendCtrlCb,
                                                  gwRaSettings);
        m_utHelper = CreateObject<SatUtHelperDvb>(bandwidthConverterCb,
                                                  fwdLinkCarrierCount,
                                                  seq,
                                                  fwdReadCtrlCb,
                                                  rtnReserveCtrlCb,
                                                  rtnSendCtrlCb,
                                                  utRaSettings);
        break;
    }
    case SatEnums::LORA: {
        m_gwHelper = CreateObject<SatGwHelperLora>(bandwidthConverterCb,
                                                   rtnLinkCarrierCount,
                                                   seq,
                                                   rtnReadCtrlCb,
                                                   fwdReserveCtrlCb,
                                                   fwdSendCtrlCb,
                                                   gwRaSettings);
        m_utHelper = CreateObject<SatUtHelperLora>(bandwidthConverterCb,
                                                   fwdLinkCarrierCount,
                                                   seq,
                                                   fwdReadCtrlCb,
                                                   rtnReserveCtrlCb,
                                                   rtnSendCtrlCb,
                                                   utRaSettings);
        break;
    }
    default:
        NS_FATAL_ERROR("Unknown standard");
    }

    // Two usage of link results is two-fold: on the other hand they are needed in the
    // packet reception for packet decoding, but on the other hand they are utilized in
    // transmission side in ACM for deciding the best MODCOD.
    //
    // Return link results:
    // - Packet reception at the GW
    // - RTN link packet scheduling at the NCC
    // DVB-S2 link results:
    // - Packet reception at the UT
    // - FWD link packet scheduling at the GW
    //
    Ptr<SatLinkResultsFwd> linkResultsFwd;
    switch (m_dvbVersion)
    {
    case SatEnums::DVB_S2:
        linkResultsFwd = CreateObject<SatLinkResultsDvbS2>();
        break;
    case SatEnums::DVB_S2X:
        linkResultsFwd = CreateObject<SatLinkResultsDvbS2X>();
        break;
    default:
        NS_FATAL_ERROR("The DVB version does not exist");
    }

    Ptr<SatLinkResultsRtn> linkResultsReturnLink;
    switch (m_rlLinkResultsType)
    {
    case SatEnums::LR_RCS2: {
        linkResultsReturnLink = CreateObject<SatLinkResultsDvbRcs2>();
        break;
    }
    case SatEnums::LR_FSIM: {
        linkResultsReturnLink = CreateObject<SatLinkResultsFSim>();
        break;
    }
    case SatEnums::LR_LORA: {
        linkResultsReturnLink = CreateObject<SatLinkResultsLora>();
        break;
    }
    default: {
        NS_FATAL_ERROR("Invalid address for multicast group");
        break;
    }
    }

    linkResultsFwd->Initialize();
    linkResultsReturnLink->Initialize();

    bool useScpc = (m_returnLinkRegenerationMode == SatEnums::REGENERATION_LINK ||
                    m_returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK);

    // DVB-S2 link results for packet decoding at the UT
    m_utHelper->Initialize(linkResultsFwd);
    // DVB-RCS2 link results for packet decoding at the GW +
    // DVB-S2 link results for FWD link RRM
    m_gwHelper->Initialize(linkResultsReturnLink, linkResultsFwd, m_dvbVersion, useScpc);
    // link results on satellite, to use if regeneration
    m_geoHelper->Initialize(linkResultsFwd, linkResultsReturnLink);
    // DVB-RCS2 link results for RTN link waveform configurations
    m_superframeSeq->GetWaveformConf()->InitializeEbNoRequirements(linkResultsReturnLink);

    m_geoNodes = geoNodes;
    m_geoHelper->Install(m_geoNodes);

    m_ncc = CreateObject<SatNcc>();

    if (m_randomAccessModel != SatEnums::RA_MODEL_OFF)
    {
        PointerValue llsConf;
        m_utHelper->GetAttribute("LowerLayerServiceConf", llsConf);
        uint8_t allocationChannelCount =
            llsConf.Get<SatLowerLayerServiceConf>()->GetRaServiceCount();

        /// set dynamic load control values required by NCC
        for (uint8_t i = 0; i < allocationChannelCount; i++)
        {
            m_ncc->SetRandomAccessLowLoadBackoffProbability(
                i,
                llsConf.Get<SatLowerLayerServiceConf>()->GetRaBackOffProbability(i));
            m_ncc->SetRandomAccessHighLoadBackoffProbability(
                i,
                llsConf.Get<SatLowerLayerServiceConf>()->GetRaHighLoadBackOffProbability(i));
            m_ncc->SetRandomAccessLowLoadBackoffTime(
                i,
                llsConf.Get<SatLowerLayerServiceConf>()->GetRaBackOffTimeInMilliSeconds(i));
            m_ncc->SetRandomAccessHighLoadBackoffTime(
                i,
                llsConf.Get<SatLowerLayerServiceConf>()->GetRaHighLoadBackOffTimeInMilliSeconds(i));
            m_ncc->SetRandomAccessAverageNormalizedOfferedLoadThreshold(
                i,
                llsConf.Get<SatLowerLayerServiceConf>()->GetRaAverageNormalizedOfferedLoadThreshold(
                    i));
        }
    }

    switch (m_fadingModel)
    {
    case SatEnums::FADING_MARKOV: {
        /// create default Markov & Loo configurations
        m_markovConf = CreateObject<SatMarkovConf>();
        break;
    }
    case SatEnums::FADING_OFF:
    case SatEnums::FADING_TRACE:
    default: {
        m_markovConf = NULL;
        break;
    }
    }

    if (m_enableFwdLinkBeamHopping)
    {
        m_bstpController = CreateObject<SatBstpController>();
    }
}

void
SatBeamHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_beam.clear();
    m_gwNode.clear();
    m_ulChannels = NULL;
    m_flChannels = NULL;
    m_beamFreqs.clear();
    m_markovConf = NULL;
    m_ncc = NULL;
    m_geoHelper = NULL;
    m_gwHelper = NULL;
    m_utHelper = NULL;
    m_antennaGainPatterns = NULL;
}

void
SatBeamHelper::Init()
{
    NS_LOG_FUNCTION(this);

    if (m_bstpController)
    {
        m_bstpController->Initialize();
    }
}

void
SatBeamHelper::SetAntennaGainPatterns(Ptr<SatAntennaGainPatternContainer> antennaPatterns)
{
    NS_LOG_FUNCTION(this << antennaPatterns);

    m_antennaGainPatterns = antennaPatterns;
}

void
SatBeamHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);
}

void
SatBeamHelper::SetChannelAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);

    m_channelFactory.Set(n1, v1);
}

void
SatBeamHelper::SetNccRoutingCallback(SatNcc::UpdateRoutingCallback cb)
{
    NS_LOG_FUNCTION(this << &cb);

    m_ncc->SetUpdateRoutingCallback(cb);
}

std::pair<Ptr<NetDevice>, NetDeviceContainer>
SatBeamHelper::Install(NodeContainer ut,
                       Ptr<Node> gwNode,
                       uint32_t gwId,
                       uint32_t satId,
                       uint32_t beamId,
                       uint32_t rtnUlFreqId,
                       uint32_t rtnFlFreqId,
                       uint32_t fwdUlFreqId,
                       uint32_t fwdFlFreqId,
                       SatMac::RoutingUpdateCallback routingCallback)
{
    NS_LOG_FUNCTION(this << gwNode << gwId << satId << beamId << rtnUlFreqId << rtnFlFreqId
                         << fwdUlFreqId << fwdFlFreqId);

    // add pair satId / beamId as key and gwId as value to beam map. In case it's there already,
    // assertion failure is caused
    std::pair<std::map<std::pair<uint32_t, uint32_t>, uint32_t>::iterator, bool> beam =
        m_beam.insert(std::make_pair(std::make_pair(satId, beamId), gwId));
    NS_ASSERT(beam.second == true);

    // TODO: Update channels to store 4 frequency ID
    // save frequency pair to map with beam ID
    // FrequencyPair_t freqPair = FrequencyPair_t (ulFreqId, flFreqId);
    // m_beamFreqs.insert (std::pair<uint32_t, FrequencyPair_t > (beamId, freqPair));

    // next it is found feeder link channels and if not found channels are created and saved to map
    SatChannelPair::ChannelPair_t feederLink =
        GetChannelPair(satId, beamId, fwdFlFreqId, rtnFlFreqId, false);

    // next it is found user link channels and if not found channels are created and saved to map
    SatChannelPair::ChannelPair_t userLink =
        GetChannelPair(satId, beamId, fwdUlFreqId, rtnUlFreqId, true);

    Ptr<Node> geoNode = m_geoNodes.Get(satId);

    NS_ASSERT(geoNode);

    // Get the position of the GW serving this beam, get the best beam based on antenna patterns
    // for this position, and set the antenna patterns to the feeder PHY objects via
    // AttachChannels method.
    GeoCoordinate gwPos = gwNode->GetObject<SatMobilityModel>()->GetGeoPosition();
    uint32_t feederSatId = GetClosestSat(gwPos);
    uint32_t feederBeamId = m_antennaGainPatterns->GetBestBeamId(feederSatId, gwPos, true);
    if (feederBeamId == 0)
    {
        feederBeamId = 1;
    }

    // attach channels to geo satellite device
    m_geoHelper->AttachChannels(geoNode->GetDevice(0),
                                feederLink.first,
                                feederLink.second,
                                userLink.first,
                                userLink.second,
                                m_antennaGainPatterns->GetAntennaGainPattern(beamId),
                                m_antennaGainPatterns->GetAntennaGainPattern(feederBeamId),
                                m_ncc,
                                satId,
                                gwId,
                                beamId,
                                m_forwardLinkRegenerationMode,
                                m_returnLinkRegenerationMode);

    Ptr<SatMobilityModel> gwMobility = gwNode->GetObject<SatMobilityModel>();
    NS_ASSERT(gwMobility);

    // enable timing advance in observers of the UTs
    for (NodeContainer::Iterator i = ut.Begin(); i != ut.End(); i++)
    {
        // enable timing advance observing in nodes.

        Ptr<SatMobilityObserver> observer = (*i)->GetObject<SatMobilityObserver>();
        NS_ASSERT(observer);
        observer->ObserveTimingAdvance(userLink.second->GetPropagationDelayModel(),
                                       feederLink.second->GetPropagationDelayModel(),
                                       gwMobility);

        if (m_fadingModel != SatEnums::FADING_OFF)
        {
            InstallFadingContainer(*i);
        }

        // save UT node pointer to multimap container
        m_utNode.insert(std::make_pair(std::make_pair(satId, beamId), *i));
    }

    Ptr<NetDevice> gwNd = InstallFeeder(DynamicCast<SatGeoNetDevice>(geoNode->GetDevice(0)),
                                        gwNode,
                                        gwId,
                                        satId,
                                        beamId,
                                        feederSatId,
                                        feederBeamId,
                                        feederLink,
                                        rtnFlFreqId,
                                        fwdFlFreqId,
                                        routingCallback);

    NetDeviceContainer utNd;
    if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_LINK ||
        m_returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        if (m_gwNdMap.count(gwId) == 0)
        {
            // If first time we create a Net Device for this GW ID, store it
            m_gwNdMap[gwId] = gwNd;
        }
        utNd = InstallUser(DynamicCast<SatGeoNetDevice>(geoNode->GetDevice(0)),
                           ut,
                           m_gwNdMap[gwId],
                           satId,
                           beamId,
                           userLink,
                           rtnUlFreqId,
                           fwdUlFreqId,
                           routingCallback);
    }
    else
    {
        utNd = InstallUser(DynamicCast<SatGeoNetDevice>(geoNode->GetDevice(0)),
                           ut,
                           gwNd,
                           satId,
                           beamId,
                           userLink,
                           rtnUlFreqId,
                           fwdUlFreqId,
                           routingCallback);
    }

    if (m_bstpController)
    {
        SatBstpController::ToggleCallback gwNdCb =
            MakeCallback(&SatNetDevice::ToggleState, DynamicCast<SatNetDevice>(gwNd));

        m_bstpController->AddNetDeviceCallback(beamId, fwdUlFreqId, fwdFlFreqId, gwId, gwNdCb);
    }

    return std::make_pair(gwNd, utNd);
}

Ptr<NetDevice>
SatBeamHelper::InstallFeeder(Ptr<SatGeoNetDevice> geoNetDevice,
                             Ptr<Node> gwNode,
                             uint32_t gwId,
                             uint32_t satId,
                             uint32_t beamId,
                             uint32_t feederSatId,
                             uint32_t feederBeamId,
                             SatChannelPair::ChannelPair_t feederLink,
                             uint32_t rtnFlFreqId,
                             uint32_t fwdFlFreqId,
                             SatMac::RoutingUpdateCallback routingCallback)
{
    NS_LOG_FUNCTION(this << gwNode << gwId << satId << beamId << rtnFlFreqId << fwdFlFreqId);

    // Set trace files if options ask for it
    if (m_enableTracesOnReturnLink)
    {
        feederLink.second->SetAttribute("RxPowerCalculationMode",
                                        EnumValue(SatEnums::RX_PWR_INPUT_TRACE));
    }

    // store GW node
    bool storedOk = StoreGwNode(gwId, gwNode);
    NS_ASSERT(storedOk);

    // install fading container to GW
    if (m_fadingModel != SatEnums::FADING_OFF)
    {
        InstallFadingContainer(gwNode);
    }

    // install GW
    PointerValue llsConf;
    m_utHelper->GetAttribute("LowerLayerServiceConf", llsConf);
    Ptr<NetDevice> gwNd =
        m_gwHelper->Install(gwNode,
                            gwId,
                            satId,
                            beamId,
                            feederSatId,
                            feederBeamId,
                            feederLink.first,
                            feederLink.second,
                            MakeCallback(&SatChannelPair::GetChannelPair, m_flChannels),
                            m_ncc,
                            llsConf.Get<SatLowerLayerServiceConf>(),
                            m_forwardLinkRegenerationMode,
                            m_returnLinkRegenerationMode);

    // calculate maximum size of the BB frame with the most robust MODCOD
    Ptr<SatBbFrameConf> bbFrameConf = m_gwHelper->GetBbFrameConf();

    SatEnums::SatBbFrameType_t frameType = SatEnums::NORMAL_FRAME;

    if (bbFrameConf->GetBbFrameUsageMode() == SatEnums::SHORT_FRAMES)
    {
        frameType = SatEnums::SHORT_FRAME;
    }

    uint32_t maxBbFrameDataSizeInBytes =
        (bbFrameConf->GetBbFramePayloadBits(bbFrameConf->GetMostRobustModcod(frameType),
                                            frameType) /
         SatConstVariables::BITS_PER_BYTE) -
        bbFrameConf->GetBbFrameHeaderSizeInBytes();

    Address satelliteUserAddress = Address();
    if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_LINK ||
        m_returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        satelliteUserAddress = geoNetDevice->GetSatelliteUserAddress(beamId);
    }

    switch (m_standard)
    {
    case SatEnums::DVB:
        m_ncc->AddBeam(
            satId,
            beamId,
            DynamicCast<SatNetDevice>(gwNd),
            geoNetDevice,
            MakeCallback(&SatNetDevice::SendControlMsg, DynamicCast<SatNetDevice>(gwNd)),
            MakeCallback(&SatGwMac::TbtpSent,
                         DynamicCast<SatGwMac>(DynamicCast<SatNetDevice>(gwNd)->GetMac())),
            m_superframeSeq,
            maxBbFrameDataSizeInBytes,
            satelliteUserAddress,
            gwNd->GetAddress());
        break;
    case SatEnums::LORA:
        m_ncc->AddBeam(satId,
                       beamId,
                       DynamicCast<SatNetDevice>(gwNd),
                       geoNetDevice,
                       MakeCallback(&SatLorawanNetDevice::SendControlMsg,
                                    DynamicCast<SatLorawanNetDevice>(gwNd)),
                       MakeNullCallback<void, Ptr<SatTbtpMessage>>(),
                       m_superframeSeq,
                       maxBbFrameDataSizeInBytes,
                       satelliteUserAddress,
                       gwNd->GetAddress());
        break;
    default:
        NS_FATAL_ERROR("Incorrect standard chosen");
    }

    // Add satellite addresses to GW MAC layers.
    if (m_forwardLinkRegenerationMode == SatEnums::REGENERATION_LINK ||
        m_forwardLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        Ptr<SatGwMac> gwMac = DynamicCast<SatGwMac>(DynamicCast<SatNetDevice>(gwNd)->GetMac());
        Mac48Address satFeederAddress = geoNetDevice->GetSatelliteFeederAddress(beamId);
        gwMac->SetGeoNodesCallback(MakeCallback(&SatBeamHelper::GetGeoSatNodes, this));
        gwMac->SetSatelliteAddress(satFeederAddress);
    }

    // Add satellite addresses to GW LLC layers.
    if (m_forwardLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        Ptr<SatGwMac> gwMac = DynamicCast<SatGwMac>(DynamicCast<SatNetDevice>(gwNd)->GetMac());
        Ptr<SatGwLlc> gwLlc = DynamicCast<SatGwLlc>(DynamicCast<SatNetDevice>(gwNd)->GetLlc());
        Mac48Address satFeederAddress = geoNetDevice->GetSatelliteFeederAddress(beamId);
        gwMac->SetGwLlcSetSatelliteAddress(MakeCallback(&SatGwLlc::SetSatelliteAddress, gwLlc));
        gwLlc->SetSatelliteAddress(satFeederAddress);
    }

    return gwNd;
}

NetDeviceContainer
SatBeamHelper::InstallUser(Ptr<SatGeoNetDevice> geoNetDevice,
                           NodeContainer ut,
                           Ptr<NetDevice> gwNd,
                           uint32_t satId,
                           uint32_t beamId,
                           SatChannelPair::ChannelPair_t userLink,
                           uint32_t rtnUlFreqId,
                           uint32_t fwdUlFreqId,
                           SatMac::RoutingUpdateCallback routingCallback)
{
    NS_LOG_FUNCTION(this << beamId << rtnUlFreqId << fwdUlFreqId);

    // Set trace files if options ask for it
    if (m_enableTracesOnReturnLink)
    {
        userLink.second->SetAttribute("RxPowerCalculationMode",
                                      EnumValue(SatEnums::RX_PWR_INPUT_TRACE));
    }

    Address satUserAddress = Address();
    if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_LINK ||
        m_returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        satUserAddress = geoNetDevice->GetUserMac(beamId)->GetAddress();
    }

    // install UTs
    NetDeviceContainer utNd =
        m_utHelper->Install(ut,
                            satId,
                            beamId,
                            userLink.first,
                            userLink.second,
                            DynamicCast<SatNetDevice>(gwNd),
                            m_ncc,
                            satUserAddress,
                            MakeCallback(&SatChannelPair::GetChannelPair, m_ulChannels),
                            routingCallback,
                            m_forwardLinkRegenerationMode,
                            m_returnLinkRegenerationMode);

    // Add satellite addresses UT MAC layers.
    if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_LINK ||
        m_returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        for (NetDeviceContainer::Iterator i = utNd.Begin(); i != utNd.End(); i++)
        {
            DynamicCast<SatUtMac>(DynamicCast<SatNetDevice>(*i)->GetMac())
                ->SetSatelliteAddress(satUserAddress);
        }
    }

    // Add satellite addresses UT LLC layers.
    if (m_returnLinkRegenerationMode == SatEnums::REGENERATION_NETWORK)
    {
        for (NetDeviceContainer::Iterator i = utNd.Begin(); i != utNd.End(); i++)
        {
            DynamicCast<SatUtLlc>(DynamicCast<SatNetDevice>(*i)->GetLlc())
                ->SetSatelliteAddress(Mac48Address::ConvertFrom(satUserAddress));
        }
    }

    return utNd;
}

void
SatBeamHelper::InstallIsls()
{
    NS_LOG_FUNCTION(this);

    Ptr<PointToPointIslHelper> p2pIslHelper = CreateObject<PointToPointIslHelper>();
    TrafficControlHelper tchIsl;

    for (std::vector<std::pair<uint32_t, uint32_t>>::iterator it = m_isls.begin();
         it != m_isls.end();
         it++)
    {
        Ptr<Node> sat1 = m_geoNodes.Get(it->first);
        Ptr<Node> sat2 = m_geoNodes.Get(it->second);

        // Install a p2p ISL link between these two satellites
        NetDeviceContainer netDevices = p2pIslHelper->Install(sat1, sat2);
        Ptr<PointToPointIslNetDevice> islNdSat1 =
            DynamicCast<PointToPointIslNetDevice>(netDevices.Get(0));
        Ptr<PointToPointIslNetDevice> islNdSat2 =
            DynamicCast<PointToPointIslNetDevice>(netDevices.Get(1));

        Ptr<SatGeoNetDevice> geoNdSat1 = DynamicCast<SatGeoNetDevice>(sat1->GetDevice(0));
        Ptr<SatGeoNetDevice> geoNdSat2 = DynamicCast<SatGeoNetDevice>(sat2->GetDevice(0));

        geoNdSat1->AddIslsNetDevice(islNdSat1);
        geoNdSat2->AddIslsNetDevice(islNdSat2);

        islNdSat1->SetGeoNetDevice(geoNdSat1);
        islNdSat2->SetGeoNetDevice(geoNdSat2);
    }
}

void
SatBeamHelper::SetIslRoutes()
{
    NS_LOG_FUNCTION(this);

    m_geoHelper->SetIslRoutes(m_geoNodes, m_isls);
}

uint32_t
SatBeamHelper::GetGwId(uint32_t satId, uint32_t beamId) const
{
    std::map<std::pair<uint32_t, uint32_t>, uint32_t>::const_iterator i =
        m_beam.find(std::make_pair(satId, beamId));

    if (i == m_beam.end())
    {
        return 0;
    }
    else
    {
        return i->second;
    }
}

Ptr<Node>
SatBeamHelper::GetGwNode(uint32_t gwId) const
{
    NS_LOG_FUNCTION(this << gwId);

    std::map<uint32_t, Ptr<Node>>::const_iterator gwIterator = m_gwNode.find(gwId);
    Ptr<Node> node = NULL;

    if (gwIterator != m_gwNode.end())
    {
        node = gwIterator->second;
    }

    return node;
}

NodeContainer
SatBeamHelper::GetGeoSatNodes() const
{
    NS_LOG_FUNCTION(this);
    return m_geoNodes;
}

Ptr<SatUtHelper>
SatBeamHelper::GetUtHelper() const
{
    NS_LOG_FUNCTION(this);
    return m_utHelper;
}

Ptr<SatGwHelper>
SatBeamHelper::GetGwHelper() const
{
    NS_LOG_FUNCTION(this);
    return m_gwHelper;
}

Ptr<SatGeoHelper>
SatBeamHelper::GetGeoHelper() const
{
    NS_LOG_FUNCTION(this);
    return m_geoHelper;
}

NodeContainer
SatBeamHelper::GetGwNodes() const
{
    NS_LOG_FUNCTION(this);

    NodeContainer gwNodes;

    for (std::map<uint32_t, Ptr<Node>>::const_iterator i = m_gwNode.begin(); i != m_gwNode.end();
         ++i)
    {
        gwNodes.Add(i->second);
    }

    return gwNodes;
}

NodeContainer
SatBeamHelper::GetUtNodes() const
{
    NS_LOG_FUNCTION(this);

    NodeContainer utNodes;

    for (std::multimap<std::pair<uint32_t, uint32_t>, Ptr<Node>>::const_iterator i =
             m_utNode.begin();
         i != m_utNode.end();
         ++i)
    {
        utNodes.Add(i->second);
    }

    return utNodes;
}

NodeContainer
SatBeamHelper::GetUtNodes(uint32_t satId, uint32_t beamId) const
{
    NS_LOG_FUNCTION(this << beamId);

    NodeContainer utNodes;

    // find all entries with the specified beamId
    std::pair<std::multimap<std::pair<uint32_t, uint32_t>, Ptr<Node>>::const_iterator,
              std::multimap<std::pair<uint32_t, uint32_t>, Ptr<Node>>::const_iterator>
        range;
    range = m_utNode.equal_range(std::make_pair(satId, beamId));

    for (std::map<std::pair<uint32_t, uint32_t>, Ptr<Node>>::const_iterator i = range.first;
         i != range.second;
         ++i)
    {
        utNodes.Add(i->second);
    }

    return utNodes;
}

std::list<std::pair<uint32_t, uint32_t>>
SatBeamHelper::GetBeams() const
{
    NS_LOG_FUNCTION(this);

    std::list<std::pair<uint32_t, uint32_t>> ret;

    for (std::map<std::pair<uint32_t, uint32_t>, uint32_t>::const_iterator it = m_beam.begin();
         it != m_beam.end();
         ++it)
    {
        ret.push_back(it->first);
    }

    return ret;
}

Ptr<SatNcc>
SatBeamHelper::GetNcc() const
{
    NS_LOG_FUNCTION(this);

    return m_ncc;
}

uint32_t
SatBeamHelper::GetUtBeamId(Ptr<Node> utNode) const
{
    NS_LOG_FUNCTION(this);

    uint32_t beamId = 0;

    for (std::multimap<std::pair<uint32_t, uint32_t>, Ptr<Node>>::const_iterator it =
             m_utNode.begin();
         ((it != m_utNode.end()) && (beamId == 0));
         it++)
    {
        if (it->second == utNode)
        {
            beamId = it->first.second;
        }
    }

    return beamId;
}

SatEnums::RegenerationMode_t
SatBeamHelper::GetReturnLinkRegenerationMode() const
{
    NS_LOG_FUNCTION(this);

    return m_returnLinkRegenerationMode;
}

NetDeviceContainer
SatBeamHelper::AddMulticastGroupRoutes(MulticastBeamInfo_t beamInfo,
                                       Ptr<Node> sourceUtNode,
                                       Ipv4Address sourceAddress,
                                       Ipv4Address groupAddress,
                                       bool routeToGwUsers,
                                       Ptr<NetDevice>& gwOutputDev)
{
    NS_LOG_FUNCTION(this);

    Mac48Address groupMacAddress;
    Ipv4StaticRoutingHelper multicast;
    NetDeviceContainer gwInputDevices;
    NetDeviceContainer routerGwOutputDevices;
    Ptr<NetDevice> routerDev = NULL;

    uint32_t sourceBeamId = GetUtBeamId(sourceUtNode);
    gwOutputDev = NULL;

    // Check the address sanity
    if (groupAddress.IsMulticast())
    {
        groupMacAddress = Mac48Address::GetMulticast(groupAddress);
        NS_LOG_INFO("IP address for multicast group: "
                    << groupAddress << ", MAC address for multicast group: " << groupMacAddress);
    }
    else
    {
        NS_FATAL_ERROR("Invalid address for multicast group");
    }

    NodeContainer gwNodes = GetGwNodes();

    // go through all GW nodes and devices in them
    for (NodeContainer::Iterator it = gwNodes.Begin(); it != gwNodes.End(); it++)
    {
        bool routerGw = false;
        NetDeviceContainer gwOutputDevices;
        Ptr<NetDevice> gwInputDev = NULL;

        // go through devices in GW node
        for (uint32_t i = 1; i < (*it)->GetNDevices(); i++)
        {
            Ptr<NetDevice> dev = (*it)->GetDevice(i);
            int32_t beamId = Singleton<SatIdMapper>::Get()->GetBeamIdWithMac(dev->GetAddress());
            MulticastBeamInfo_t::iterator beamIt = beamInfo.find(beamId);

            // device is device serving source UT. (UT source is in question)
            if (beamId == (int32_t)sourceBeamId)
            {
                routerGw = true; // GW node is routing GW
                routerDev = dev; // save device routing multicast traffic from UT
            }

            // device needs to serve multicast receiving UT(s) in the beam (beam is among of the
            // multicast group)
            if (beamIt != beamInfo.end())
            {
                // device must be also SatNetDevice, just sanity check it
                if (dev->GetInstanceTypeId().GetName() == "ns3::SatNetDevice")
                {
                    // add device to GW's output list (container) to route multicast traffic to
                    // beam's UTs
                    gwOutputDevices.Add(dev);

                    // go through UTs receiving multicast traffic in the beam
                    // to route multicast traffic toward public network (UT users)
                    for (std::set<Ptr<Node>>::iterator utIt = beamIt->second.begin();
                         utIt != beamIt->second.end();
                         utIt++)
                    {
                        AddMulticastRouteToUt(*utIt, sourceAddress, groupAddress, false);
                    }
                }
                else
                {
                    NS_FATAL_ERROR("Not a satellite net device!!!");
                }
            }
            else if (dev->GetInstanceTypeId().GetName() != "ns3::SatNetDevice")
            {
                // save device receiving traffic from IP router
                gwInputDev = dev;
            }
        }

        if (routerGw)
        {
            // in case that GW is source beam serving (routing) GW
            // traffic is going to IP router, so input is output
            gwOutputDev = gwInputDev;

            // GW output devices to satellite network UTs are saved here to set later.
            // In router case it is needed to check later that if traffic is needed to route
            // IP router too. (GW user or some other beam behind another GW is receiving traffic)
            routerGwOutputDevices = gwOutputDevices;
        }
        else if (gwOutputDevices.GetN() >
                 0) // no router GW some device in GW belong to beam receiving multicast traffic
        {
            // route traffic from source beam to receiving beams
            multicast.AddMulticastRoute(*it,
                                        sourceAddress,
                                        groupAddress,
                                        gwInputDev,
                                        gwOutputDevices);

            // save devices receiving traffic from IP router (backbone network)
            gwInputDevices.Add(gwInputDev);
        }
    }

    // source is UT and traffic is needed to route toward backbone network
    // add output device to routing GW's output container (list)
    if (sourceUtNode && ((gwInputDevices.GetN() > 0) || routeToGwUsers))
    {
        if (!routerDev)
        {
            NS_FATAL_ERROR("Router device shall exist!!!");
        }

        routerGwOutputDevices.Add(gwOutputDev);
    }
    else
    {
        gwOutputDev = NULL;
    }

    // route traffic from source beam satellite net device to satellite net devices forwarding
    // traffic to receiving beams inside router GW and/or to backbone network (GW users). add also
    // route to UT from public network to satellite network
    if (routerDev && (routerGwOutputDevices.GetN() > 0))
    {
        multicast.AddMulticastRoute(routerDev->GetNode(),
                                    sourceAddress,
                                    groupAddress,
                                    routerDev,
                                    routerGwOutputDevices);
        AddMulticastRouteToUt(sourceUtNode, sourceAddress, groupAddress, true);
    }

    // return list of GW net devices receiving traffic from IP router (backbone network)
    return gwInputDevices;
}

void
SatBeamHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase& cb)
{
    NS_LOG_FUNCTION(this);

    TraceConnect("Creation", "SatBeamHelper", cb);
    m_geoHelper->EnableCreationTraces(stream, cb);
    m_gwHelper->EnableCreationTraces(stream, cb);
    m_utHelper->EnableCreationTraces(stream, cb);
}

uint32_t
SatBeamHelper::GetClosestSat(GeoCoordinate position)
{
    NS_LOG_FUNCTION(this);

    double distanceMin = std::numeric_limits<double>::max();
    uint32_t indexDistanceMin = 0;

    for (uint32_t i = 0; i < m_geoNodes.GetN(); i++)
    {
        GeoCoordinate satPos = m_geoNodes.Get(i)->GetObject<SatMobilityModel>()->GetGeoPosition();
        double distance = CalculateDistance(position.ToVector(), satPos.ToVector());
        if (distance < distanceMin)
        {
            distanceMin = distance;
            indexDistanceMin = i;
        }
    }
    return indexDistanceMin;
}

void
SatBeamHelper::EnablePacketTrace()
{
    // Create packet trace instance
    m_packetTrace = CreateObject<SatPacketTrace>();

    /**
     * Connect the trace callbacks
     * By default the packet traces are connected to
     * - NetDevice
     * - LLC
     * - MAC
     * - PHY
     */

    SatEnums::RegenerationMode_t maxRegeneration =
        std::max(m_forwardLinkRegenerationMode, m_returnLinkRegenerationMode);

    /**
     * TODO: Currently the packet trace logs all entries updated by the protocol layers. Here
     * we could restrict the protocol layers from where the traced data are collected from.
     * This could be controlled by the user using attributes.
     */
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/PacketTrace",
                                  MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/SatPhy/PacketTrace",
                                  MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/UserPhy/*/PacketTrace",
                                  MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/FeederPhy/*/PacketTrace",
                                  MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/SatMac/PacketTrace",
                                  MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
    if (maxRegeneration == SatEnums::REGENERATION_LINK ||
        maxRegeneration == SatEnums::REGENERATION_NETWORK)
    {
        Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/UserMac/*/PacketTrace",
                                      MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
        Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/FeederMac/*/PacketTrace",
                                      MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
    }
    if (m_standard == SatEnums::DVB)
    {
        Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/SatLlc/PacketTrace",
                                      MakeCallback(&SatPacketTrace::AddTraceEntry, m_packetTrace));
    }
}

std::string
SatBeamHelper::GetBeamInfo() const
{
    NS_LOG_FUNCTION(this);

    std::ostringstream oss;
    oss << "--- Beam Info, "
        << "number of created beams: " << m_beam.size() << " ---" << std::endl;

    if (m_beam.size() > 0)
    {
        oss << CreateBeamInfo();
    }

    return oss.str();
}

std::string
SatBeamHelper::GetUtInfo() const
{
    NS_LOG_FUNCTION(this);

    std::ostringstream oss;

    for (std::multimap<std::pair<uint32_t, uint32_t>, Ptr<Node>>::const_iterator i =
             m_utNode.begin();
         i != m_utNode.end();
         ++i)
    {
        Ptr<SatMobilityModel> model = i->second->GetObject<SatMobilityModel>();
        GeoCoordinate pos = model->GetGeoPosition();

        Address devAddress;
        Ptr<Ipv4> ipv4 = i->second->GetObject<Ipv4>(); // Get Ipv4 instance of the node

        std::vector<Ipv4Address> IPAddressVector;
        std::vector<std::string> devNameVector;
        std::vector<Address> devAddressVector;

        for (uint32_t j = 0; j < i->second->GetNDevices(); j++)
        {
            Ptr<NetDevice> device = i->second->GetDevice(j);

            if (device->GetInstanceTypeId().GetName() == "ns3::SatNetDevice")
            {
                devAddress = device->GetAddress();
            }
            IPAddressVector.push_back(
                ipv4->GetAddress(j, 0).GetLocal()); // Get Ipv4InterfaceAddress of interface
            devNameVector.push_back(device->GetInstanceTypeId().GetName());
            devAddressVector.push_back(device->GetAddress());
        }

        if (m_printDetailedInformationToCreationTraces)
        {
            oss << i->first.second << " "
                << Singleton<SatIdMapper>::Get()->GetUtIdWithMac(devAddress) << " "
                << pos.GetLatitude() << " " << pos.GetLongitude() << " " << pos.GetAltitude()
                << " ";

            for (uint32_t j = 0; j < i->second->GetNDevices(); j++)
            {
                oss << devNameVector[j] << " " << devAddressVector[j] << " " << IPAddressVector[j]
                    << " ";
            }

            oss << std::endl;
        }
        else
        {
            oss << i->first.second << " "
                << Singleton<SatIdMapper>::Get()->GetUtIdWithMac(devAddress) << " "
                << pos.GetLatitude() << " " << pos.GetLongitude() << " " << pos.GetAltitude()
                << std::endl;
        }
    }

    return oss.str();
}

std::string
SatBeamHelper::CreateBeamInfo() const
{
    NS_LOG_FUNCTION(this);

    std::ostringstream oss;

    oss << std::endl << " -- Beam details --";

    for (std::map<std::pair<uint32_t, uint32_t>, uint32_t>::const_iterator i = m_beam.begin();
         i != m_beam.end();
         ++i)
    {
        uint32_t satId = i->first.first;
        uint32_t beamId = i->first.second;

        oss << std::endl << "Sat ID: " << satId << " ";
        oss << "Beam ID: " << beamId << " ";

        std::map<std::pair<uint32_t, uint32_t>, FrequencyPair_t>::const_iterator freqIds =
            m_beamFreqs.find(std::make_pair(satId, beamId));

        if (freqIds != m_beamFreqs.end())
        {
            oss << "user link frequency ID: " << (*freqIds).second.first << ", ";
            oss << "feeder link frequency ID: " << (*freqIds).second.second;
        }

        oss << ", GW ID: " << (*i).second;
    }

    oss << std::endl << std::endl << " -- GW details --" << std::endl;

    oss.precision(8);
    oss.setf(std::ios::fixed, std::ios::floatfield);

    for (std::map<uint32_t, Ptr<Node>>::const_iterator i = m_gwNode.begin(); i != m_gwNode.end();
         ++i)
    {
        Ptr<SatMobilityModel> model = i->second->GetObject<SatMobilityModel>();
        GeoCoordinate pos = model->GetGeoPosition();

        Address devAddress;
        Ptr<Ipv4> ipv4 = i->second->GetObject<Ipv4>(); // Get Ipv4 instance of the node

        std::vector<Ipv4Address> IPAddressVector;
        std::vector<std::string> devNameVector;
        std::vector<Address> devAddressVector;

        for (uint32_t j = 0; j < i->second->GetNDevices(); j++)
        {
            Ptr<NetDevice> device = i->second->GetDevice(j);

            if (device->GetInstanceTypeId().GetName() == "ns3::SatNetDevice")
            {
                devAddress = device->GetAddress();
            }

            IPAddressVector.push_back(
                ipv4->GetAddress(j, 0).GetLocal()); // Get Ipv4InterfaceAddress of interface
            devNameVector.push_back(device->GetInstanceTypeId().GetName());
            devAddressVector.push_back(device->GetAddress());
        }

        if (m_printDetailedInformationToCreationTraces)
        {
            oss << "GW=" << i->first << " "
                << Singleton<SatIdMapper>::Get()->GetUtIdWithMac(devAddress) << " "
                << " latitude=" << pos.GetLatitude() << " longitude=" << pos.GetLongitude()
                << " altitude=" << pos.GetAltitude() << " ";

            for (uint32_t j = 0; j < i->second->GetNDevices(); j++)
            {
                oss << devNameVector[j] << " " << devAddressVector[j] << " " << IPAddressVector[j]
                    << " ";
            }

            oss << std::endl;
        }
        else
        {
            oss << "GW=" << i->first << " "
                << Singleton<SatIdMapper>::Get()->GetUtIdWithMac(devAddress) << " "
                << " latitude=" << pos.GetLatitude() << " longitude=" << pos.GetLongitude()
                << " altitude=" << pos.GetAltitude() << std::endl;
        }
    }

    oss << std::endl << " -- Geo Satellite position --" << std::endl;

    Ptr<SatMobilityModel> model = m_geoNodes.Get(0)->GetObject<SatMobilityModel>();
    GeoCoordinate pos = model->GetGeoPosition();
    oss << "latitude=" << pos.GetLatitude() << ", longitude=" << pos.GetLongitude()
        << ", altitude=" << pos.GetAltitude() << std::endl;

    return oss.str();
}

SatChannelPair::ChannelPair_t
SatBeamHelper::GetChannelPair(uint32_t satId,
                              uint32_t beamId,
                              uint32_t fwdFrequencyId,
                              uint32_t rtnFrequencyId,
                              bool isUserLink)
{
    NS_LOG_FUNCTION(this << satId << beamId << fwdFrequencyId << rtnFrequencyId << isUserLink);

    Ptr<SatChannelPair> chPairs = isUserLink ? m_ulChannels : m_flChannels;

    bool hasFwdChannel = chPairs->HasFwdChannel(satId, fwdFrequencyId);
    bool hasRtnChannel = chPairs->HasRtnChannel(satId, rtnFrequencyId);

    if (hasFwdChannel && hasRtnChannel)
    {
        chPairs->UpdateBeamsForFrequency(satId, beamId, fwdFrequencyId, rtnFrequencyId);
    }
    else
    {
        Ptr<SatFreeSpaceLoss> pFsl;
        Ptr<PropagationDelayModel> pDelay;
        Ptr<SatChannel> forwardCh;
        Ptr<SatChannel> returnCh;

        if (hasFwdChannel)
        {
            forwardCh = chPairs->GetForwardChannel(satId, fwdFrequencyId);
            pDelay = forwardCh->GetPropagationDelayModel();
            pFsl = forwardCh->GetFreeSpaceLoss();
        }
        else if (hasRtnChannel)
        {
            returnCh = chPairs->GetReturnChannel(satId, rtnFrequencyId);
            pDelay = returnCh->GetPropagationDelayModel();
            pFsl = returnCh->GetFreeSpaceLoss();
        }
        else
        {
            if (m_propagationDelayModel == SatEnums::PD_CONSTANT_SPEED)
            {
                // Signal propagates at the speed of light
                pDelay = CreateObject<ConstantSpeedPropagationDelayModel>();
                DynamicCast<ConstantSpeedPropagationDelayModel>(pDelay)->SetSpeed(
                    SatConstVariables::SPEED_OF_LIGHT);
            }
            else if (m_propagationDelayModel == SatEnums::PD_CONSTANT)
            {
                pDelay = CreateObject<SatConstantPropagationDelayModel>();
                DynamicCast<SatConstantPropagationDelayModel>(pDelay)->SetDelay(
                    m_constantPropagationDelay);
            }
            else
            {
                NS_FATAL_ERROR("Unsupported propagation delay model!");
            }

            pFsl = CreateObject<SatFreeSpaceLoss>();
        }

        if (!hasFwdChannel)
        {
            forwardCh = m_channelFactory.Create<SatChannel>();
            forwardCh->SetChannelType(isUserLink ? SatEnums::FORWARD_USER_CH
                                                 : SatEnums::FORWARD_FEEDER_CH);
            forwardCh->SetFrequencyConverter(m_carrierFreqConverter);
            forwardCh->SetBandwidthConverter(m_carrierBandwidthConverter);
            forwardCh->SetFrequencyId(fwdFrequencyId);
            forwardCh->SetPropagationDelayModel(pDelay);
            forwardCh->SetFreeSpaceLoss(pFsl);
        }

        if (!hasRtnChannel)
        {
            returnCh = m_channelFactory.Create<SatChannel>();
            returnCh->SetChannelType(isUserLink ? SatEnums::RETURN_USER_CH
                                                : SatEnums::RETURN_FEEDER_CH);
            returnCh->SetFrequencyConverter(m_carrierFreqConverter);
            returnCh->SetBandwidthConverter(m_carrierBandwidthConverter);
            returnCh->SetFrequencyId(rtnFrequencyId);
            returnCh->SetPropagationDelayModel(pDelay);
            returnCh->SetFreeSpaceLoss(pFsl);
        }

        chPairs
            ->StoreChannelPair(satId, beamId, fwdFrequencyId, forwardCh, rtnFrequencyId, returnCh);
    }

    return chPairs->GetChannelPair(satId, beamId);
}

bool
SatBeamHelper::StoreGwNode(uint32_t id, Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << id << node);

    bool storingSuccess = false;

    Ptr<Node> storedNode = GetGwNode(id);

    if (storedNode) // nGW node with id already stored
    {
        if (storedNode == node) // check that node is same
        {
            storingSuccess = true;
        }
    }
    else // try to store if not stored
    {
        std::pair<std::map<uint32_t, Ptr<Node>>::iterator, bool> result =
            m_gwNode.insert(std::make_pair(id, node));
        storingSuccess = result.second;
    }

    return storingSuccess;
}

Ptr<SatBaseFading>
SatBeamHelper::InstallFadingContainer(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);

    Ptr<SatBaseFading> fadingContainer = node->GetObject<SatBaseFading>();

    if (fadingContainer == nullptr)
    {
        switch (m_fadingModel)
        {
        case SatEnums::FADING_MARKOV: {
            Ptr<SatMobilityObserver> observer = node->GetObject<SatMobilityObserver>();
            NS_ASSERT(observer);

            SatBaseFading::ElevationCallback elevationCb =
                MakeCallback(&SatMobilityObserver::GetElevationAngle, observer);
            SatBaseFading::VelocityCallback velocityCb =
                MakeCallback(&SatMobilityObserver::GetVelocity, observer);

            /// create a Markov fading container based on default configuration
            fadingContainer =
                CreateObject<SatMarkovContainer>(m_markovConf, elevationCb, velocityCb);
            node->AggregateObject(fadingContainer);
            break;
        }
        case SatEnums::FADING_TRACE: {
            /// create a input trace fading container based on default configuration

            fadingContainer =
                CreateObject<SatFadingInputTrace>(Singleton<SatFadingInputTraceContainer>::Get());

            node->AggregateObject(fadingContainer);
            break;
        }
        /// FADING_OFF option should never get to InstallFadingContainer
        case SatEnums::FADING_OFF:
        default: {
            NS_FATAL_ERROR("SatBeamHelper::InstallFadingContainer - Incorrect fading model");
            break;
        }
        }
    }
    return fadingContainer;
}

void
SatBeamHelper::AddMulticastRouteToUt(Ptr<Node> utNode,
                                     Ipv4Address sourceAddress,
                                     Ipv4Address groupAddress,
                                     bool routeToSatellite)
{
    NS_LOG_FUNCTION(this);

    Ptr<NetDevice> satDev = NULL;
    Ptr<NetDevice> publicDev = NULL;

    for (uint32_t i = 1; i < utNode->GetNDevices(); i++)
    {
        Ptr<NetDevice> dev = utNode->GetDevice(i);

        // in UT SatNetDevice is routing traffic to public network NetDevice e.g. CSMA
        if (dev->GetInstanceTypeId().GetName() == "ns3::SatNetDevice")
        {
            satDev = dev;
        }
        else
        {
            // just save last non SatNetDevice (in practice there should be only one)
            publicDev = dev;
        }
    }

    if (satDev && publicDev)
    {
        Ipv4StaticRoutingHelper multicast;

        if (routeToSatellite)
        {
            multicast.AddMulticastRoute(utNode, sourceAddress, groupAddress, publicDev, satDev);
        }
        else
        {
            multicast.AddMulticastRoute(utNode, sourceAddress, groupAddress, satDev, publicDev);
        }

        // Add multicast route to UT
    }
    else
    {
        NS_FATAL_ERROR("Input of output device not found in UT node!!!");
    }
}

Ptr<PropagationDelayModel>
SatBeamHelper::GetPropagationDelayModel(uint32_t satId,
                                        uint32_t beamId,
                                        SatEnums::ChannelType_t channelType)
{
    Ptr<SatChannel> channel = NULL;
    switch (channelType)
    {
    case SatEnums::FORWARD_FEEDER_CH: {
        channel = m_flChannels->GetChannelPair(satId, beamId).first;
        break;
    }
    case SatEnums::FORWARD_USER_CH: {
        channel = m_ulChannels->GetChannelPair(satId, beamId).first;
        break;
    }
    case SatEnums::RETURN_USER_CH: {
        channel = m_ulChannels->GetChannelPair(satId, beamId).second;
        break;
    }
    case SatEnums::RETURN_FEEDER_CH: {
        channel = m_flChannels->GetChannelPair(satId, beamId).second;
        break;
    }
    default: {
        return NULL;
    }
    }

    return channel->GetPropagationDelayModel();
}

SatEnums::PropagationDelayModel_t
SatBeamHelper::GetPropagationDelayModelEnum()
{
    return m_propagationDelayModel;
}

} // namespace ns3
