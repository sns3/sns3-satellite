/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/random-variable-stream.h>
#include <ns3/double.h>
#include <ns3/string.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/simulator.h>
#include <ns3/log.h>

#include <ns3/lora-forwarder.h>

#include "lora-forwarder-helper.h"

NS_LOG_COMPONENT_DEFINE ("LoraForwarderHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoraForwarderHelper);

TypeId
LoraForwarderHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraForwarderHelper")
    .SetParent<Object> ()
    .AddConstructor<LoraForwarderHelper> ()
  ;
  return tid;
}

LoraForwarderHelper::LoraForwarderHelper ()
{
  m_factory.SetTypeId ("ns3::LoraForwarder");
}

LoraForwarderHelper::~LoraForwarderHelper ()
{
}

void
LoraForwarderHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
LoraForwarderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LoraForwarderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LoraForwarderHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  Ptr<LoraForwarder> app = m_factory.Create<LoraForwarder> ();

  app->SetNode (node);
  node->AddApplication (app);

  // Link the LoraForwarder to the SatLorawanNetDevices
  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      Ptr<NetDevice> currentNetDevice = node->GetDevice (i);
      if (currentNetDevice->GetObject<SatLorawanNetDevice> () != 0)
        {
          Ptr<SatLorawanNetDevice> loraNetDevice = currentNetDevice->GetObject<SatLorawanNetDevice> ();
          app->SetLoraNetDevice (loraNetDevice);
          loraNetDevice->SetReceiveNetworkServerCallback (MakeCallback (&LoraForwarder::ReceiveFromLora, app));
        }
      else if (currentNetDevice->GetObject<PointToPointNetDevice> () != 0)
        {
          Ptr<PointToPointNetDevice> pointToPointNetDevice = currentNetDevice->GetObject<PointToPointNetDevice> ();
          app->SetPointToPointNetDevice (pointToPointNetDevice);
          pointToPointNetDevice->SetReceiveCallback (MakeCallback (&LoraForwarder::ReceiveFromPointToPoint, app));
        }
    }

  return app;
}
} // namespace ns3
