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
#include "ns3/enum.h"
#include "ns3/config.h"
#include "../model/satellite-channel.h"
#include "../model/satellite-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-arp-cache.h"
#include "../model/satellite-mobility-model.h"
#include "../model/satellite-propagation-delay-model.h"
#include "../model/satellite-antenna-gain-pattern-container.h"
#include "../model/satellite-packet-trace.h"
#include "satellite-beam-helper.h"
#include "ns3/satellite-fading-input-trace-container.h"
#include "ns3/satellite-fading-input-trace.h"
#include "ns3/singleton.h"
#include "ns3/satellite-id-mapper.h"

NS_LOG_COMPONENT_DEFINE ("SatBeamHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBeamHelper);

TypeId
SatBeamHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatBeamHelper")
      .SetParent<Object> ()
      .AddConstructor<SatBeamHelper>()
      .AddAttribute ("CarrierFrequencyConverter", "Callback to convert carrier id to generate frequency.",
                      CallbackValue (),
                      MakeCallbackAccessor (&SatBeamHelper::m_carrierFreqConverter),
                      MakeCallbackChecker ())
      .AddAttribute ("FadingModel",
                     "Fading model",
                      EnumValue (SatEnums::FADING_MARKOV),
                      MakeEnumAccessor (&SatBeamHelper::m_fadingModel),
                      MakeEnumChecker (SatEnums::FADING_OFF, "FadingOff",
                                       SatEnums::FADING_TRACE, "FadingTrace",
                                       SatEnums::FADING_MARKOV, "FadingMarkov"))
      .AddAttribute ("PrintDetailedInformationToCreationTraces",
                     "Print detailed information to creation traces",
                     BooleanValue (true),
                     MakeBooleanAccessor(&SatBeamHelper::m_printDetailedInformationToCreationTraces),
                     MakeBooleanChecker ())
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatBeamHelper::m_creation))
    ;
    return tid;
}

TypeId
SatBeamHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId();
}

SatBeamHelper::SatBeamHelper () :
    m_printDetailedInformationToCreationTraces (false),
    m_fadingModel ()
{
  NS_LOG_FUNCTION (this);

  // this default constructor should not be called...
  NS_ASSERT(false);
}

SatBeamHelper::SatBeamHelper (Ptr<Node> geoNode,
                              CarrierBandwidthConverter bandwidthConverterCb,
                              uint32_t rtnLinkCarrierCount,
                              uint32_t fwdLinkCarrierCount,
                              Ptr<SatSuperframeSeq> seq)
  : m_superframeSeq (seq),
    m_printDetailedInformationToCreationTraces (false),
    m_fadingModel (SatEnums::FADING_MARKOV)
{
  NS_LOG_FUNCTION (this << geoNode << rtnLinkCarrierCount << fwdLinkCarrierCount << seq);

  // uncomment next code line, if attributes are needed already in construction phase.
  // E.g attributes set by object factory affecting object creation
  ObjectBase::ConstructSelf(AttributeConstructionList ());

  m_channelFactory.SetTypeId ("ns3::SatChannel");

  // create needed low level satellite helpers
  m_geoHelper = CreateObject<SatGeoHelper> ( bandwidthConverterCb, rtnLinkCarrierCount, fwdLinkCarrierCount);
  m_gwHelper = CreateObject<SatGwHelper> ( bandwidthConverterCb, rtnLinkCarrierCount);
  m_utHelper = CreateObject<SatUtHelper> ( bandwidthConverterCb, fwdLinkCarrierCount, seq );

  // Two usage of link results is two-fold: on the other hand they are needed in the
  // packet reception for packet decoding, but on the other hand they are utilized in
  // transmission side in ACM for deciding the best MODCOD.
  //
  // DVB-RCS2 link results:
  // - Packet reception at the GW
  // - RTN link packet scheduling at the NCC
  // DVB-S2 link results:
  // - Packet reception at the UT
  // - FWD link packet scheduling at the GW
  //
  Ptr<SatLinkResultsDvbS2> linkResultsS2 = CreateObject<SatLinkResultsDvbS2> ();
  Ptr<SatLinkResultsDvbRcs2> linkResultsRcs2 = CreateObject<SatLinkResultsDvbRcs2> ();
  linkResultsS2->Initialize ();
  linkResultsRcs2->Initialize ();

  // DVB-S2 link results for packet decoding at the UT
  m_utHelper->Initialize (linkResultsS2);
  // DVB-RCS2 link results for packet decoding at the GW +
  // DVB-S2 link results for FWD link RRM
  m_gwHelper->Initialize (linkResultsRcs2, linkResultsS2);
  // DVB-RCS2 link results for RTN link waveform configurations
  m_superframeSeq->GetWaveformConf()->InitializeEbNoRequirements (linkResultsRcs2);

  m_geoNode = geoNode;
  m_geoHelper->Install (m_geoNode);

  m_ncc = CreateObject<SatNcc> ();

  switch (m_fadingModel)
    {
    case SatEnums::FADING_MARKOV:
      {
        /// create default Markov & Loo configurations
        m_markovConf = CreateObject<SatMarkovConf> ();
        break;
      }
    case SatEnums::FADING_OFF:
    case SatEnums::FADING_TRACE:
    default:
      {
        m_markovConf = NULL;
        break;
      }
    }
}

void
SatBeamHelper::DoDispose()
{
  NS_LOG_FUNCTION (this);

  m_beam.clear ();
  m_gwNode.clear ();
  m_ulChannels.clear ();
  m_flChannels.clear ();
  m_beamFreqs.clear ();
  m_markovConf = NULL;
  m_ncc = NULL;
  m_geoHelper = NULL;
  m_gwHelper = NULL;
  m_utHelper = NULL;
}

void 
SatBeamHelper::SetAntennaGainPatterns (Ptr<SatAntennaGainPatternContainer> antennaPatterns)
{
  NS_LOG_FUNCTION (this << antennaPatterns);

  m_antennaGainPatterns = antennaPatterns;
}

void
SatBeamHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1);
}

void
SatBeamHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1);

  m_channelFactory.Set (n1, v1);
}

void SatBeamHelper::SetBaseAddress ( const Ipv4Address network, const Ipv4Mask mask, const Ipv4Address address)
{
  NS_LOG_FUNCTION (this << network << mask << address);

  m_ipv4Helper.SetBase (network, mask, address);
}

Ptr<Node>
SatBeamHelper::Install (NodeContainer ut, Ptr<Node> gwNode, uint32_t gwId, uint32_t beamId, uint32_t ulFreqId, uint32_t flFreqId)
{
  NS_LOG_FUNCTION (this << gwNode << gwId << beamId << ulFreqId << flFreqId);

  // add beamId as key and gwId as value pair to beam map. In case it's there already, assertion failure is caused
  std::pair<std::map<uint32_t,uint32_t >::iterator, bool> beam = m_beam.insert (std::make_pair (beamId, gwId));
  NS_ASSERT (beam.second == true);

  // add gwId and flFreqId pair to GW link set. In case it's there already, assertion failure is caused
  std::pair<std::set<GwLink_t >::iterator, bool>  gw = m_gwLinks.insert (GwLink_t (gwId, flFreqId));
  NS_ASSERT (gw.second == true);

  // save frequency pair to map with beam ID
  FrequencyPair_t freqPair = FrequencyPair_t (ulFreqId, flFreqId);
  m_beamFreqs.insert(std::pair<uint32_t, FrequencyPair_t > (beamId, freqPair));

  // next it is found user link channels and if not found channels are created and saved to map
  ChannelPair_t userLink = GetChannelPair (m_ulChannels, ulFreqId, true);

  // next it is found feeder link channels and if not found channels are created nd saved to map
  ChannelPair_t feederLink = GetChannelPair (m_flChannels, flFreqId, false);

  NS_ASSERT (m_geoNode != NULL);

  // Get the position of the GW serving this beam, get the best beam based on antenna patterns
  // for this position, and set the antenna patterns to the feeder PHY objects via
  // AttachChannels method.
  GeoCoordinate gwPos = gwNode->GetObject<SatMobilityModel> ()->GetGeoPosition ();
  uint32_t feederBeamId = m_antennaGainPatterns->GetBestBeamId (gwPos);

  // attach channels to geo satellite device
  m_geoHelper->AttachChannels ( m_geoNode->GetDevice(0),
                                feederLink.first,
                                feederLink.second,
                                userLink.first,
                                userLink.second,
                                m_antennaGainPatterns->GetAntennaGainPattern (beamId),
                                m_antennaGainPatterns->GetAntennaGainPattern (feederBeamId),
                                beamId);

  // store GW node
  bool storedOk = StoreGwNode (gwId, gwNode);
  NS_ASSERT ( storedOk );

  // install fading container to GW
  if (m_fadingModel != SatEnums::FADING_OFF)
    {
      InstallFadingContainer (gwNode);
    }

  Ptr<SatMobilityModel> gwMobility = gwNode->GetObject<SatMobilityModel> ();
  NS_ASSERT (gwMobility != NULL);

  // enable timing advance in observers of the UTs
  for ( NodeContainer::Iterator i = ut.Begin ();  i != ut.End (); i++ )
    {
      // enable timing advance observing in nodes.

      Ptr<SatMobilityObserver> observer = (*i)->GetObject<SatMobilityObserver> ();
      NS_ASSERT (observer != NULL);

      observer->ObserveTimingAdvance (userLink.second->GetPropagationDelayModel(),
                                      feederLink.second->GetPropagationDelayModel(), gwMobility);

      if (m_fadingModel != SatEnums::FADING_OFF)
        {
          InstallFadingContainer (*i);
        }

      //save UT node pointer to multimap container
      m_utNode.insert (std::make_pair (beamId, *i) );
    }

  //install GW
  Ptr<NetDevice> gwNd = m_gwHelper->Install (gwNode,
                                             gwId,
                                             beamId,
                                             feederLink.first,
                                             feederLink.second,
                                             m_ncc);

  Ipv4InterfaceContainer gwAddress = m_ipv4Helper.Assign (gwNd);

  // add beam to NCC
  m_ncc->AddBeam (beamId, MakeCallback (&SatNetDevice::SendControl, DynamicCast<SatNetDevice>(gwNd)), m_superframeSeq );

  // install UTs
  NetDeviceContainer utNd = m_utHelper->Install (ut,
                                                 beamId,
                                                 userLink.first,
                                                 userLink.second,
                                                 DynamicCast<SatNetDevice>(gwNd),
                                                 m_ncc);

  Ipv4InterfaceContainer utAddress = m_ipv4Helper.Assign (utNd);

  // set needed routings and fill ARP cache
  PopulateRoutings (ut, utNd, gwNode, gwNd, gwAddress.GetAddress (0), utAddress );

  m_ipv4Helper.NewNetwork ();

  return gwNode;
}

Ptr<Node>
SatBeamHelper::GetGwNode (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);

  std::map<uint32_t, Ptr<Node> >::iterator gwIterator = m_gwNode.find (id);
  Ptr<Node> node = NULL;

  if ( gwIterator != m_gwNode.end () )
    {
      node = gwIterator->second;
    }

  return node;
}

Ptr<Node>
SatBeamHelper::GetGeoSatNode()
{
  NS_LOG_FUNCTION (this);

  return m_geoNode;
}

NodeContainer
SatBeamHelper::GetGwNodes()
{
  NS_LOG_FUNCTION (this);

  NodeContainer gwNodes;

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_gwNode.begin (); i != m_gwNode.end (); i++)
    {
      gwNodes.Add (i->second);
    }

  return gwNodes;
}

NodeContainer
SatBeamHelper::GetUtNodes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer utNodes;

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_utNode.begin (); i != m_utNode.end (); i++)
    {
      utNodes.Add (i->second);
    }

  return utNodes;
}

void
SatBeamHelper::EnableCreationTraces (Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect ("Creation", "SatBeamHelper", cb);
  m_geoHelper->EnableCreationTraces (stream, cb);
  m_gwHelper->EnableCreationTraces (stream, cb);
  m_utHelper->EnableCreationTraces (stream, cb);
}

void
SatBeamHelper::EnablePacketTrace ()
{
  /**
   * TODO: Enabling of the packet traces could be changed to use an attribute
   * instead of a simulation script function call. This would enable packet trace
   * enabling/disabling without recompilation.
   */

  // Create packet trace instance
  m_packetTrace = CreateObject<SatPacketTrace> ();

  /**
   * Connect the trace callbacks
   * By default the packet traces are connected to
   * - NetDevice
   * - LLC
   * - MAC
   * - PHY
   */

  /**
   * TODO: Currently the packet trace logs all entries updated by the protocol layers. Here
   * we could restrict the protocol layers from where the traced data are collected from.
   * This could be controlled by the user using attributes.
   */
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/PacketTrace", MakeCallback (&SatPacketTrace::AddTraceEntry, m_packetTrace));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatPhy/PacketTrace", MakeCallback (&SatPacketTrace::AddTraceEntry, m_packetTrace));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/UserPhy/*/PacketTrace", MakeCallback (&SatPacketTrace::AddTraceEntry, m_packetTrace));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/FeederPhy/*/PacketTrace", MakeCallback (&SatPacketTrace::AddTraceEntry, m_packetTrace));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatMac/PacketTrace", MakeCallback (&SatPacketTrace::AddTraceEntry, m_packetTrace));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatLlc/PacketTrace", MakeCallback (&SatPacketTrace::AddTraceEntry, m_packetTrace));
}

std::string
SatBeamHelper::GetBeamInfo ()
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;
  oss << "--- Beam Info, "  << "number of created beams: " << m_beam.size () << " ---" << std::endl;

  if ( m_beam.size () > 0 )
    {
      oss << CreateBeamInfo ();
    }

  return oss.str();
}

std::string
SatBeamHelper::GetUtInfo ()
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_utNode.begin (); i != m_utNode.end (); i++)
    {
      Ptr<SatMobilityModel> model = i->second->GetObject<SatMobilityModel> ();
      GeoCoordinate pos = model->GetGeoPosition ();

      Address devAddress;
      Ptr<Ipv4> ipv4 = i->second->GetObject<Ipv4> (); // Get Ipv4 instance of the node

      std::vector<Ipv4Address> IPAddressVector;
      std::vector<std::string> devNameVector;
      std::vector<Address> devAddressVector;

      for ( uint32_t j = 0; j < i->second->GetNDevices (); j++)
        {
          Ptr<NetDevice> device = i->second->GetDevice (j);

          if ( device->GetInstanceTypeId ().GetName () == "ns3::SatNetDevice")
            {
              devAddress = device->GetAddress ();
            }
          IPAddressVector.push_back (ipv4->GetAddress (j, 0).GetLocal()); // Get Ipv4InterfaceAddress of interface
          devNameVector.push_back (device->GetInstanceTypeId ().GetName ());
          devAddressVector.push_back (device->GetAddress ());
        }

      if ( m_printDetailedInformationToCreationTraces )
        {
          oss << i->first << " " << Singleton <SatIdMapper>::Get ()->GetUtIdWithMac (devAddress) << " "
              << pos.GetLatitude () << " " << pos.GetLongitude () << " " << pos.GetAltitude () << " ";

          for ( uint32_t j = 0; j < i->second->GetNDevices (); j++)
            {
              oss << devNameVector[j] << " " << devAddressVector[j] << " " << IPAddressVector[j] << " ";
            }

          oss << std::endl;
        }
      else
        {
          oss << i->first << " " << Singleton <SatIdMapper>::Get ()->GetUtIdWithMac (devAddress) << " "
        		  << pos.GetLatitude () << " " << pos.GetLongitude () << " " << pos.GetAltitude ()
        		  << std::endl;
        }

    }

  return oss.str ();
}

std::string
SatBeamHelper::CreateBeamInfo ()
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;

  oss << std::endl << " -- Beam details --";

  for (std::map<uint32_t, uint32_t>::iterator i = m_beam.begin (); i != m_beam.end (); i++)
    {
      oss << std::endl << "Beam ID: " << (*i).first << " ";

      std::map<uint32_t, FrequencyPair_t >::iterator freqIds = m_beamFreqs.find ((*i).first);

      if ( freqIds != m_beamFreqs.end ())
        {
          oss << "user link frequency ID: " << (*freqIds).second.first << ", ";
          oss << "feeder link frequency ID: " << (*freqIds).second.second;
        }

      oss << ", GW ID: " << (*i).second;
    }

  oss << std::endl << std::endl << " -- GW details --" << std::endl;

  oss.precision (8);
  oss.setf (std::ios::fixed, std::ios::floatfield);

  for (std::map<uint32_t, Ptr<Node> >::iterator i = m_gwNode.begin (); i != m_gwNode.end (); i++)
    {
      Ptr<SatMobilityModel> model = i->second->GetObject<SatMobilityModel> ();
      GeoCoordinate pos = model->GetGeoPosition ();

      Address devAddress;
      Ptr<Ipv4> ipv4 = i->second->GetObject<Ipv4> (); // Get Ipv4 instance of the node

      std::vector<Ipv4Address> IPAddressVector;
      std::vector<std::string> devNameVector;
      std::vector<Address> devAddressVector;

      for ( uint32_t j = 0; j < i->second->GetNDevices (); j++)
        {
          Ptr<NetDevice> device = i->second->GetDevice (j);

          if ( device->GetInstanceTypeId ().GetName () == "ns3::SatNetDevice")
            {
              devAddress = device->GetAddress ();
            }

          IPAddressVector.push_back (ipv4->GetAddress (j, 0).GetLocal()); // Get Ipv4InterfaceAddress of interface
          devNameVector.push_back (device->GetInstanceTypeId ().GetName ());
          devAddressVector.push_back (device->GetAddress ());
        }

      if ( m_printDetailedInformationToCreationTraces )
        {
          oss << "GW=" << i->first << " " << Singleton <SatIdMapper>::Get ()->GetUtIdWithMac (devAddress) << " "
              << " latitude=" << pos.GetLatitude ()
              << " longitude=" << pos.GetLongitude ()
              << " altitude=" << pos.GetAltitude () << " ";

          for ( uint32_t j = 0; j < i->second->GetNDevices (); j++)
            {
              oss << devNameVector[j] << " " << devAddressVector[j] << " " << IPAddressVector[j] << " ";
            }

          oss << std::endl;
        }
      else
        {
          oss << "GW=" << i->first << " "
              << Singleton <SatIdMapper>::Get ()->GetUtIdWithMac (devAddress) <<  " "
              << " latitude=" << pos.GetLatitude ()
              << " longitude=" << pos.GetLongitude ()
              << " altitude=" << pos.GetAltitude () << std::endl;
        }
    }

  oss << std::endl << " -- Geo Satellite position --" << std::endl;

  Ptr<SatMobilityModel> model = m_geoNode->GetObject<SatMobilityModel> ();
  GeoCoordinate pos = model->GetGeoPosition ();
  oss << "latitude=" << pos.GetLatitude () << ", longitude=" << pos.GetLongitude () << ", altitude=" << pos.GetAltitude () << std::endl;

  return oss.str();
}

SatBeamHelper::ChannelPair_t
SatBeamHelper::GetChannelPair (std::map<uint32_t, ChannelPair_t > & chPairMap, uint32_t frequencyId, bool isUserLink)
{
  NS_LOG_FUNCTION (this << frequencyId << isUserLink);

  ChannelPair_t channelPair;
  std::map<uint32_t, ChannelPair_t >::iterator mapIterator = chPairMap.find (frequencyId);

  if ( mapIterator == chPairMap.end ())
      {
        Ptr<SatChannel> forwardCh = m_channelFactory.Create<SatChannel> ();
        Ptr<SatChannel> returnCh = m_channelFactory.Create<SatChannel> ();

        if ( isUserLink )
          {
            forwardCh->SetChannelType (SatEnums::FORWARD_USER_CH);
            returnCh->SetChannelType (SatEnums::RETURN_USER_CH);
          }
        else
          {
            forwardCh->SetChannelType (SatEnums::FORWARD_FEEDER_CH);
            returnCh->SetChannelType (SatEnums::RETURN_FEEDER_CH);
          }

        forwardCh->SetFrequencyConverter (m_carrierFreqConverter);
        returnCh->SetFrequencyConverter (m_carrierFreqConverter);

        forwardCh->SetFrequencyId (frequencyId);
        returnCh->SetFrequencyId (frequencyId);
        /*
         * Average propagation delay between UT/GW and satellite in seconds
         * TODO: Change the propagation delay to be a parameter.
        */
        Ptr<SatConstantPropagationDelayModel> pDelay = CreateObject<SatConstantPropagationDelayModel> ();
        forwardCh->SetPropagationDelayModel (pDelay);
        returnCh->SetPropagationDelayModel (pDelay);

        Ptr<SatFreeSpaceLoss> pFsl =  CreateObject<SatFreeSpaceLoss> ();
        forwardCh->SetFreeSpaceLoss (pFsl);
        returnCh->SetFreeSpaceLoss (pFsl);

        channelPair.first = forwardCh;
        channelPair.second = returnCh;

        chPairMap.insert(std::pair<uint32_t, ChannelPair_t > (frequencyId, channelPair));
      }
    else
      {
        channelPair = mapIterator->second;
      }

  return channelPair;
}

bool
SatBeamHelper::StoreGwNode (uint32_t id, Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << id << node);

  bool storingSuccess = false;

  Ptr<Node> storedNode = GetGwNode (id);

  if ( storedNode != NULL ) // nGW node with id already stored
    {
      if ( storedNode == node ) // check that node is same
        {
          storingSuccess = true;
        }
    }
  else  // try to store if not stored
    {
      std::pair<std::map<uint32_t, Ptr<Node> >::iterator, bool> result = m_gwNode.insert (std::make_pair(id, node));
      storingSuccess = result.second;
    }

  return storingSuccess;
}

void
SatBeamHelper::PopulateRoutings (NodeContainer ut, NetDeviceContainer utNd, Ptr<Node> gw, Ptr<NetDevice> gwNd, Ipv4Address gwAddr, Ipv4InterfaceContainer utIfs)
{
  NS_LOG_FUNCTION (this << gw << gwNd << gwAddr);

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
      NS_ASSERT (utIfs.GetN () == utNd.GetN ());
      Ptr<NetDevice> nd = utNd.Get (i);
      Ipv4Address ipv4Addr = utIfs.GetAddress (i);
      gwArpCache->Add (ipv4Addr, nd->GetAddress ());
      NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, GW arp entry:  " << ipv4Addr << " - " << nd->GetAddress ());
    }

  // Set the ARP cache to the proper GW IPv4Interface (the one for satellite
  // link). ARP cache contains the entries for all UTs within this spot-beam.
  ipv4Gw->GetInterface (gwNd->GetIfIndex ())->SetArpCache (gwArpCache);
  NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, Add ARP cache to GW: " << gw->GetId () );

  uint32_t utAddressIndex = 0;

  for (NodeContainer::Iterator i = ut.Begin (); i != ut.End (); i++)
    {
      Ptr<Ipv4L3Protocol> ipv4Ut = (*i)->GetObject<Ipv4L3Protocol> ();

      uint32_t count = ipv4Ut->GetNInterfaces ();

      for (uint32_t j = 1; j < count; j++)
        {
          std::string devName = ipv4Ut->GetNetDevice (j)->GetInstanceTypeId ().GetName ();

          // If SatNetDevice interface, add default route to towards GW of the beam on UTs
          if ( devName == "ns3::SatNetDevice" )
            {
              Ptr<Ipv4StaticRouting> srUt = ipv4RoutingHelper.GetStaticRouting (ipv4Ut);
              srUt->SetDefaultRoute (gwAddr, j);
              NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, UT default route: " << gwAddr);

              // Set the ARP cache (including the ARP entry for the default GW) to the UT
              ipv4Ut->GetInterface (j)->SetArpCache (utArpCache);
              NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, add the ARP cache to UT " << (*i)->GetId () );

            }
          else  // add other interface route to GW's Satellite interface
            {
              Ipv4Address address = ipv4Ut->GetAddress (j, 0).GetLocal();
              Ipv4Mask mask = ipv4Ut->GetAddress (j, 0).GetMask();

              srGw->AddNetworkRouteTo (address.CombineMask (mask), mask, utIfs.GetAddress (utAddressIndex) ,gwNd->GetIfIndex ());
              NS_LOG_INFO ("SatBeamHelper::PopulateRoutings, GW Network route:  " << address.CombineMask(mask) <<
                           ", " << mask << ", " << utIfs.GetAddress (utAddressIndex));
            }
        }

      utAddressIndex++;
    }
}

Ptr<SatBaseFading>
SatBeamHelper::InstallFadingContainer (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  Ptr<SatBaseFading> fadingContainer = node->GetObject<SatBaseFading> ();

  if (fadingContainer == 0)
    {
      switch (m_fadingModel)
        {
        case SatEnums::FADING_MARKOV:
          {
            Ptr<SatMobilityObserver> observer = node->GetObject<SatMobilityObserver> ();
            NS_ASSERT(observer != NULL);

            SatBaseFading::ElevationCallback elevationCb = MakeCallback (&SatMobilityObserver::GetElevationAngle,
                                                                     observer);
            SatBaseFading::VelocityCallback velocityCb = MakeCallback (&SatMobilityObserver::GetVelocity,
                                                                   observer);

            /// create a Markov fading container based on default configuration
            fadingContainer = CreateObject<SatMarkovContainer> (m_markovConf,
                                                                elevationCb,
                                                                velocityCb);
            node->AggregateObject (fadingContainer);
            break;
          }
        case SatEnums::FADING_TRACE:
          {
            /// create a input trace fading container based on default configuration

            fadingContainer = CreateObject<SatFadingInputTrace> (Singleton<SatFadingInputTraceContainer>::Get ());

            node->AggregateObject (fadingContainer);
            break;
          }
        /// FADING_OFF option should never get to InstallFadingContainer
        case SatEnums::FADING_OFF:
        default:
          {
            NS_FATAL_ERROR ("SatBeamHelper::InstallFadingContainer - Incorrect fading model");
            break;
          }
        }
    }
  return fadingContainer;
}


} // namespace ns3
