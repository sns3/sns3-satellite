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
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-interface.h"
#include "ns3/propagation-delay-model.h"
#include "../model/satellite-channel.h"
#include "../model/satellite-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-arp-cache.h"
#include "satellite-beam-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatBeamHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBeamHelper);

TypeId
SatBeamHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatBeamHelper")
      .SetParent<Object> ()
      .AddConstructor<SatBeamHelper> ()
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatBeamHelper::m_creation))
    ;
    return tid;
}

TypeId
SatBeamHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatBeamHelper::SatBeamHelper ()
{
  // uncomment next code line, if attributes are needed already in construction phase.
  // E.g attributes set by object factory affecting object creation
  //ObjectBase::ConstructSelf(AttributeConstructionList ());

  m_channelFactory.SetTypeId ("ns3::SatChannel");

  // create needed low level satellite helpers
  m_geoHelper = CreateObject<SatGeoHelper>();
  m_gwHelper = CreateObject<SatGwHelper>();
  m_utHelper = CreateObject<SatUtHelper>();

  // create Geo Node and install net device on it already here because it is not scenario dependent
  m_geoNode = CreateObject<Node>();
  m_geoHelper->Install(m_geoNode);
}

void 
SatBeamHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{

}

void
SatBeamHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

void SatBeamHelper::SetBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, const Ipv4Address address)
{
  m_ipv4Helper.SetBase(network, mask, address);
}

Ptr<Node>
SatBeamHelper::Install (NodeContainer ut, uint16_t gwId, uint16_t beamId, uint16_t ulFreqId, uint16_t flFreqId )
{
  // add beamId and gwId pair to beam set. In case it's there already, assertion failure is caused
  std::pair<std::set<std::pair<uint16_t,uint16_t> >::iterator, bool> beam = m_beam.insert(std::pair<uint16_t,uint16_t>(beamId, gwId));
  NS_ASSERT(beam.second == true);

  // add gwId and flFreqId pair to GW link set. In case it's there already, assertion failure is caused
  std::pair<std::set<std::pair<uint16_t,uint16_t> >::iterator, bool>  gw = m_gwLink.insert(std::pair<uint16_t,uint16_t>(gwId, flFreqId));
  NS_ASSERT(gw.second == true);

  // next it is found GW node and if not found it is created
  std::map<uint16_t, Ptr<Node> >::iterator gwIterator = m_gwNode.find(gwId);
  Ptr<Node> gwNode;

  if ( gwIterator == m_gwNode.end())
    {
      gwNode = CreateObject<Node> ();
      m_gwNode.insert(std::pair<uint16_t,Ptr<Node> >(gwId, gwNode));
      m_gwNodeList.Add(gwNode);
      InternetStackHelper internet;
      internet.Install(gwNode);
    }
  else
    {
      gwNode = gwIterator->second;
    }

  // next it is found user link channels and if not found channels are created
  std::map<uint16_t, SatLink >::iterator ulIterator = m_ulChannels.find(ulFreqId);
  Ptr<SatChannel> ulForwardCh;
  Ptr<SatChannel> ulReturnCh;
  BeamFreqs freqPair;
  freqPair.first = ulFreqId;
  freqPair.second = flFreqId;
  m_beamLink.insert(std::pair<uint16_t, BeamFreqs >(beamId, freqPair));

  if ( ulIterator == m_ulChannels.end())
    {
      ulForwardCh = m_channelFactory.Create<SatChannel> ();
      ulReturnCh = m_channelFactory.Create<SatChannel> ();

      SatLink satLink;
      satLink.first = ulForwardCh;
      satLink.second = ulReturnCh;

      /*
       * Average propagation delay between UT/GW and satellite in seconds
       * TODO: Change the propagation delay to be a parameter.
      */
      double pd = 0.13;
      Ptr<ConstantPropagationDelayModel> pDelay = CreateObject<ConstantPropagationDelayModel> (pd);
      ulForwardCh->SetPropagationDelayModel (pDelay);
      ulReturnCh->SetPropagationDelayModel (pDelay);

      m_ulChannels.insert(std::pair<uint16_t, SatLink >(ulFreqId, satLink));
    }
  else
    {
      SatLink satLink = ulIterator->second;
      ulForwardCh = satLink.first;
      ulReturnCh =  satLink.second;
    }

  // next it is found feeder link channels and if not found channels are created
  std::map<uint16_t, SatLink >::iterator flIterator = m_flChannels.find(flFreqId);
  Ptr<SatChannel> flForwardCh;
  Ptr<SatChannel> flReturnCh;

  if ( flIterator == m_flChannels.end())
    {
      flForwardCh = m_channelFactory.Create<SatChannel> ();
      flReturnCh = m_channelFactory.Create<SatChannel> ();

      SatLink satLink;
      satLink.first = flForwardCh;
      satLink.second = flReturnCh;

      /*
       * Average propagation delay between UT/GW and satellite in seconds
       * TODO: Change the propagation delay to be a parameter.
      */
      double pd = 0.13;

      Ptr<ConstantPropagationDelayModel> pDelay = CreateObject<ConstantPropagationDelayModel> (pd);
      flForwardCh->SetPropagationDelayModel (pDelay);
      flReturnCh->SetPropagationDelayModel (pDelay);

      m_flChannels.insert(std::pair<uint16_t, SatLink >(flFreqId, satLink));
    }
  else
    {
      SatLink satLink = flIterator->second;
      flForwardCh = satLink.first;
      flReturnCh = satLink.second;
    }

  NS_ASSERT(m_geoNode != NULL);

  m_geoHelper->AttachChannels(m_geoNode->GetDevice(0), flForwardCh, flReturnCh, ulForwardCh, ulReturnCh, beamId );

  // next is created GW
  Ptr<NetDevice> gwNd = m_gwHelper->Install(gwNode, beamId, flForwardCh, flReturnCh);
  Ipv4InterfaceContainer gwAddress = m_ipv4Helper.Assign(gwNd);

  // finally is created UTs and set default route to them
  NetDeviceContainer utNd = m_utHelper->Install(ut, beamId, ulForwardCh, ulReturnCh);
  Ipv4InterfaceContainer utAddress = m_ipv4Helper.Assign(utNd);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4L3Protocol> ipv4Gw = gwNode->GetObject<Ipv4L3Protocol> ();
  Ptr<Ipv4StaticRouting> srGw = ipv4RoutingHelper.GetStaticRouting (ipv4Gw);

  // Create an ARP entry of the default GW for the UTs in this beam
  Ipv4Address ipv4AddressGw = gwAddress.GetAddress (0);
  Address macAddressGw = gwNd->GetAddress ();
  Ptr<SatArpCache> utArpCache = CreateObject<SatArpCache> ();
  utArpCache->Add (ipv4AddressGw, macAddressGw);
  NS_LOG_INFO ("SatBeamHelper::Install, UT arp entry:  " << ipv4AddressGw << " - " << macAddressGw );

  uint32_t utAddressIndex = 0;

  // Add the ARP entries of all the UTs in this beam
  // - MAC address vs. IPv4 address
  Ptr<SatArpCache> gwArpCache = CreateObject<SatArpCache> ();
  for (uint32_t i = 0; i < utAddress.GetN (); ++i)
    {
      NS_ASSERT (utAddress.GetN() == utNd.GetN());
      Ptr<NetDevice> nd = utNd.Get (i);
      Ipv4Address ipv4Addr = utAddress.GetAddress (i);
      gwArpCache->Add (ipv4Addr, nd->GetAddress ());
      NS_LOG_INFO ("SatBeamHelper::Install, GW arp entry:  " << ipv4Addr << " - " << nd->GetAddress ());
    }

  // Set the ARP cache to the proper GW IPv4Interface (the one for satellite
  // link). ARP cache contains the entries for all UTs within this spot-beam.
  ipv4Gw->GetInterface (gwNd->GetIfIndex ())->SetArpCache (gwArpCache);
  NS_LOG_INFO ("SatBeamHelper::Install, Add ARP cache to GW: " << gwNode->GetId() );

  for (NodeContainer::Iterator i = ut.Begin (); i != ut.End (); i++)
    {
      Ptr<Ipv4L3Protocol> ipv4Ut = (*i)->GetObject<Ipv4L3Protocol> ();

      uint32_t count = ipv4Ut->GetNInterfaces();

      for (uint32_t j = 1; j < count; j++)
        {
          std::string devName = ipv4Ut->GetNetDevice(j)->GetInstanceTypeId().GetName();

          // If SatNetDevice interface, add default route to towards GW of the beam on UTs
          if ( devName == "ns3::SatNetDevice" )
            {
              Ptr<Ipv4StaticRouting> srUt = ipv4RoutingHelper.GetStaticRouting (ipv4Ut);
              srUt->SetDefaultRoute (gwAddress.GetAddress(0), j);
              NS_LOG_INFO ("SatBeamHelper::Install, UT default route: " << gwAddress.GetAddress(0));

              // Set the ARP cache (including the ARP entry for the default GW) to the UT
              ipv4Ut->GetInterface (j)->SetArpCache (utArpCache);
              NS_LOG_INFO ("SatBeamHelper::Install, add the ARP cache to UT " << (*i)->GetId() );

            }
          else  // add other interface route to GW's Satellite interface
            {
              Ipv4Address address = ipv4Ut->GetAddress(j, 0).GetLocal();
              Ipv4Mask mask = ipv4Ut->GetAddress(j, 0).GetMask();

              srGw->AddNetworkRouteTo (address.CombineMask(mask), mask, utAddress.GetAddress(utAddressIndex) ,gwNd->GetIfIndex());
              NS_LOG_INFO ("SatBeamHelper::Install, GW Network route:  " << address.CombineMask(mask) << ", " << mask << ", " << utAddress.GetAddress(utAddressIndex));
            }
        }

      utAddressIndex++;
    }

  m_ipv4Helper.NewNetwork();

  return gwNode;
  }

NodeContainer
SatBeamHelper::GetGwNodes()
{
  return m_gwNodeList;
}


void
SatBeamHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  TraceConnect("Creation", "SatBeamHelper", cb);
  m_geoHelper->EnableCreationTraces(stream, cb);
  m_gwHelper->EnableCreationTraces(stream, cb);
  m_utHelper->EnableCreationTraces(stream, cb);
}

std::string
SatBeamHelper::GetBeamInfo()
{
  std::ostringstream oss;
  oss << "--- Beam Info, "  << "number of created beams: " << m_beam.size() << " ---" << std::endl;

  if ( m_beam.size() > 0 )
    {
      oss << CreateBeamInfo();
    }

  return oss.str();
}

std::string
SatBeamHelper::CreateBeamInfo()
{
  std::ostringstream oss;

  for (std::set<std::pair<uint16_t, uint16_t> >::iterator i = m_beam.begin(); i != m_beam.end (); i++)
    {
      oss << std::endl << "Beam ID: " << (*i).first << " ";

      std::map<uint16_t, BeamFreqs >::iterator freqIds = m_beamLink.find((*i).first);

      if ( freqIds != m_beamLink.end())
        {
          oss << "user link frequency ID: " << (*freqIds).second.first << ", ";
          oss << "feeder link frequency ID: " << (*freqIds).second.second;
        }
      oss << ", GWs (IDs): ";

      for (std::set<std::pair<uint16_t, uint16_t> >::iterator j = m_beam.begin(); j != m_beam.end (); j++)
        {
            if ( (*i).first == (*j).first )
              {
                 oss << (*j).second << " ";
              }
        }
    }

  return oss.str();
}

} // namespace ns3
