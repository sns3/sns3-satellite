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

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/queue.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/mpi-interface.h"
#include "ns3/mpi-receiver.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-mac.h"
#include "ns3/satellite-net-device.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-phy.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/virtual-channel.h"

#include "ns3/trace-helper.h"
#include "ns3/satellite-ut-helper.h"
#include "ns3/satellite-geo-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatUtHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtHelper);

TypeId
SatUtHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatUtHelper")
      .SetParent<Object> ()
      .AddConstructor<SatUtHelper> ()
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatUtHelper::m_creation))
    ;
    return tid;
}

TypeId
SatUtHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatUtHelper::SatUtHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  //LogComponentEnable ("SatUtHelper", LOG_LEVEL_INFO);
}

void 
SatUtHelper::SetQueue (std::string type,
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
SatUtHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
SatUtHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

NetDeviceContainer 
SatUtHelper::Install (NodeContainer c, uint16_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
    NetDeviceContainer devs;

    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add(Install(*i, beamId, fCh, rCh));
    }

    return devs;
}

Ptr<NetDevice>
SatUtHelper::Install (Ptr<Node> n, uint16_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  NetDeviceContainer container;

  // Create SatNetDevice
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  // Create the SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> phyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> phyRx = CreateObject<SatPhyRx> ();

  // Set SatChannels to SatPhyTx/SatPhyRx
  phyTx->SetChannel (rCh);
  phyRx->SetChannel (fCh);
  phyRx->SetDevice (dev);

  // By default, there is one carrier in the forward link
  uint16_t FEEDER_CARRIERS(1);
  phyRx->ConfigurePhyRxCarriers (FEEDER_CARRIERS);

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
SatUtHelper::Install (std::string aName, uint16_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  Ptr<Node> a = Names::Find<Node> (aName);
  return Install (a, beamId, fCh, rCh);
}

void
SatUtHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  TraceConnect("Creation", "SatUtHelper", cb);
}

} // namespace ns3
