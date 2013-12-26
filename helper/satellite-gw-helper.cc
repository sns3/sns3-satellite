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
#include "../model/satellite-llc.h"
#include "../model/satellite-gw-mac.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-geo-net-device.h"
#include "../model/satellite-gw-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/virtual-channel.h"
#include "../model/satellite-phy-rx-carrier-conf.h"
#include "../model/satellite-link-results.h"
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
      .AddTraceSource ("Creation", "Creation traces",
                        MakeTraceSourceAccessor (&SatGwHelper::m_creation))
    ;
    return tid;
}

TypeId
SatGwHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatGwHelper::SatGwHelper ()
{
  // this default constructor should be never called
  NS_ASSERT (false);
}

SatGwHelper::SatGwHelper (CarrierBandwidthConverter carrierBandwidthConverter, uint32_t rtnLinkCarrierCount)
 : m_carrierBandwidthConverter (carrierBandwidthConverter),
   m_rtnLinkCarrierCount (rtnLinkCarrierCount)
{
  NS_LOG_FUNCTION (this << rtnLinkCarrierCount);

  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  m_bbFrameConf = CreateObject<SatBbFrameConf> ();

  //LogComponentEnable ("SatGwHelper", LOG_LEVEL_INFO);
}

void
SatGwHelper::Initialize ()
{
  NS_LOG_FUNCTION (this );

  /*
   * Return channel link results (DVB-RCS2) are created for GWs.
   */
  if (m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_linkResults = CreateObject<SatLinkResultsDvbRcs2> ();
      m_linkResults->Initialize ();
    }
}

void 
SatGwHelper::SetQueue (std::string type,
                              std::string n1, const AttributeValue &v1,
                              std::string n2, const AttributeValue &v2,
                              std::string n3, const AttributeValue &v3,
                              std::string n4, const AttributeValue &v4)
{
  NS_LOG_FUNCTION (this << type << n1 << n2 << n3 <<  n4  );

  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
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

  Ptr<SatGwPhy> phy = CreateObject<SatGwPhy> (params, m_errorModel, m_linkResults, m_interferenceModel, m_carrierBandwidthConverter, m_rtnLinkCarrierCount);

  // Set fading
  phy->SetTxFadingContainer (n->GetObject<SatBaseFading> ());
  phy->SetRxFadingContainer (n->GetObject<SatBaseFading> ());

  Ptr<SatGwMac> mac = CreateObject<SatGwMac> ();

  mac->SetAttribute ("BBFrameConf", PointerValue (m_bbFrameConf));

  // TODO: Usage of multiple carriers needed to take into account, now only one carrier assumed to be used.
  // TODO: Symbol rate needed to check.
  double symbolrate = m_carrierBandwidthConverter (SatEnums::FORWARD_FEEDER_CH, 0, SatEnums::EFFECTIVE_BANDWIDTH);

  mac->SetAttribute ("SymbolRate", DoubleValue (symbolrate));

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback recCb = MakeCallback (&SatGwMac::Receive, mac);

  // Attach the NCC C/N0 update to Phy
  SatPhy::CnoCallback cnoCb = MakeCallback (&SatNcc::UtCnoUpdated, ncc);

  phy->SetAttribute ("ReceiveCb", CallbackValue(recCb));
  phy->SetAttribute ("CnoCb", CallbackValue(cnoCb));

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac (mac);

  // Create Logical Link Control (LLC) layer
  Ptr<SatLlc> llc = CreateObject<SatLlc> (false);

  // Attach the LLC layer to SatNetDevice
  dev->SetLlc (llc);

  // Create and set queues for Mac modules
  Ptr<Queue> queue = m_queueFactory.Create<Queue> ();
  llc->SetQueue (queue);

  // Attach the LLC Tx opportunity callback to SatMac
  mac->SetTxOpportunityCallback (MakeCallback (&SatLlc::NotifyTxOpportunity, llc));
  mac->SetSchedContextCallback (MakeCallback (&SatLlc::GetSchedulingContexts, llc));

  // Attach the device receive callback to SatNetDevice
  llc->SetReceiveCallback (MakeCallback (&SatNetDevice::Receive, dev));

  // Attach the transmit callback to PHY
  mac->SetTransmitCallback (MakeCallback (&SatPhy::SendPdu, phy));

  // Attach the device receive callback to SatLlc
  mac->SetReceiveCallback (MakeCallback (&SatLlc::Receive, llc));

  // Set the device address and pass it to MAC as well
  Mac48Address addr = Mac48Address::Allocate ();
  dev->SetAddress (addr);
  phy->SetAddress (Mac48Address::ConvertFrom (dev->GetAddress ()));

  Singleton<SatIdMapper>::Get ()->AttachMacToTraceId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToGwId (dev->GetAddress (),gwId);
  Singleton<SatIdMapper>::Get ()->AttachMacToBeamId (dev->GetAddress (),beamId);

  mac->StartScheduling ();

  phy->Initialize();

  return dev;
}

Ptr<NetDevice>
SatGwHelper::Install (std::string aName, uint32_t gwId, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << aName << beamId << fCh << rCh );

  Ptr<Node> a = Names::Find<Node> (aName);

  return Install (a, gwId, beamId, fCh, rCh, ncc);
}

void
SatGwHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this );

  TraceConnect ("Creation", "SatGwHelper", cb);
}

} // namespace ns3
