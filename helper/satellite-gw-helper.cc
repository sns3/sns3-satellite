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

#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-channel.h"
#include "../model/satellite-gw-llc.h"
#include "../model/satellite-gw-mac.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-geo-net-device.h"
#include "../model/satellite-gw-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/virtual-channel.h"
#include "../model/satellite-phy-rx-carrier-conf.h"
#include "../model/satellite-link-results.h"
#include "../model/satellite-node-info.h"
#include "../model/satellite-enums.h"
#include "../model/satellite-channel-estimation-error-container.h"
#include "../model/satellite-packet-classifier.h"
#include "ns3/satellite-gw-helper.h"
#include "ns3/singleton.h"
#include "ns3/satellite-id-mapper.h"

NS_LOG_COMPONENT_DEFINE ("SatGwHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGwHelper);

TypeId
SatGwHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatGwHelper")
      .SetParent<Object> ()
      .AddConstructor<SatGwHelper> ()
      .AddAttribute ("RtnLinkErrorModel",
                     "Return link error model",
                      EnumValue (SatPhyRxCarrierConf::EM_AVI),
                      MakeEnumAccessor (&SatGwHelper::m_errorModel),
                      MakeEnumChecker (SatPhyRxCarrierConf::EM_NONE, "None",
                                      SatPhyRxCarrierConf::EM_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::EM_AVI, "AVI"))
      .AddAttribute ("RtnLinkInterferenceModel",
                     "Return link interference model",
                      EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET),
                      MakeEnumAccessor (&SatGwHelper::m_interferenceModel),
                      MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                       SatPhyRxCarrierConf::IF_TRACE, "Trace",
                                       SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute ("EnableChannelEstimationError",
                     "Enable channel estimation error in return link receiver at GW.",
                     BooleanValue (true),
                     MakeBooleanAccessor (&SatGwHelper::m_enableChannelEstimationError),
                     MakeBooleanChecker ())
      .AddTraceSource ("Creation",
                       "Creation traces",
                        MakeTraceSourceAccessor (&SatGwHelper::m_creationTrace))
    ;
    return tid;
}

TypeId
SatGwHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatGwHelper::SatGwHelper ()
 : m_rtnLinkCarrierCount (0),
   m_interferenceModel (),
   m_errorModel (),
   m_symbolRate (0.0),
   m_enableChannelEstimationError (false)
{
  // this default constructor should be never called
  NS_FATAL_ERROR ("Default constructor not supported!!!");
}

SatGwHelper::SatGwHelper (CarrierBandwidthConverter carrierBandwidthConverter,
                          uint32_t rtnLinkCarrierCount,
                          Ptr<SatSuperframeSeq> seq,
                          SatMac::ReadCtrlMsgCallback readCb,
                          SatMac::WriteCtrlMsgCallback writeCb )
 : m_carrierBandwidthConverter (carrierBandwidthConverter),
   m_rtnLinkCarrierCount (rtnLinkCarrierCount),
   m_superframeSeq (seq),
   m_readCtrlCb (readCb),
   m_writeCtrlCb (writeCb),
   m_interferenceModel (),
   m_errorModel (),
   m_symbolRate (0.0),
   m_enableChannelEstimationError (false)
{
  NS_LOG_FUNCTION (this << rtnLinkCarrierCount);

  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  //LogComponentEnable ("SatGwHelper", LOG_LEVEL_INFO);
}

void
SatGwHelper::Initialize (Ptr<SatLinkResultsDvbRcs2> lrRcs2, Ptr<SatLinkResultsDvbS2> lrS2)
{
  NS_LOG_FUNCTION (this);

  // TODO: Usage of multiple carriers needed to take into account, now only one carrier assumed to be used.
  // TODO: Symbol rate needed to check.
  m_symbolRate = m_carrierBandwidthConverter (SatEnums::FORWARD_FEEDER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);

  /*
   * Return channel link results (DVB-RCS2) are created for GWs.
   */
  if (lrRcs2 && m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_linkResults = lrRcs2;
    }

  m_bbFrameConf = CreateObject<SatBbFrameConf> (m_symbolRate);
  m_bbFrameConf->InitializeCNoRequirements (lrS2);
}

Ptr<SatBbFrameConf>
SatGwHelper::GetBbFrameConf () const
{
  NS_LOG_FUNCTION (this );

  return m_bbFrameConf;
}

void 
SatGwHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void 
SatGwHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_channelFactory.Set (n1, v1);
}

void
SatGwHelper::SetPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGwPhy::" + n1, v1);
}

NetDeviceContainer 
SatGwHelper::Install (NodeContainer c, uint32_t gwId, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << beamId << fCh << rCh );

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add (Install (*i, gwId, beamId, fCh, rCh, ncc));
  }

  return devs;
}

Ptr<NetDevice>
SatGwHelper::Install (Ptr<Node> n, uint32_t gwId, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << n << beamId << fCh << rCh );

  NetDeviceContainer container;

  // Create SatNetDevice
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  // Attach the SatNetDevices to nodes
  n->AddDevice (dev);

  SatPhy::CreateParam_t params;
  params.m_beamId = beamId;
  params.m_device = dev;
  params.m_txCh = fCh;
  params.m_rxCh = rCh;

  // Create a packet classifier
  Ptr<SatPacketClassifier> classifier = Create<SatPacketClassifier> ();

  /**
   * Channel estimation errors
   */
  Ptr<SatChannelEstimationErrorContainer> cec;
  // Not enabled, create only base class
  if (!m_enableChannelEstimationError)
    {
      cec = Create<SatSimpleChannelEstimationErrorContainer> ();
    }
  // Create SatFwdLinkChannelEstimationErrorContainer
  else
    {
      uint32_t minWfId = m_superframeSeq->GetWaveformConf ()->GetMinWfId ();
      uint32_t maxWfId = m_superframeSeq->GetWaveformConf ()->GetMaxWfId ();
      cec = Create<SatRtnLinkChannelEstimationErrorContainer> (minWfId, maxWfId);
    }

  Ptr<SatGwPhy> phy = CreateObject<SatGwPhy> (params,
                                              m_errorModel,
                                              m_linkResults,
                                              m_interferenceModel,
                                              m_carrierBandwidthConverter,
                                              m_rtnLinkCarrierCount,
                                              cec);

  // Set fading
  phy->SetTxFadingContainer (n->GetObject<SatBaseFading> ());
  phy->SetRxFadingContainer (n->GetObject<SatBaseFading> ());

  Ptr<SatGwMac> mac = CreateObject<SatGwMac> (beamId);
  mac->SetReadCtrlCallback (m_readCtrlCb);
  mac->SetWriteCtrlCallback (m_writeCtrlCb);
  mac->SetCrReceiveCallback (MakeCallback (&SatNcc::UtCrReceived, ncc));

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback recCb = MakeCallback (&SatGwMac::Receive, mac);

  // Attach the NCC C/N0 update to Phy
  SatPhy::CnoCallback cnoCb = MakeCallback (&SatNcc::UtCnoUpdated, ncc);

  // Attach the NCC random access load update to Phy
  SatPhy::AverageNormalizedOfferedLoadCallback avgNormalizedOfferedLoadCb = MakeCallback (&SatNcc::DoRandomAccessDynamicLoad, ncc);

  phy->SetAttribute ("ReceiveCb", CallbackValue(recCb));
  phy->SetAttribute ("CnoCb", CallbackValue(cnoCb));
  phy->SetAttribute ("AverageNormalizedOfferedLoad", CallbackValue(avgNormalizedOfferedLoadCb));

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac (mac);

  // Create Logical Link Control (LLC) layer
  Ptr<SatGwLlc> llc = CreateObject<SatGwLlc> ();

  // Attach the LLC layer to SatNetDevice
  dev->SetLlc (llc);

  // Attach the packet classifier
  dev->SetPacketClassifier (classifier);

  // Attach the device receive callback to SatNetDevice
  llc->SetReceiveCallback (MakeCallback (&SatNetDevice::Receive, dev));

  // Attach the transmit callback to PHY
  mac->SetTransmitCallback (MakeCallback (&SatPhy::SendPdu, phy));

  // Attach the device receive callback to SatLlc
  mac->SetReceiveCallback (MakeCallback (&SatLlc::Receive, llc));

  // Set the device address and pass it to MAC as well
  Mac48Address addr = Mac48Address::Allocate ();
  dev->SetAddress (addr);

  Singleton<SatIdMapper>::Get ()->AttachMacToTraceId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToGwId (dev->GetAddress (), gwId);
  Singleton<SatIdMapper>::Get ()->AttachMacToBeamId (dev->GetAddress (), beamId);

  phy->Initialize();

  // Create a node info to all the protocol layers
  Ptr<SatNodeInfo> nodeInfo = Create <SatNodeInfo> (SatEnums::NT_GW, n->GetId (), Mac48Address::ConvertFrom (addr));
  dev->SetNodeInfo (nodeInfo);
  llc->SetNodeInfo (nodeInfo);
  mac->SetNodeInfo (nodeInfo);
  phy->SetNodeInfo (nodeInfo);

  Ptr<SatFwdLinkScheduler> fdwLinkScheduler = CreateObject<SatFwdLinkScheduler> (m_bbFrameConf, addr);

  // Attach the LLC Tx opportunity and scheduling context getter callbacks to SatFwdLinkScheduler
  fdwLinkScheduler->SetTxOpportunityCallback (MakeCallback (&SatGwLlc::NotifyTxOpportunity, llc));
  fdwLinkScheduler->SetSchedContextCallback (MakeCallback (&SatLlc::GetSchedulingContexts, llc));

  // set scheduler to Mac
  mac->SetAttribute("Scheduler", PointerValue (fdwLinkScheduler));

  mac->StartPeriodicTransmissions ();

  return dev;
}

void
SatGwHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this );

  TraceConnect ("Creation", "SatGwHelper", cb);
}

} // namespace ns3
