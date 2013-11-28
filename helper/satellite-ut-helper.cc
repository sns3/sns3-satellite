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

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-channel.h"
#include "../model/satellite-mobility-observer.h"
#include "../model/satellite-llc.h"
#include "../model/satellite-ut-mac.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-ut-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-phy-rx-carrier-conf.h"
#include "../model/satellite-generic-encapsulator.h"
#include "../model/satellite-net-device.h"

#include "satellite-ut-helper.h"
#include "satellite-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatUtHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtHelper);

TypeId
SatUtHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatUtHelper")
      .SetParent<Object> ()
      .AddConstructor<SatUtHelper> ()
      .AddAttribute ("FwdLinkErrorModel",
                     "Forward link error model",
                     EnumValue (SatPhyRxCarrierConf::EM_AVI),
                     MakeEnumAccessor (&SatUtHelper::m_errorModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::EM_NONE, "None",
                                      SatPhyRxCarrierConf::EM_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::EM_AVI, "AVI"))
      .AddAttribute ("FwdLinkInterferenceModel",
                     "Forward link interference model",
                     EnumValue (SatPhyRxCarrierConf::IF_CONSTANT),
                     MakeEnumAccessor (&SatUtHelper::m_interferenceModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute ("CraAllocMode",
                     "Constant Rate Assignment (CRA) allocation mode used for UTs.",
                      EnumValue (SatUtHelper::CONSTANT_CRA),
                      MakeEnumAccessor (&SatUtHelper::m_craAllocMode),
                      MakeEnumChecker (SatUtHelper::CONSTANT_CRA, "Constant CRA used for UTs. (not set by helper, UT's attribute defines value.",
                                       SatUtHelper::RANDOM_CRA, "Random CRA value (128 is set by helper for every UT."))
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatUtHelper::m_creation))
    ;
    return tid;
}

TypeId
SatUtHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId();
}

SatUtHelper::SatUtHelper ()
{
  NS_LOG_FUNCTION (this);

  // this default constructor should be never called
  NS_ASSERT (false);
}

SatUtHelper::SatUtHelper (CarrierBandwidthConverter carrierBandwidthConverter, uint32_t fwdLinkCarrierCount, Ptr<SatSuperframeSeq> seq)
 : m_carrierBandwidthConverter (carrierBandwidthConverter),
   m_fwdLinkCarrierCount (fwdLinkCarrierCount),
   m_superframeSeq (seq)
{
  NS_LOG_FUNCTION (this << fwdLinkCarrierCount << seq );

  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  //LogComponentEnable ("SatUtHelper", LOG_LEVEL_INFO);
}

void
SatUtHelper::Initialize ()
{
  NS_LOG_FUNCTION (this);
  /*
   * Forward channel link results (DVB-S2) are created for UTs.
   */
  if (m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_linkResults = CreateObject<SatLinkResultsDvbS2> ();
      m_linkResults->Initialize ();
    }
}

void 
SatUtHelper::SetQueue (std::string type,
                              std::string n1, const AttributeValue &v1,
                              std::string n2, const AttributeValue &v2,
                              std::string n3, const AttributeValue &v3,
                              std::string n4, const AttributeValue &v4)
{
  NS_LOG_FUNCTION (this << type << n1 << n2 << n3 << n4 );

  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void 
SatUtHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void 
SatUtHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_channelFactory.Set (n1, v1);
}

void
SatUtHelper::SetPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatUtPhy::" + n1, v1);
}

NetDeviceContainer 
SatUtHelper::Install (NodeContainer c, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << beamId << fCh << rCh );

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add (Install (*i, beamId, fCh, rCh, gwNd, ncc));
  }

  return devs;
}

Ptr<NetDevice>
SatUtHelper::Install (Ptr<Node> n, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << n << beamId << fCh << rCh );

  NetDeviceContainer container;

  // Create SatNetDevice
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  // Attach the SatNetDevice to node
  n->AddDevice (dev);

  SatPhy::CreateParam_t params;
  params.m_beamId = beamId;
  params.m_device = dev;
  params.m_txCh = rCh;
  params.m_rxCh = fCh;

  Ptr<SatUtPhy> phy = CreateObject<SatUtPhy> (params, m_errorModel, m_linkResults, m_interferenceModel,
                                              m_carrierBandwidthConverter, m_fwdLinkCarrierCount);

  // Set fading
  phy->SetTxFadingContainer (n->GetObject<SatBaseFading> ());
  phy->SetRxFadingContainer (n->GetObject<SatBaseFading> ());

  Ptr<SatUtMac> mac = CreateObject<SatUtMac> (m_superframeSeq);

  if ( m_craAllocMode == SatUtHelper::RANDOM_CRA )
    {
      Ptr<UniformRandomVariable> craRnd = CreateObject<UniformRandomVariable> ();
      mac->SetAttribute ( "Cra", DoubleValue ( craRnd->GetValue (0.0, std::numeric_limits<double>::max ())));
    }

  // Set timing advance callback to mac.
  Ptr<SatMobilityObserver> observer = n->GetObject<SatMobilityObserver> ();
  NS_ASSERT (observer != NULL);
  SatUtMac::TimingAdvanceCallback timingCb = MakeCallback (&SatMobilityObserver::GetTimingAdvance, observer);
  mac->SetTimingAdvanceCallback (timingCb);

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback cb = MakeCallback (&SatUtMac::Receive, mac);

  phy->SetAttribute ("ReceiveCb", CallbackValue (cb));

  // Create Logical Link Control (LLC) layer
  Ptr<SatLlc> llc = CreateObject<SatLlc> (true);

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac (mac);

  // Attach the LLC layer to SatNetDevice
  dev->SetLlc (llc);

  // Set the device address and pass it to MAC as well
  Mac48Address addr = Mac48Address::Allocate ();
  dev->SetAddress (addr);
  phy->SetAddress (addr);

  // Create encapsulator and add it to UT's LLC
  Mac48Address gwAddr = Mac48Address::ConvertFrom (gwNd->GetAddress());
  Ptr<SatGenericEncapsulator> utEncap = CreateObject<SatGenericEncapsulator> (addr, gwAddr);
  Ptr<SatGenericEncapsulator> utDecap = CreateObject<SatGenericEncapsulator> (gwAddr, addr);
  utDecap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, llc));
  llc->AddEncap (addr, utEncap); // Tx
  llc->AddDecap (addr, utDecap); // Rx

  // Create encapsulator and add it to GW's LLC
  Ptr<SatGenericEncapsulator> gwEncap = CreateObject<SatGenericEncapsulator> (gwAddr, addr);
  Ptr<SatGenericEncapsulator> gwDecap = CreateObject<SatGenericEncapsulator> (addr, gwAddr);
  Ptr<SatLlc> gwLlc = gwNd->GetLlc ();
  gwLlc->AddEncap (addr, gwEncap); // Tx
  gwLlc->AddDecap (addr, gwDecap); // Rx
  gwDecap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, gwLlc));

  // Create and set control packet queue to LLC
  Ptr<Queue> queue = m_queueFactory.Create<Queue> ();
  llc->SetQueue (queue);

  // Attach the transmit callback to PHY
  mac->SetTransmitCallback (MakeCallback (&SatPhy::SendPdu, phy));

  // Attach the LLC receive callback to SatMac
  mac->SetReceiveCallback (MakeCallback (&SatLlc::Receive, llc));

  // Attach the LLC Tx opportunity callback to SatMac
  mac->SetTxOpportunityCallback (MakeCallback (&SatLlc::NotifyTxOpportunity, llc));

  // Attach the device receive callback to SatMac
  llc->SetReceiveCallback (MakeCallback (&SatNetDevice::Receive, dev));

  // Add UT to NCC
  DoubleValue macCra (0.0);
  mac->GetAttribute ( "Cra", macCra );
  ncc->AddUt (dev->GetAddress (), macCra.Get (), beamId);

  phy->Initialize();

  return dev;
}

Ptr<NetDevice>
SatUtHelper::Install (std::string aName, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << aName << beamId << fCh << rCh );

  Ptr<Node> a = Names::Find<Node> (aName);

  return Install (a, beamId, fCh, rCh, gwNd, ncc);
}

void
SatUtHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect("Creation", "SatUtHelper", cb);
}

} // namespace ns3
