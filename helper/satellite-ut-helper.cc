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
#include "../model/satellite-generic-stream-encapsulator.h"
#include "../model/satellite-return-link-encapsulator.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-node-info.h"
#include "../model/satellite-enums.h"
#include "../model/satellite-request-manager.h"
#include "../model/satellite-queue.h"
#include "satellite-ut-helper.h"
#include "ns3/singleton.h"
#include "ns3/satellite-id-mapper.h"

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
                                      SatPhyRxCarrierConf::IF_TRACE, "Trace",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute ("RandomAccessModel",
                     "Random Access Model",
                     EnumValue (SatRandomAccess::RA_OFF),
                     MakeEnumAccessor (&SatUtHelper::m_randomAccessModel),
                     MakeEnumChecker (SatRandomAccess::RA_OFF, "RA not in use.",
                                      SatRandomAccess::RA_SLOTTED_ALOHA, "Slotted ALOHA",
                                      SatRandomAccess::RA_CRDSA, "CRDSA",
                                      SatRandomAccess::RA_ANY_AVAILABLE, "Any available"))
      .AddAttribute ("LowerLayerServiceConf",
                     "Pointer to lower layer service configuration.",
                     PointerValue (),
                     MakePointerAccessor (&SatUtHelper::m_llsConf),
                     MakePointerChecker<SatLowerLayerServiceConf> ())
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatUtHelper::m_creationTrace))
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
 : m_carrierBandwidthConverter (),
   m_fwdLinkCarrierCount (),
   m_superframeSeq (),
   m_interferenceModel (),
   m_errorModel (),
   m_linkResults (),
   m_randomAccessModel (SatRandomAccess::RA_OFF)
{
  NS_LOG_FUNCTION (this);

  // this default constructor should be never called
  NS_FATAL_ERROR ("SatUtHelper::SatUtHelper - Constructor not in use");
}

SatUtHelper::SatUtHelper (CarrierBandwidthConverter carrierBandwidthConverter, uint32_t fwdLinkCarrierCount, Ptr<SatSuperframeSeq> seq)
 : m_carrierBandwidthConverter (carrierBandwidthConverter),
   m_fwdLinkCarrierCount (fwdLinkCarrierCount),
   m_superframeSeq (seq),
   m_interferenceModel (),
   m_errorModel (),
   m_linkResults (),
   m_randomAccessModel (SatRandomAccess::RA_OFF)
{
  NS_LOG_FUNCTION (this << fwdLinkCarrierCount << seq );
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  m_llsConf = CreateObject<SatLowerLayerServiceConf>  ();

  //LogComponentEnable ("SatUtHelper", LOG_LEVEL_INFO);
}

void
SatUtHelper::Initialize (Ptr<SatLinkResultsDvbS2> lrS2)
{
  NS_LOG_FUNCTION (this);
  /*
   * Forward channel link results (DVB-S2) are created for UTs.
   */
  if (lrS2 && m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_linkResults = lrS2;
    }
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
SatUtHelper::Install (NodeContainer c, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc)
{
  NS_LOG_FUNCTION (this << beamId << fCh << rCh );

  NetDeviceContainer devs;

  Ptr<SatRandomAccessConf> randomAccessConf = NULL;

  if (m_randomAccessModel != SatRandomAccess::RA_OFF)
    {
      randomAccessConf = CreateObject<SatRandomAccessConf> (m_llsConf);
    }

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add (Install (*i, beamId, fCh, rCh, gwNd, ncc, randomAccessConf));
  }

  return devs;
}

Ptr<NetDevice>
SatUtHelper::Install (Ptr<Node> n, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNetDevice> gwNd, Ptr<SatNcc> ncc, Ptr<SatRandomAccessConf> randomAccessConf)
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

  Ptr<SatUtMac> mac = CreateObject<SatUtMac> (m_superframeSeq, beamId, randomAccessConf, m_randomAccessModel);
  mac->SetAttribute ("LowerLayerServiceConf", PointerValue (m_llsConf));

  // Set timing advance callback to mac.
  Ptr<SatMobilityObserver> observer = n->GetObject<SatMobilityObserver> ();
  NS_ASSERT (observer != NULL);

  SatUtMac::TimingAdvanceCallback timingCb = MakeCallback (&SatMobilityObserver::GetTimingAdvance, observer);
  mac->SetTimingAdvanceCallback (timingCb);

  SatUtMac::SendCallback txCb = MakeCallback (&SatNetDevice::SendControl, dev);
  mac->SetTxCallback (txCb);

  // Attach the Mac layer C/N0 updates receiver to Phy
  SatPhy::CnoCallback cnoCb = MakeCallback (&SatUtMac::CnoUpdated, mac);

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback recCb = MakeCallback (&SatUtMac::Receive, mac);

  // Attach the NCC C/N0 update to Phy

  phy->SetAttribute ("CnoCb", CallbackValue (cnoCb));
  phy->SetAttribute ("ReceiveCb", CallbackValue(recCb));

  // Create Logical Link Control (LLC) layer
  Ptr<SatLlc> llc = CreateObject<SatLlc> ();

  // Create a request manager and attach it to LLC
  Ptr<SatRequestManager> rm = CreateObject<SatRequestManager> ();
  llc->AddRequestManager (rm);

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac (mac);

  // Attach the LLC layer to SatNetDevice
  dev->SetLlc (llc);

  // Set the device address and pass it to MAC as well
  Mac48Address addr = Mac48Address::Allocate ();
  dev->SetAddress (addr);

  Singleton<SatIdMapper>::Get ()->AttachMacToTraceId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToUtId (dev->GetAddress ());
  Singleton<SatIdMapper>::Get ()->AttachMacToBeamId (dev->GetAddress (),beamId);

  // Create encapsulator and add it to UT's LLC
  Mac48Address gwAddr = Mac48Address::ConvertFrom (gwNd->GetAddress());

  Ptr<SatLlc> gwLlc = gwNd->GetLlc ();

  // Return link
  uint32_t numSupportedRcs (2);
  // Creation of encapsulators start always from RC index number 1, since 0
  // is reserved for control.
  for (uint32_t rc = 1; rc <= numSupportedRcs; ++rc)
    {
      Ptr<SatReturnLinkEncapsulator> utEncap = CreateObject<SatReturnLinkEncapsulator> (addr, gwAddr, rc);
      llc->AddEncap (addr, utEncap, rc); // Tx

      // Create encapsulator and add it to GW's LLC
      Ptr<SatReturnLinkEncapsulator> gwDecap = CreateObject<SatReturnLinkEncapsulator> (addr, gwAddr, rc);
      gwLlc->AddDecap (addr, gwDecap, rc); // Rx
      gwDecap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, gwLlc));
    }

  // Forward link
  Ptr<SatGenericStreamEncapsulator> gwEncap = CreateObject<SatGenericStreamEncapsulator> (gwAddr, addr);
  Ptr<SatGenericStreamEncapsulator> utDecap = CreateObject<SatGenericStreamEncapsulator> (gwAddr, addr);
  utDecap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, llc));
  gwLlc->AddEncap (addr, gwEncap, 1); // Tx
  llc->AddDecap (addr, utDecap, 1); // Rx

  // set serving GW MAC address to UT MAC
  mac->SetGwAddress (gwAddr);

  // Create and set control packet queue to LLC
  Ptr<SatQueue> queue = CreateObject<SatQueue> (0);
  llc->SetQueue (queue);

  // Callback to Request manager
  SatQueue::QueueEventCallback rmCb = MakeCallback (&SatRequestManager::ReceiveQueueEvent, rm);
  queue->AddQueueEventCallback (rmCb);

  // Callback to UT MAC
  SatQueue::QueueEventCallback macCb = MakeCallback (&SatUtMac::ReceiveQueueEvent, mac);
  queue->AddQueueEventCallback (macCb);

  // Attach the transmit callback to PHY
  mac->SetTransmitCallback (MakeCallback (&SatPhy::SendPdu, phy));

  // Attach the LLC receive callback to SatMac
  mac->SetReceiveCallback (MakeCallback (&SatLlc::Receive, llc));

  // Attach the LLC Tx opportunity callback to SatMac
  mac->SetTxOpportunityCallback (MakeCallback (&SatLlc::NotifyTxOpportunity, llc));

  // Attach the device receive callback to SatMac
  llc->SetReceiveCallback (MakeCallback (&SatNetDevice::Receive, dev));

  // Add UT to NCC
  ncc->AddUt (dev->GetAddress (), m_llsConf, beamId);

  phy->Initialize();

  llc->SetQueueSatisticsCallbacks ();

  // Create a node info to all the protocol layers
  Ptr<SatNodeInfo> nodeInfo = Create <SatNodeInfo> (SatEnums::NT_UT, n->GetId (), Mac48Address::ConvertFrom (addr));
  dev->SetNodeInfo (nodeInfo);
  llc->SetNodeInfo (nodeInfo);
  mac->SetNodeInfo (nodeInfo);
  phy->SetNodeInfo (nodeInfo);

  return dev;
}

void
SatUtHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect("Creation", "SatUtHelper", cb);
}

} // namespace ns3
