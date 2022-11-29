/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * (based on point-to-point helper)
 * Author: Andre Aguas         March 2020
 *         Simon               2020
 * Adapted to SNS-3 by: Bastien Tauran <bastien.tauran@viveris.fr>
 */


#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/packet.h"
#include "ns3/queue.h"

#include "ns3/satellite-point-to-point-isl-channel.h"
#include "ns3/satellite-const-variables.h"
#include "ns3/net-device-queue-interface.h"

#include "ns3/satellite-point-to-point-isl-helper.h"

NS_LOG_COMPONENT_DEFINE ("PointToPointIslHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PointToPointIslHelper);

TypeId
PointToPointIslHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PointToPointIslHelper")
    .SetParent<Object> ()
    .AddConstructor<PointToPointIslHelper> ()
  ;
  return tid;
}

PointToPointIslHelper::PointToPointIslHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
  m_deviceFactory.SetTypeId ("ns3::PointToPointIslNetDevice");
  m_channelFactory.SetTypeId ("ns3::PointToPointIslChannel");
}

NetDeviceContainer 
PointToPointIslHelper::Install (Ptr<Node> a, Ptr<Node> b)
{
  NS_LOG_FUNCTION (this << a << b);

  NetDeviceContainer container;

  Ptr<PointToPointIslNetDevice> devA = m_deviceFactory.Create<PointToPointIslNetDevice> ();
  devA->SetAddress (Mac48Address::Allocate ());
  devA->SetDestinationNode(b);
  a->AddDevice (devA);
  Ptr<Queue<Packet> > queueA = m_queueFactory.Create<Queue<Packet> > ();
  devA->SetQueue (queueA);
  Ptr<PointToPointIslNetDevice> devB = m_deviceFactory.Create<PointToPointIslNetDevice> ();
  devB->SetAddress (Mac48Address::Allocate ());
  devB->SetDestinationNode(a);
  b->AddDevice (devB);
  Ptr<Queue<Packet> > queueB = m_queueFactory.Create<Queue<Packet> > ();
  devB->SetQueue (queueB);

  // Aggregate NetDeviceQueueInterface objects
  Ptr<NetDeviceQueueInterface> ndqiA = CreateObject<NetDeviceQueueInterface> ();
  ndqiA->GetTxQueue (0)->ConnectQueueTraces (queueA);
  devA->AggregateObject (ndqiA);
  Ptr<NetDeviceQueueInterface> ndqiB = CreateObject<NetDeviceQueueInterface> ();
  ndqiB->GetTxQueue (0)->ConnectQueueTraces (queueB);
  devB->AggregateObject (ndqiB);

  // Create and attach channel
  Ptr<PointToPointIslChannel> channel = m_channelFactory.Create<PointToPointIslChannel> ();
  devA->Attach (channel);
  devB->Attach (channel);
  container.Add (devA);
  container.Add (devB);

  return container;
}

} // namespace ns3
