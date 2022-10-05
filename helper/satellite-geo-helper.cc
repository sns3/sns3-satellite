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

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "ns3/singleton.h"

#include "ns3/satellite-const-variables.h"
#include "ns3/satellite-utils.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-geo-feeder-phy.h"
#include "ns3/satellite-geo-user-phy.h"
#include "ns3/satellite-geo-feeder-mac.h"
#include "ns3/satellite-geo-user-mac.h"
#include "ns3/satellite-geo-feeder-llc.h"
#include "ns3/satellite-geo-user-llc.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/satellite-phy-rx-carrier-conf.h"
#include "ns3/satellite-channel-estimation-error-container.h"
#include "ns3/satellite-helper.h"
#include "ns3/satellite-typedefs.h"
#include "ns3/satellite-id-mapper.h"

#include "ns3/satellite-geo-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGeoHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoHelper);

TypeId
SatGeoHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoHelper")
    .SetParent<Object> ()
    .AddConstructor<SatGeoHelper> ()
    .AddAttribute ("DaFwdLinkInterferenceModel",
                   "Forward link interference model for dedicated access",
                   EnumValue (SatPhyRxCarrierConf::IF_CONSTANT),
                   MakeEnumAccessor (&SatGeoHelper::m_daFwdLinkInterferenceModel),
                   MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                    SatPhyRxCarrierConf::IF_TRACE, "Trace",
                                    SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket",
                                    SatPhyRxCarrierConf::IF_PER_FRAGMENT, "PerFragment"))
    .AddAttribute ("DaRtnLinkInterferenceModel",
                   "Return link interference model for dedicated access",
                   EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET),
                   MakeEnumAccessor (&SatGeoHelper::m_daRtnLinkInterferenceModel),
                   MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                    SatPhyRxCarrierConf::IF_TRACE, "Trace",
                                    SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket",
                                    SatPhyRxCarrierConf::IF_PER_FRAGMENT, "PerFragment"))
    .AddAttribute ("FwdLinkErrorModel",
                   "Forward feeder link error model",
                   EnumValue (SatPhyRxCarrierConf::EM_NONE),
                   MakeEnumAccessor (&SatGeoHelper::m_fwdErrorModel),
                   MakeEnumChecker (SatPhyRxCarrierConf::EM_NONE, "None",
                                    SatPhyRxCarrierConf::EM_CONSTANT, "Constant",
                                    SatPhyRxCarrierConf::EM_AVI, "AVI"))
    .AddAttribute ("FwdLinkConstantErrorRate",
                   "Constant error rate on forward feeder link",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SatGeoHelper::m_fwdDaConstantErrorRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RtnLinkErrorModel",
                   "Return user link error model",
                   EnumValue (SatPhyRxCarrierConf::EM_NONE),
                   MakeEnumAccessor (&SatGeoHelper::m_rtnErrorModel),
                   MakeEnumChecker (SatPhyRxCarrierConf::EM_NONE, "None",
                                    SatPhyRxCarrierConf::EM_CONSTANT, "Constant",
                                    SatPhyRxCarrierConf::EM_AVI, "AVI"))
    .AddAttribute ("RtnLinkConstantErrorRate",
                   "Constant error rate on return user link",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SatGeoHelper::m_rtnDaConstantErrorRate),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("Creation", "Creation traces",
                     MakeTraceSourceAccessor (&SatGeoHelper::m_creationTrace),
                     "ns3::SatTypedefs::CreationCallback")
  ;
  return tid;
}

TypeId
SatGeoHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this );

  return GetTypeId ();
}


SatGeoHelper::SatGeoHelper ()
  : m_nodeId (0),
  m_carrierBandwidthConverter (),
  m_fwdLinkCarrierCount (),
  m_rtnLinkCarrierCount (),
  m_deviceCount (0),
  m_deviceFactory (),
  m_daFwdLinkInterferenceModel (SatPhyRxCarrierConf::IF_CONSTANT),
  m_daRtnLinkInterferenceModel (SatPhyRxCarrierConf::IF_CONSTANT),
  m_raSettings (),
  m_fwdLinkResults (),
  m_rtnLinkResults ()
{
  NS_LOG_FUNCTION (this );

  // this default constructor should be never called
  NS_ASSERT (false);
}

SatGeoHelper::SatGeoHelper (SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                            uint32_t rtnLinkCarrierCount,
                            uint32_t fwdLinkCarrierCount,
                            Ptr<SatSuperframeSeq> seq,
                            RandomAccessSettings_s randomAccessSettings)
  : m_nodeId (0),
  m_carrierBandwidthConverter (bandwidthConverterCb),
  m_fwdLinkCarrierCount (fwdLinkCarrierCount),
  m_rtnLinkCarrierCount (rtnLinkCarrierCount),
  m_deviceCount (0),
  m_deviceFactory (),
  m_daFwdLinkInterferenceModel (SatPhyRxCarrierConf::IF_CONSTANT),
  m_daRtnLinkInterferenceModel (SatPhyRxCarrierConf::IF_CONSTANT),
  m_superframeSeq (seq),
  m_raSettings (randomAccessSettings),
  m_fwdLinkResults (),
  m_rtnLinkResults ()
{
  NS_LOG_FUNCTION (this << rtnLinkCarrierCount << fwdLinkCarrierCount );

  m_deviceFactory.SetTypeId ("ns3::SatGeoNetDevice");
}

void
SatGeoHelper::Initialize (Ptr<SatLinkResultsFwd> lrFwd, Ptr<SatLinkResultsRtn> lrRcs2)
{
  NS_LOG_FUNCTION (this);

  /*
   * Forward channel link results (DVB-S2 or DVB-S2X).
   */
  if (lrFwd && m_fwdErrorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_fwdLinkResults = lrFwd;
    }

  /*
   * Return channel link results (DVB-RCS2).
   */
  if (lrRcs2 && m_rtnErrorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_rtnLinkResults = lrRcs2;
    }

  m_symbolRate = m_carrierBandwidthConverter (SatEnums::RETURN_FEEDER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);

  m_bbFrameConf = CreateObject<SatBbFrameConf> (m_symbolRate, SatEnums::DVB_S2); // TODO We should be able to switch to S2X ?
  m_bbFrameConf->InitializeCNoRequirements (lrFwd);
}

void
SatGeoHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void
SatGeoHelper::SetUserPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGeoUserPhy::" + n1, v1);
}

void
SatGeoHelper::SetFeederPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGeoFeederPhy::" + n1, v1);
}

NetDeviceContainer
SatGeoHelper::Install (NodeContainer c)
{
  NS_LOG_FUNCTION (this );

  // currently only one node supported by helper
  NS_ASSERT (c.GetN () == 1);

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add (Install (*i));
    }

  return devs;
}

Ptr<NetDevice>
SatGeoHelper::Install (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this << n);

  NS_ASSERT (m_deviceCount == 0);

  // Create SatGeoNetDevice
  Ptr<SatGeoNetDevice> satDev = m_deviceFactory.Create<SatGeoNetDevice> ();

  satDev->SetAddress (Mac48Address::Allocate ());
  n->AddDevice (satDev);
  m_deviceCount++;
  m_nodeId = n->GetId ();

  Singleton<SatIdMapper>::Get ()->AttachMacToTraceId (satDev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToSatId (satDev->GetAddress ());

  return satDev;
}

Ptr<NetDevice>
SatGeoHelper::Install (std::string aName)
{
  NS_LOG_FUNCTION (this << aName );

  Ptr<Node> n = Names::Find<Node> (aName);

  return Install (n);
}

void
SatGeoHelper::AttachChannels (Ptr<NetDevice> d,
                              Ptr<SatChannel> ff,
                              Ptr<SatChannel> fr,
                              Ptr<SatChannel> uf,
                              Ptr<SatChannel> ur,
                              Ptr<SatAntennaGainPattern> userAgp,
                              Ptr<SatAntennaGainPattern> feederAgp,
                              uint32_t gwId,
                              uint32_t userBeamId,
                              SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                              SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this << d << ff << fr << uf << ur << userAgp << feederAgp << userBeamId);

  Ptr<SatGeoNetDevice> dev = DynamicCast<SatGeoNetDevice> (d);

  dev->SetForwardLinkRegenerationMode (forwardLinkRegenerationMode);
  dev->SetReturnLinkRegenerationMode (returnLinkRegenerationMode);

  AttachChannelsFeeder ( dev, ff, fr, feederAgp, gwId, userBeamId, forwardLinkRegenerationMode, returnLinkRegenerationMode);
  AttachChannelsUser ( dev, uf, ur, userAgp, userBeamId, forwardLinkRegenerationMode, returnLinkRegenerationMode);
}

void
SatGeoHelper::AttachChannelsFeeder ( Ptr<SatGeoNetDevice> dev,
                                     Ptr<SatChannel> ff,
                                     Ptr<SatChannel> fr,
                                     Ptr<SatAntennaGainPattern> feederAgp,
                                     uint32_t gwId,
                                     uint32_t userBeamId,
                                     SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                     SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{

  NS_LOG_FUNCTION (this << dev << ff << fr << feederAgp << forwardLinkRegenerationMode << returnLinkRegenerationMode);

  SatPhy::CreateParam_t params;
  params.m_beamId = userBeamId;
  params.m_device = dev;
  params.m_standard = SatEnums::GEO;

  /**
   * Simple channel estimation, which does not do actually anything
   */
  Ptr<SatChannelEstimationErrorContainer> cec = Create<SatSimpleChannelEstimationErrorContainer> ();

  SatPhyRxCarrierConf::RxCarrierCreateParams_s parametersFeeder = SatPhyRxCarrierConf::RxCarrierCreateParams_s ();
  parametersFeeder.m_errorModel = m_fwdErrorModel;
  parametersFeeder.m_daConstantErrorRate = m_fwdDaConstantErrorRate;
  parametersFeeder.m_daIfModel = m_daFwdLinkInterferenceModel;
  parametersFeeder.m_raIfModel = m_raSettings.m_raFwdInterferenceModel;
  parametersFeeder.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
  parametersFeeder.m_linkRegenerationMode = forwardLinkRegenerationMode;
  parametersFeeder.m_bwConverter = m_carrierBandwidthConverter;
  parametersFeeder.m_carrierCount = m_fwdLinkCarrierCount;
  parametersFeeder.m_cec = cec;
  parametersFeeder.m_raCollisionModel = m_raSettings.m_raCollisionModel;
  parametersFeeder.m_randomAccessModel = m_raSettings.m_randomAccessModel;

  params.m_txCh = fr;
  params.m_rxCh = ff;

  Ptr<SatGeoFeederPhy> fPhy = CreateObject<SatGeoFeederPhy> (params,
                                                             m_fwdLinkResults,
                                                             parametersFeeder,
                                                             m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE),
                                                             forwardLinkRegenerationMode,
                                                             returnLinkRegenerationMode);

  // Note, that currently we have only one set of antenna patterns,
  // which are utilized in both in user link and feeder link, and
  // in both uplink and downlink directions.
  fPhy->SetTxAntennaGainPattern (feederAgp);
  fPhy->SetRxAntennaGainPattern (feederAgp);

  dev->AddFeederPhy (fPhy, userBeamId);

  fPhy->Initialize ();

  Ptr<SatGeoFeederMac> fMac;
  Ptr<SatGeoFeederLlc> fLlc;
  bool startScheduler = false;

  Mac48Address feederAddress;

  // Create layers needed depending on max regeneration mode
  switch (std::max (forwardLinkRegenerationMode, returnLinkRegenerationMode))
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          // Create a node info to PHY layers
          Ptr<SatNodeInfo> niPhyFeeder = Create <SatNodeInfo> (SatEnums::NT_SAT, m_nodeId, Mac48Address::ConvertFrom (dev->GetAddress ()));
          fPhy->SetNodeInfo (niPhyFeeder);

          break;
        }
      case SatEnums::REGENERATION_LINK:
        {
          // Create MAC layer
          fMac = CreateObject<SatGeoFeederMac> (forwardLinkRegenerationMode,
                                                returnLinkRegenerationMode);

          // Create LLC layer
          fLlc = CreateObject<SatGeoFeederLlc> ();

          if (m_gwMacMap.count(gwId))
            {
              // MAC already exists for this GW ID, reusing it, and disabling the other
              dev->AddFeederMac (m_gwMacMap[gwId], userBeamId);
            }
          else
            {
              // First MAC for this GW ID, storing it to the map
              dev->AddFeederMac (fMac, userBeamId);
              m_gwMacMap[gwId] = fMac;
              startScheduler = true;
            }

          // Create a node info to PHY and MAC layers
          feederAddress = Mac48Address::Allocate ();
          Ptr<SatNodeInfo> niFeeder = Create <SatNodeInfo> (SatEnums::NT_SAT, m_nodeId, feederAddress);
          fPhy->SetNodeInfo (niFeeder);
          fMac->SetNodeInfo (niFeeder);
          fLlc->SetNodeInfo (niFeeder);

          break;
        }
      default:
        NS_FATAL_ERROR ("Forward or return link regeneration mode unknown");
    }

  // Connect callbacks on forward link
  switch (forwardLinkRegenerationMode)
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          SatPhy::ReceiveCallback fCb = MakeCallback (&SatGeoNetDevice::ReceiveFeeder, dev);
          fPhy->SetAttribute ("ReceiveCb", CallbackValue (fCb));

          break;
        }
      default:
        NS_FATAL_ERROR ("Forward link regeneration mode unknown");
    }

  // Connect callbacks on return link
  switch (returnLinkRegenerationMode)
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          // Nothing to do on feeder side
          break;
        }
      case SatEnums::REGENERATION_LINK:
        {
          fMac->SetTransmitFeederCallback (MakeCallback (&SatGeoFeederPhy::SendPduWithParams, fPhy));

          double carrierBandwidth = m_carrierBandwidthConverter (SatEnums::RETURN_FEEDER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);
          Ptr<SatFwdLinkSchedulerScpc> fwdLinkSchedulerScpc = CreateObject<SatFwdLinkSchedulerScpc> (m_bbFrameConf, feederAddress, carrierBandwidth);
          fMac->SetFwdScheduler (fwdLinkSchedulerScpc);
          fMac->SetLlc (fLlc);
          if (startScheduler)
            {
              fMac->StartPeriodicTransmissions ();
            }

          // Attach the LLC Tx opportunity and scheduling context getter callbacks to SatFwdLinkScheduler
          fwdLinkSchedulerScpc->SetTxOpportunityCallback (MakeCallback (&SatGeoLlc::NotifyTxOpportunity, fLlc));
          fwdLinkSchedulerScpc->SetSchedContextCallback (MakeCallback (&SatLlc::GetSchedulingContexts, fLlc));

          break;
        }
      default:
        NS_FATAL_ERROR ("Return link regeneration mode unknown");
    }
}


void
SatGeoHelper::AttachChannelsUser ( Ptr<SatGeoNetDevice> dev,
                                   Ptr<SatChannel> uf,
                                   Ptr<SatChannel> ur,
                                   Ptr<SatAntennaGainPattern> userAgp,
                                   uint32_t userBeamId,
                                   SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                   SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this << dev << uf << ur << userAgp << userBeamId << forwardLinkRegenerationMode << returnLinkRegenerationMode);

  SatPhy::CreateParam_t params;
  params.m_beamId = userBeamId;
  params.m_device = dev;
  params.m_standard = SatEnums::GEO;

  /**
   * Simple channel estimation, which does not do actually anything
   */
  Ptr<SatChannelEstimationErrorContainer> cec = Create<SatSimpleChannelEstimationErrorContainer> ();

  SatPhyRxCarrierConf::RxCarrierCreateParams_s parametersUser = SatPhyRxCarrierConf::RxCarrierCreateParams_s ();
  parametersUser.m_errorModel = m_rtnErrorModel;
  parametersUser.m_daConstantErrorRate = m_rtnDaConstantErrorRate;
  parametersUser.m_daIfModel = m_daRtnLinkInterferenceModel;
  parametersUser.m_raIfModel = m_raSettings.m_raRtnInterferenceModel;
  parametersUser.m_raIfEliminateModel = m_raSettings.m_raInterferenceEliminationModel;
  parametersUser.m_linkRegenerationMode = returnLinkRegenerationMode;
  parametersUser.m_bwConverter = m_carrierBandwidthConverter;
  parametersUser.m_carrierCount = m_rtnLinkCarrierCount;
  parametersUser.m_cec = cec;
  parametersUser.m_raCollisionModel = m_raSettings.m_raCollisionModel;
  parametersUser.m_randomAccessModel = m_raSettings.m_randomAccessModel;

  params.m_txCh = uf;
  params.m_rxCh = ur;

  Ptr<SatGeoUserPhy> uPhy = CreateObject<SatGeoUserPhy> (params,
                                                         m_rtnLinkResults,
                                                         parametersUser,
                                                         m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE),
                                                         forwardLinkRegenerationMode,
                                                         returnLinkRegenerationMode);

  // Note, that currently we have only one set of antenna patterns,
  // which are utilized in both in user link and feeder link, and
  // in both uplink and downlink directions.
  uPhy->SetTxAntennaGainPattern (userAgp);
  uPhy->SetRxAntennaGainPattern (userAgp);

  dev->AddUserPhy (uPhy, userBeamId);

  uPhy->Initialize ();

  Ptr<SatGeoUserMac> uMac;
  Ptr<SatGeoUserLlc> uLlc;

  Mac48Address userAddress;

  // Create layers needed depending on max regeneration mode
  switch (std::max (forwardLinkRegenerationMode, returnLinkRegenerationMode))
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          // Create a node info to PHY layers
          Ptr<SatNodeInfo> niPhyUser = Create <SatNodeInfo> (SatEnums::NT_SAT, m_nodeId, Mac48Address::ConvertFrom (dev->GetAddress ()));
          uPhy->SetNodeInfo (niPhyUser);

          break;
        }
      case SatEnums::REGENERATION_LINK:
        {
          // Create MAC layer
          uMac = CreateObject<SatGeoUserMac> (userBeamId,
                                              forwardLinkRegenerationMode,
                                              returnLinkRegenerationMode);

          dev->AddUserMac (uMac, userBeamId);

          // Create a node info to PHY and MAC layers
          userAddress = Mac48Address::Allocate ();
          Ptr<SatNodeInfo> niUser = Create <SatNodeInfo> (SatEnums::NT_SAT, m_nodeId, userAddress);
          uPhy->SetNodeInfo (niUser);
          uMac->SetNodeInfo (niUser);

          break;
        }
      default:
        NS_FATAL_ERROR ("Forward or return link regeneration mode unknown");
    }

  // Connect callbacks on forward link
  switch (forwardLinkRegenerationMode)
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          // Nothing to do on user side
          break;
        }
      default:
        NS_FATAL_ERROR ("Forward link regeneration mode unknown");
    }

  // Connect callbacks on return link
  switch (returnLinkRegenerationMode)
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          SatPhy::ReceiveCallback uCb = MakeCallback (&SatGeoNetDevice::ReceiveUser, dev);
          uPhy->SetAttribute ("ReceiveCb", CallbackValue (uCb));

          break;
        }
      case SatEnums::REGENERATION_LINK:
        {
          SatPhy::ReceiveCallback uCb = MakeCallback (&SatGeoUserMac::Receive, uMac);
          uPhy->SetAttribute ("ReceiveCb", CallbackValue (uCb));

          uMac->SetReceiveUserCallback (MakeCallback (&SatGeoNetDevice::ReceiveUser, dev));

          break;
        }
      default:
        NS_FATAL_ERROR ("Return link regeneration mode unknown");
    }

  if (returnLinkRegenerationMode != SatEnums::TRANSPARENT)
    {
      uPhy->BeginEndScheduling ();
    }
}

void
SatGeoHelper::EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect ("Creation", "SatGeoHelper",  cb);
}

} // namespace ns3
