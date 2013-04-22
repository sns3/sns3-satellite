/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/satellite-channel.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/satellite-phy-tx.h"
#include <ns3/object.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/packet.h>
#include <ns3/net-device.h>
#include <ns3/node.h>
#include <ns3/propagation-delay-model.h>
#include <algorithm>


NS_LOG_COMPONENT_DEFINE ("SatChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatChannel);


SatChannel::SatChannel ()
{
  NS_LOG_FUNCTION (this);
}

SatChannel::~SatChannel ()
{
}

void
SatChannel::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phyList.clear ();
  m_propagationDelay = 0;
  Channel::DoDispose ();
}

TypeId
SatChannel::GetTypeId (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static TypeId tid = TypeId ("ns3::SatChannel")
    .SetParent<Channel> ()
    .AddConstructor<SatChannel> ()
  ;
  return tid;
}

void
SatChannel::AddRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);
  m_phyList.push_back (phyRx);
}

void
SatChannel::RemoveRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);

  PhyList::iterator phyIter = std::find(m_phyList.begin(), m_phyList.end(), phyRx);

  if (phyIter != m_phyList.end()) // == vector.end() means the element was not found
    {
      m_phyList.erase (phyIter);
    }
}

void
SatChannel::StartTx (Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this << txParams->m_duration << txParams->m_phyTx);
  NS_ASSERT_MSG (txParams->m_phyTx, "NULL phyTx");

  Ptr<MobilityModel> senderMobility = txParams->m_phyTx->GetMobility ();

  for (PhyList::const_iterator rxPhyIterator = m_phyList.begin ();
       rxPhyIterator != m_phyList.end ();
       ++rxPhyIterator)
    {
      Time delay = MicroSeconds (0);

      Ptr<MobilityModel> receiverMobility = (*rxPhyIterator)->GetMobility ();
      NS_LOG_LOGIC ("copying signal parameters " << txParams);
      Ptr<SatSignalParameters> rxParams = txParams->Copy ();

      if (m_propagationDelay)
        {
          delay = m_propagationDelay->GetDelay (senderMobility, receiverMobility);
          NS_LOG_LOGIC("Time: " << Simulator::Now ().GetSeconds () << ": setting propagation delay: " << delay);
        }

      Ptr<SatNetDevice> netDev = (*rxPhyIterator)->GetDevice ();
      uint32_t dstNode =  netDev->GetNode ()->GetId ();
      Simulator::ScheduleWithContext (dstNode, delay, &SatChannel::StartRx, this, rxParams, *rxPhyIterator);
    }
}


void
SatChannel::StartRx (Ptr<SatSignalParameters> rxParams, Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << rxParams);

  /*
   * Here we should calculate the received signal strength for this specific
   * transmitter/receiver pair at this time moment.
   */

  phyRx->StartRx (rxParams);
}


void
SatChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  NS_LOG_FUNCTION (this << delay);
  NS_ASSERT (m_propagationDelay == 0);
  m_propagationDelay = delay;
}


uint32_t
SatChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyList.size ();
}


Ptr<NetDevice>
SatChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_phyList.at (i)->GetDevice ()->GetObject<SatNetDevice> ();
}

} // namespace ns3
