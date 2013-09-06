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
#include "ns3/string.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-interface.h"
#include "ns3/mobility-helper.h"
#include "../model/satellite-channel.h"
#include "../model/satellite-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-arp-cache.h"
#include "../model/satellite-mobility-model.h"
#include "../model/satellite-propagation-delay-model.h"
#include "satellite-beam-helper.h"


NS_LOG_COMPONENT_DEFINE ("SatBeamHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBeamHelper);

TypeId
SatBeamHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatBeamHelper")
      .SetParent<Object> ()
      .AddConstructor<SatBeamHelper>()
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
  // this default constructor should not be called...
  NS_ASSERT(false);
}

SatBeamHelper::SatBeamHelper (Ptr<Node> geoNode)
{
  // uncomment next code line, if attributes are needed already in construction phase.
  // E.g attributes set by object factory affecting object creation
  //ObjectBase::ConstructSelf(AttributeConstructionList ());

  m_channelFactory.SetTypeId ("ns3::SatChannel");

  // create needed low level satellite helpers
  m_geoHelper = CreateObject<SatGeoHelper>();
  m_gwHelper = CreateObject<SatGwHelper>();
  m_utHelper = CreateObject<SatUtHelper>();

  m_gwHelper->Initialize ();
  m_utHelper->Initialize ();

  m_geoNode = geoNode;
  m_geoHelper->Install(m_geoNode);

  m_ncc = CreateObject<SatNcc>();
}

void
SatBeamHelper::DoDispose()
{
  NS_LOG_FUNCTION (this);

  m_beam.clear();
  m_gwNode.clear();
  m_ulChannels.clear();
  m_flChannels.clear();
  m_beamFreqs.clear();
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
  NS_LOG_FUNCTION (this << network << mask << address);

  m_ipv4Helper.SetBase(network, mask, address);
}

Ptr<Node>
SatBeamHelper::Install (NodeContainer ut, Ptr<Node> gwNode, uint32_t gwId, uint32_t beamId, uint32_t ulFreqId, uint32_t flFreqId )
{
  // add beamId as key and gwId as value pair to beam map. In case it's there already, assertion failure is caused
  std::pair<std::map<uint32_t,uint32_t >::iterator, bool> beam = m_beam.insert(std::make_pair(beamId, gwId));
  NS_ASSERT(beam.second == true);

  // add gwId and flFreqId pair to GW link set. In case it's there already, assertion failure is caused
  std::pair<std::set<GwLink_t >::iterator, bool>  gw = m_gwLinks.insert(GwLink_t(gwId, flFreqId));
  NS_ASSERT(gw.second == true);

  // save frequency pair to map with beam ID
  FrequencyPair_t freqPair = FrequencyPair_t(ulFreqId, flFreqId);
  m_beamFreqs.insert(std::pair<uint32_t, FrequencyPair_t >(beamId, freqPair));

  // next it is found user link channels and if not found channels are created and saved to map
  ChannelPair_t userLink = GetChannelPair(m_ulChannels, ulFreqId, true);

  // next it is found feeder link channels and if not found channels are created nd saved to map
  ChannelPair_t feederLink = GetChannelPair(m_flChannels, flFreqId, false);

  NS_ASSERT(m_geoNode != NULL);

  // attach channels to geo satellite device
  m_geoHelper->AttachChannels( m_geoNode->GetDevice(0), feederLink.first, feederLink.second,
                               userLink.first, userLink.second, beamId );

  // store GW node
  bool storedOk = StoreGwNode(gwId, gwNode);
  NS_ASSERT( storedOk );

  //install GW
  Ptr<NetDevice> gwNd = m_gwHelper->Install(gwNode, beamId, feederLink.first, feederLink.second);
  Ipv4InterfaceContainer gwAddress = m_ipv4Helper.Assign(gwNd);

  // install UTs
  NetDeviceContainer utNd = m_utHelper->Install(ut, beamId, userLink.first, userLink.second);
  Ipv4InterfaceContainer utAddress = m_ipv4Helper.Assign(utNd);

  // set needed routings and fill ARP cache
  PopulateRoutings(ut, utNd, gwNode, gwNd, gwAddress.GetAddress(0), utAddress );

  // add beam to NCC
  m_ncc->AddBeam(beamId, MakeCallback(&NetDevice::Send, gwNd));

  // add UTs to NCC
  for ( NetDeviceContainer::Iterator i = utNd.Begin();  i != utNd.End(); i++ )
    {
      m_ncc->AddUt((*i)->GetAddress(), beamId);
    }

  m_ipv4Helper.NewNetwork();

  //save UT node pointers to multimap
  for ( NodeContainer::Iterator i = ut.Begin();  i != ut.End(); i++ )
    {
      m_utNode.insert(std::make_pair(beamId, *i));
    }

  return gwNode;
}

Ptr<Node>
SatBeamHelper::GetGwNode(uint32_t id)
{
  std::map<uint32_t, Ptr<Node> >::iterator gwIterator = m_gwNode.find(id);
  Ptr<Node> node = NULL;

  if ( gwIterator != m_gwNode.end())
    {
      node = gwIterator->second;
    }

  return node;
}

Ptr<Node>
SatBeamHelper::GetGeoSatNode()
{
  return m_geoNode;
}

NodeContainer
SatBeamHelper::GetGwNodes()
{
  NodeContainer gwNodes;

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_gwNode.begin(); i != m_gwNode.end(); i++)
    {
      gwNodes.Add(i->second);
    }

  return gwNodes;
}

NodeContainer
SatBeamHelper::GetUtNodes()
{
  NodeContainer utNodes;

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_utNode.begin(); i != m_utNode.end(); i++)
    {
      utNodes.Add(i->second);
    }

  return utNodes;
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
SatBeamHelper::GetUtPositionInfo(bool printMacAddress)
{
  std::ostringstream oss;

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_utNode.begin(); i != m_utNode.end (); i++)
    {
      Ptr<SatMobilityModel> model = i->second->GetObject<SatMobilityModel> ();
      GeoCoordinate pos = model->GetGeoPosition();

      if ( printMacAddress  )
        {
          Address devAddress;

          for ( uint32_t j = 0; j < i->second->GetNDevices(); j++)
            {
              Ptr<NetDevice> device = i->second->GetDevice(j);

              if ( device->GetInstanceTypeId().GetName() == "ns3::SatNetDevice")
                {
                  devAddress = device->GetAddress();
                }
            }

          oss << i->first << " " << devAddress << " " <<  i->second->GetId() << " "
              << pos.GetLongitude() << " " << pos.GetLatitude() << " " << pos.GetAltitude() << std::endl;
        }
      else
        {
          oss << i->first << " " <<  i->second->GetId() << " "
              << pos.GetLongitude() << " " << pos.GetLatitude() << " " << pos.GetAltitude() << std::endl;
        }

    }

  return oss.str();
}

std::string
SatBeamHelper::CreateBeamInfo()
{
  std::ostringstream oss;

  oss << std::endl << " -- Beam detailes --";

  for (std::map<uint32_t, uint32_t>::iterator i = m_beam.begin(); i != m_beam.end (); i++)
    {
      oss << std::endl << "Beam ID: " << (*i).first << " ";

      std::map<uint32_t, FrequencyPair_t >::iterator freqIds = m_beamFreqs.find((*i).first);

      if ( freqIds != m_beamFreqs.end())
        {
          oss << "user link frequency ID: " << (*freqIds).second.first << ", ";
          oss << "feeder link frequency ID: " << (*freqIds).second.second;
        }

      oss << ", GW ID: " << (*i).second;
    }

  oss << std::endl << std::endl << " -- GW positions --" << std::endl;

  oss.precision(8);
  oss.setf(std::ios::fixed, std::ios::floatfield);

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_gwNode.begin(); i != m_gwNode.end (); i++)
    {
      Ptr<SatMobilityModel> model = i->second->GetObject<SatMobilityModel> ();
      GeoCoordinate pos = model->GetGeoPosition();
      oss << "GW " << i->first << ": longitude=" << pos.GetLongitude()
          << ", latitude=" << pos.GetLatitude() << ", altitude=" << pos.GetAltitude() << std::endl;
    }

  oss << std::endl << " -- Geo Satellite position --" << std::endl;

  Ptr<SatMobilityModel> model = m_geoNode->GetObject<SatMobilityModel> ();
  GeoCoordinate pos = model->GetGeoPosition();
  oss << "longitude=" << pos.GetLongitude() << ", latitude=" << pos.GetLatitude() << ", altitude=" << pos.GetAltitude() << std::endl;

  return oss.str();
}

SatBeamHelper::ChannelPair_t
SatBeamHelper::GetChannelPair(std::map<uint32_t, ChannelPair_t > & chPairMap, uint32_t frequencyId, bool isUserLink)
{
  ChannelPair_t channelPair;
  std::map<uint32_t, ChannelPair_t >::iterator mapIterator = chPairMap.find(frequencyId);

  if ( mapIterator == chPairMap.end())
      {
        Ptr<SatChannel> forwardCh = m_channelFactory.Create<SatChannel> ();
        Ptr<SatChannel> returnCh = m_channelFactory.Create<SatChannel> ();

        if ( isUserLink )
          {
            forwardCh->SetChannelType(SatChannel::FORWARD_USER_CH);
            returnCh->SetChannelType(SatChannel::RETURN_USER_CH);
          }
        else
          {
            forwardCh->SetChannelType(SatChannel::FORWARD_FEEDER_CH);
            returnCh->SetChannelType(SatChannel::RETURN_FEEDER_CH);
          }
        /*
         * Average propagation delay between UT/GW and satellite in seconds
         * TODO: Change the propagation delay to be a parameter.
        */
        double pd = 0.13;
        Ptr<SatConstantPropagationDelayModel> pDelay = CreateObject<SatConstantPropagationDelayModel> (pd);
        forwardCh->SetPropagationDelayModel (pDelay);
        returnCh->SetPropagationDelayModel (pDelay);

        Ptr<SatFreeSpaceLoss> pFsl =  CreateObject<SatFreeSpaceLoss> ();
        forwardCh->SetFreeSpaceLoss(pFsl);
        returnCh->SetFreeSpaceLoss(pFsl);

        channelPair.first = forwardCh;
        channelPair.second = returnCh;

        chPairMap.insert(std::pair<uint32_t, ChannelPair_t >(frequencyId, channelPair));
      }
    else
      {
        channelPair = mapIterator->second;
      }

  return channelPair;
}

bool
SatBeamHelper::StoreGwNode(uint32_t id, Ptr<Node> node)
{
  bool storingSuccess = false;

  Ptr<Node> storedNode = GetGwNode(id);

  if ( storedNode != NULL ) // nGW node with id already stored
    {
      if ( storedNode == node ) // check that node is same
        {
          storingSuccess = true;
        }
    }
  else  // try to store if not stored
    {
      std::pair<std::map<uint32_t, Ptr<Node> >::iterator, bool> result = m_gwNode.insert(std::make_pair(id, node));
      storingSuccess = result.second;
    }

  return storingSuccess;
}

void
SatBeamHelper::PopulateRoutings(NodeContainer ut, NetDeviceContainer utNd, Ptr<Node> gw, Ptr<NetDevice> gwNd, Ipv4Address gwAddr, Ipv4InterfaceContainer utIfs)
{
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4L3Protocol> ipv4Gw = gw->GetObject<Ipv4L3Protocol> ();
  Ptr<Ipv4StaticRouting> srGw = ipv4RoutingHelper.GetStaticRouting (ipv4Gw);

  // Create an ARP entry of the default GW for the UTs in this beam
  Address macAddressGw = gwNd->GetAddress ();
  Ptr<SatArpCache> utArpCache = CreateObject<SatArpCache> ();
  utArpCache->Add (gwAddr, macAddressGw);
  NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, UT arp entry:  " << gwAddr << " - " << macAddressGw );

  // Add the ARP entries of all the UTs in this beam
  // - MAC address vs. IPv4 address
  Ptr<SatArpCache> gwArpCache = CreateObject<SatArpCache> ();
  for (uint32_t i = 0; i < utIfs.GetN (); ++i)
    {
      NS_ASSERT (utIfs.GetN() == utNd.GetN());
      Ptr<NetDevice> nd = utNd.Get (i);
      Ipv4Address ipv4Addr = utIfs.GetAddress (i);
      gwArpCache->Add (ipv4Addr, nd->GetAddress ());
      NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, GW arp entry:  " << ipv4Addr << " - " << nd->GetAddress ());
    }

  // Set the ARP cache to the proper GW IPv4Interface (the one for satellite
  // link). ARP cache contains the entries for all UTs within this spot-beam.
  ipv4Gw->GetInterface (gwNd->GetIfIndex ())->SetArpCache (gwArpCache);
  NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, Add ARP cache to GW: " << gw->GetId() );

  uint32_t utAddressIndex = 0;

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
              srUt->SetDefaultRoute (gwAddr, j);
              NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, UT default route: " << gwAddr);

              // Set the ARP cache (including the ARP entry for the default GW) to the UT
              ipv4Ut->GetInterface (j)->SetArpCache (utArpCache);
              NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, add the ARP cache to UT " << (*i)->GetId() );

            }
          else  // add other interface route to GW's Satellite interface
            {
              Ipv4Address address = ipv4Ut->GetAddress(j, 0).GetLocal();
              Ipv4Mask mask = ipv4Ut->GetAddress(j, 0).GetMask();

              srGw->AddNetworkRouteTo (address.CombineMask(mask), mask, utIfs.GetAddress(utAddressIndex) ,gwNd->GetIfIndex());
              NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, GW Network route:  " << address.CombineMask(mask) << ", " << mask << ", " << utIfs.GetAddress(utAddressIndex));
            }
        }

      utAddressIndex++;
    }
}

} // namespace ns3
