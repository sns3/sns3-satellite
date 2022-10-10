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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/object-map.h>
#include <ns3/error-model.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/ipv4-header.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/channel.h>
#include <ns3/uinteger.h>

#include "satellite-geo-net-device.h"
#include "satellite-phy.h"
#include "satellite-geo-feeder-phy.h"
#include "satellite-geo-user-phy.h"
#include "satellite-phy-tx.h"
#include "satellite-phy-rx.h"
#include "satellite-mac.h"
#include "satellite-geo-feeder-mac.h"
#include "satellite-geo-user-mac.h"
#include "satellite-channel.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoNetDevice);

TypeId
SatGeoNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<SatGeoNetDevice> ()
    .AddAttribute ("ReceiveErrorModel",
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&SatGeoNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("UserPhy", "The User Phy objects attached to this device.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&SatGeoNetDevice::m_userPhy),
                   MakeObjectMapChecker<SatPhy> ())
    .AddAttribute ("FeederPhy", "The Feeder Phy objects attached to this device.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&SatGeoNetDevice::m_feederPhy),
                   MakeObjectMapChecker<SatPhy> ())
    .AddAttribute ("UserMac", "The User MAC objects attached to this device.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&SatGeoNetDevice::m_userMac),
                   MakeObjectMapChecker<SatMac> ())
    .AddAttribute ("FeederMac", "The Feeder MAC objects attached to this device.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&SatGeoNetDevice::m_feederMac),
                   MakeObjectMapChecker<SatMac> ())
  ;
  return tid;
}

SatGeoNetDevice::SatGeoNetDevice ()
  : m_node (0),
  m_mtu (0xffff),
  m_ifIndex (0)
{
  NS_LOG_FUNCTION (this);
}


void
SatGeoNetDevice::ReceiveUser (SatPhy::PacketContainer_t /*packets*/, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams->m_packetsInBurst.size () << rxParams);
  NS_LOG_INFO ("Receiving a packet at the satellite from user link");

  switch (m_returnLinkRegenerationMode)
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          DynamicCast<SatGeoFeederPhy> (m_feederPhy[rxParams->m_beamId])->SendPduWithParams (rxParams);
          break;
        }
      case SatEnums::REGENERATION_LINK:
        {
          DynamicCast<SatGeoFeederMac> (m_feederMac[rxParams->m_beamId])->EnquePackets (rxParams->m_packetsInBurst, rxParams);
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Not implemented yet");
        }
    }
}

void
SatGeoNetDevice::ReceiveFeeder (SatPhy::PacketContainer_t /*packets*/, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams->m_packetsInBurst.size () << rxParams);
  NS_LOG_INFO ("Receiving a packet at the satellite from feeder link");

  switch (m_forwardLinkRegenerationMode)
    {
      case SatEnums::TRANSPARENT:
      case SatEnums::REGENERATION_PHY:
        {
          DynamicCast<SatGeoUserPhy> (m_userPhy[rxParams->m_beamId])->SendPduWithParams (rxParams);
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Not implemented yet");
        }
    }
}

void
SatGeoNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

void
SatGeoNetDevice::SetForwardLinkRegenerationMode (SatEnums::RegenerationMode_t forwardLinkRegenerationMode)
{
  m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
}

void
SatGeoNetDevice::SetReturnLinkRegenerationMode (SatEnums::RegenerationMode_t returnLinkRegenerationMode)
{
  m_returnLinkRegenerationMode = returnLinkRegenerationMode;
}

void
SatGeoNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}
uint32_t
SatGeoNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}
void
SatGeoNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}
Address
SatGeoNetDevice::GetAddress (void) const
{
  //
  // Implicit conversion from Mac48Address to Address
  //
  NS_LOG_FUNCTION (this);
  return m_address;
}
bool
SatGeoNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}
uint16_t
SatGeoNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}
bool
SatGeoNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
void
SatGeoNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this << &callback);
}
bool
SatGeoNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}
Address
SatGeoNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}
bool
SatGeoNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
Address
SatGeoNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address SatGeoNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

bool
SatGeoNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
SatGeoNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
SatGeoNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  /**
   * The satellite does not have higher protocol layers which
   * utilize the Send method! Thus, this method should not be used!
   */
  NS_ASSERT (false);
  return false;
}
bool
SatGeoNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);

  /**
   * The satellite does not have higher protocol layers which
   * utilize the SendFrom method! Thus, this method should not be used!
   */
  NS_ASSERT (false);
  return false;
}

Ptr<Node>
SatGeoNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}
void
SatGeoNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}
bool
SatGeoNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
void
SatGeoNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
}

void
SatGeoNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_receiveErrorModel = 0;
  m_userPhy.clear ();
  m_feederPhy.clear ();
  m_userMac.clear ();
  m_feederMac.clear ();
  NetDevice::DoDispose ();
}

void
SatGeoNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscCallback = cb;
}

bool
SatGeoNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Ptr<Channel>
SatGeoNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return NULL;
}

void
SatGeoNetDevice::AddUserPhy (Ptr<SatPhy> phy, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << phy << beamId);
  m_userPhy.insert (std::pair<uint32_t, Ptr<SatPhy> > (beamId, phy));
}

void
SatGeoNetDevice::AddFeederPhy (Ptr<SatPhy> phy, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << phy << beamId);
  m_feederPhy.insert (std::pair<uint32_t, Ptr<SatPhy> > (beamId, phy));
}

Ptr<SatPhy>
SatGeoNetDevice::GetUserPhy (uint32_t beamId)
{
  if (m_userPhy.count(beamId))
    {
      return m_userPhy[beamId];
    }
  NS_FATAL_ERROR ("User Phy does not exist for beam " << beamId);
}

Ptr<SatPhy>
SatGeoNetDevice::GetFeederPhy (uint32_t beamId)
{
  if (m_userPhy.count(beamId))
    {
      return m_feederPhy[beamId];
    }
  NS_FATAL_ERROR ("User Phy does not exist for beam " << beamId);
}

std::map<uint32_t, Ptr<SatPhy> >
SatGeoNetDevice::GetUserPhy ()
{
  return m_userPhy;
}

std::map<uint32_t, Ptr<SatPhy> >
SatGeoNetDevice::GetFeederPhy ()
{
  return m_feederPhy;
}

void
SatGeoNetDevice::AddUserMac (Ptr<SatMac> mac, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << mac << beamId);
  m_userMac.insert (std::pair<uint32_t, Ptr<SatMac> > (beamId, mac));
}

void
SatGeoNetDevice::AddFeederMac (Ptr<SatMac> mac, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << mac << beamId);
  m_feederMac.insert (std::pair<uint32_t, Ptr<SatMac> > (beamId, mac));
}

Ptr<SatMac>
SatGeoNetDevice::GetUserMac (uint32_t beamId)
{
  if (m_userMac.count(beamId))
    {
      return m_userMac[beamId];
    }
  NS_FATAL_ERROR ("User MAC does not exist for beam " << beamId);
}

Ptr<SatMac>
SatGeoNetDevice::GetFeederMac (uint32_t beamId)
{
  if (m_userMac.count(beamId))
    {
      return m_feederMac[beamId];
    }
  NS_FATAL_ERROR ("User MAC does not exist for beam " << beamId);
}

std::map<uint32_t, Ptr<SatMac> >
SatGeoNetDevice::GetUserMac ()
{
  return m_userMac;
}

std::map<uint32_t, Ptr<SatMac> >
SatGeoNetDevice::GetFeederMac ()
{
  return m_feederMac;
}

} // namespace ns3
