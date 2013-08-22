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
#include "../model/satellite-channel.h"
#include "../model/satellite-mac.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-geo-net-device.h"
#include "../model/satellite-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/virtual-channel.h"
#include "../model/satellite-phy-rx-carrier-conf.h"
#include "../model/satellite-link-results.h"
#include "ns3/satellite-gw-helper.h"

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
  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  //LogComponentEnable ("SatGwHelper", LOG_LEVEL_INFO);

  m_beamId = 1;
}

void
SatGwHelper::Initialize ()
{
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
  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void 
SatGwHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
SatGwHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

NetDeviceContainer 
SatGwHelper::Install (NodeContainer c, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
    NetDeviceContainer devs;

    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add(Install(*i, beamId, fCh, rCh));
    }

    return devs;
}

Ptr<NetDevice>
SatGwHelper::Install (Ptr<Node> n, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  NetDeviceContainer container;

  // Create SatNetDevice
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  // Create the SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> phyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> phyRx = CreateObject<SatPhyRx> ();

  // Set SatChannels to SatPhyTx/SatPhyRx
  phyTx->SetChannel (fCh);
  phyRx->SetChannel (rCh);
  phyRx->SetDevice (dev);

  // Configure the SatPhyRxCarrier instances
  // \todo We should pass the whole carrier configuration to the SatPhyRxCarrier,
  // instead of just the number of carriers, since it should hold information about
  // the number of carriers, carrier center frequencies and carrier bandwidths, etc.
  uint32_t rtnLinkNumCarriers = 1;
  Ptr<SatPhyRxCarrierConf> carrierConf = CreateObject<SatPhyRxCarrierConf> (rtnLinkNumCarriers,
                                                                            m_errorModel,
                                                                            m_interferenceModel);

  // If the link results are created, we pass those
  // to SatPhyRxCarrier for error modeling
  if (m_linkResults)
    {
      carrierConf->SetLinkResults (m_linkResults);
    }

  phyRx->ConfigurePhyRxCarriers (carrierConf);

  Ptr<SatMac> mac = CreateObject<SatMac> ();

  // Create and set queues for Mac modules
  Ptr<Queue> queue = m_queueFactory.Create<Queue> ();
  mac->SetQueue (queue);

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback cb = MakeCallback (&SatMac::Receive, mac);

  // Create SatPhy modules
  Ptr<SatPhy> phy = CreateObject<SatPhy> (phyTx, phyRx, beamId, cb);

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the PHY layer to SatMac
  mac->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac(mac);

  // Set the device address and pass it to MAC as well
  dev->SetAddress (Mac48Address::Allocate ());

  // Attach the device receive callback to SatMac
  mac->SetReceiveCallback (MakeCallback (&SatNetDevice::ReceiveMac, dev));

  // Attach the SatNetDevices to nodes
  n->AddDevice (dev);

  return DynamicCast <NetDevice> (dev);
}

Ptr<NetDevice>
SatGwHelper::Install (std::string aName, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  Ptr<Node> a = Names::Find<Node> (aName);
  return Install (a, beamId, fCh, rCh);
}

void
SatGwHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  TraceConnect("Creation", "SatGwHelper", cb);
}

} // namespace ns3
