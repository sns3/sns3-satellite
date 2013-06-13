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
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
//#include "../model/satellite-channel.h"
#include "../model/satellite-geo-net-device.h"
#include "../model/satellite-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"

#include "satellite-geo-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGeoHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoHelper);

TypeId
SatGeoHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatGeoHelper")
      .SetParent<Object> ()
      .AddConstructor<SatGeoHelper> ()
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatGeoHelper::m_creation))
    ;
    return tid;
}

TypeId
SatGeoHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatGeoHelper::SatGeoHelper ()
  :m_deviceCount(0)
{
  m_deviceFactory.SetTypeId ("ns3::SatGeoNetDevice");
}

void 
SatGeoHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

NetDeviceContainer 
SatGeoHelper::Install (NodeContainer c)
{
  // currently only one node supported by helper
  NS_ASSERT (c.GetN () == 1);

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add(Install(*i));
  }

  return devs;
}

Ptr<NetDevice>
SatGeoHelper::Install (Ptr<Node> n)
{
  NS_ASSERT (m_deviceCount == 0);

  // Create SatGeoNetDevice
  Ptr<SatGeoNetDevice> satDev = m_deviceFactory.Create<SatGeoNetDevice> ();

  satDev->SetAddress (Mac48Address::Allocate ());
  n->AddDevice(satDev);
  m_deviceCount++;

  return satDev;
}

Ptr<NetDevice>
SatGeoHelper::Install (std::string nName)
{
  Ptr<Node> n = Names::Find<Node> (nName);
  return Install (n);
}

void
SatGeoHelper::AttachChannels (Ptr<NetDevice> d, Ptr<SatChannel> ff, Ptr<SatChannel> fr, Ptr<SatChannel> uf, Ptr<SatChannel> ur, uint16_t beamId )
{
  NS_LOG_FUNCTION (this << ff << fr << uf << ur);

  Ptr<SatGeoNetDevice> dev = DynamicCast<SatGeoNetDevice> (d);

  // Create the first needed SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> uPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> uPhyRx = CreateObject<SatPhyRx> ();
  Ptr<SatPhyTx> fPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> fPhyRx = CreateObject<SatPhyRx> ();

  // Set SatChannels to SatPhyTx/SatPhyRx
  uPhyTx->SetChannel (uf);
  uPhyRx->SetChannel (ur);
  uPhyRx->SetDevice (dev);

  // Configure the SatPhyRxCarrier instances
  // Note, that these have to be changed so that they match the real frame configuration.
  // Now we survive with one SatPhyRxCarrier, because we do not have a NCC scheduler.
  uint16_t RETURN_CARRIERS(1);
  uPhyRx->ConfigurePhyRxCarriers (RETURN_CARRIERS);

  fPhyTx->SetChannel (fr);
  fPhyRx->SetChannel (ff);
  fPhyRx->SetDevice (dev);

  // By default, there is one carrier in the forward link
  uint16_t FORWARD_CARRIERS(1);
  fPhyRx->ConfigurePhyRxCarriers (FORWARD_CARRIERS);

  SatPhy::ReceiveCallback uCb = MakeCallback (&SatGeoNetDevice::ReceiveUser, dev);
  SatPhy::ReceiveCallback fCb = MakeCallback (&SatGeoNetDevice::ReceiveFeeder, dev);

  // Create SatPhy modules
  Ptr<SatPhy> uPhy = CreateObject<SatPhy> (uPhyTx, uPhyRx, beamId, uCb);
  Ptr<SatPhy> fPhy = CreateObject<SatPhy> (fPhyTx, fPhyRx, beamId, fCb);

  dev->AddUserPhy(uPhy, beamId);
  dev->AddFeederPhy(fPhy, beamId);
}

void
SatGeoHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  TraceConnect("Creation", "SatGeoHelper",  cb);
}

} // namespace ns3
