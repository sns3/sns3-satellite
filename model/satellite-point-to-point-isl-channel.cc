/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 * (Based on point-to-point channel)
 * Author: Andre Aguas    March 2020
 * Adapted to SNS-3 by: Bastien Tauran <bastien.tauran@viveris.fr>
 * 
 */


#include "ns3/satellite-point-to-point-isl-channel.h"
#include "ns3/satellite-const-variables.h"
#include "ns3/core-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PointToPointIslChannel");

NS_OBJECT_ENSURE_REGISTERED (PointToPointIslChannel);

TypeId 
PointToPointIslChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PointToPointIslChannel")
    .SetParent<Channel> ()
    .AddConstructor<PointToPointIslChannel> ()
    .AddAttribute ("PropagationSpeed", "Propagation speed through the channel",
                   DoubleValue (SatConstVariables::SPEED_OF_LIGHT),
                   MakeDoubleAccessor (&PointToPointIslChannel::m_propagationSpeed),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

PointToPointIslChannel::PointToPointIslChannel()
  :
    Channel (),
    m_nDevices (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
PointToPointIslChannel::Attach (Ptr<PointToPointIslNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT_MSG (m_nDevices < N_DEVICES, "Only two devices permitted");
  NS_ASSERT (device != 0);

  m_link[m_nDevices++].m_src = device;
//
// If we have both devices connected to the channel, then finish introducing
// the two halves and set the links to IDLE.
//
  if (m_nDevices == N_DEVICES)
    {
      m_link[0].m_dst = m_link[1].m_src;
      m_link[1].m_dst = m_link[0].m_src;
      m_link[0].m_state = IDLE;
      m_link[1].m_state = IDLE;
    }
}

bool
PointToPointIslChannel::TransmitStart (Ptr<const Packet> p,
                                       Ptr<PointToPointIslNetDevice> src,
                                       Ptr<Node> dst,
                                       Time txTime)
{
  NS_LOG_FUNCTION (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);

  Ptr<MobilityModel> senderMobility = src->GetNode()->GetObject<MobilityModel>();
  Ptr<MobilityModel> receiverMobility = dst->GetObject<MobilityModel>();
  Time delay = this->GetDelay(senderMobility, receiverMobility); 

  uint32_t wire = src == m_link[0].m_src ? 0 : 1;

  Simulator::ScheduleWithContext (m_link[wire].m_dst->GetNode()->GetId (),
                                  txTime + delay, &PointToPointIslNetDevice::Receive,
                                  m_link[wire].m_dst, p->Copy ());

  return true;
}

std::size_t
PointToPointIslChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_nDevices;
}

Ptr<PointToPointIslNetDevice>
PointToPointIslChannel::GetPointToPointIslDevice (std::size_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (i < 2);
  return m_link[i].m_src;
}

Ptr<NetDevice>
PointToPointIslChannel::GetDevice (std::size_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetPointToPointIslDevice (i);
}

Time
PointToPointIslChannel::GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double distance = a->GetDistanceFrom (b);
  double seconds = distance / m_propagationSpeed;
  return Seconds (seconds);
}

Ptr<PointToPointIslNetDevice>
PointToPointIslChannel::GetSource (uint32_t i) const
{
  return m_link[i].m_src;
}

Ptr<PointToPointIslNetDevice>
PointToPointIslChannel::GetDestination (uint32_t i) const
{
  return m_link[i].m_dst;
}

bool
PointToPointIslChannel::IsInitialized (void) const
{
  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);
  return true;
}

} // namespace ns3
