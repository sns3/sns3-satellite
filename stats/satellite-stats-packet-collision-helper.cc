/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/object-vector.h>
#include <ns3/callback.h>

#include <ns3/node-container.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-rx-carrier.h>

#include <ns3/satellite-helper.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/singleton.h>

#include <ns3/data-collection-object.h>
#include <ns3/scalar-collector.h>
#include <ns3/interval-rate-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/magister-gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-packet-collision-helper.h"


NS_LOG_COMPONENT_DEFINE ("SatStatsPacketCollisionHelper");


namespace ns3 {

// BASE CLASS /////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsPacketCollisionHelper);

SatStatsPacketCollisionHelper::SatStatsPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsPacketCollisionHelper::~SatStatsPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsPacketCollisionHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsPacketCollisionHelper::SetTraceSourceName (std::string traceSourceName)
{
  NS_LOG_FUNCTION (this << traceSourceName);
  m_traceSourceName = traceSourceName;
}


std::string
SatStatsPacketCollisionHelper::GetTraceSourceName () const
{
  return m_traceSourceName;
}


void
SatStatsPacketCollisionHelper::CollisionRxCallback (uint32_t nPackets,
                                                    const Address & from,
                                                    bool isCollided)
{
  NS_LOG_FUNCTION (this << nPackets << from << isCollided);

  if (from.IsInvalid ())
    {
      NS_LOG_WARN (this << " discarding " << nPackets << " packets"
                        << " from statistics collection because of"
                        << " invalid sender address");
    }
  else
    {
      // Determine the identifier associated with the sender address.
      std::map<const Address, uint32_t>::const_iterator it = m_identifierMap.find (from);

      if (it == m_identifierMap.end ())
        {
          NS_LOG_WARN (this << " discarding " << nPackets << " packets"
                            << " from statistics collection because of"
                            << " unknown sender address " << from);
        }
      else
        {
          // Find the first-level collector with the right identifier.
          Ptr<DataCollectionObject> collector = m_terminalCollectors.Get (it->second);
          NS_ASSERT_MSG (collector != 0,
                         "Unable to find collector with identifier " << it->second);

          switch (GetOutputType ())
            {
            case SatStatsHelper::OUTPUT_SCALAR_FILE:
            case SatStatsHelper::OUTPUT_SCALAR_PLOT:
              {
                Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkBoolean (false, isCollided);
                break;
              }

            case SatStatsHelper::OUTPUT_SCATTER_FILE:
            case SatStatsHelper::OUTPUT_SCATTER_PLOT:
              {
                Ptr<IntervalRateCollector> c = collector->GetObject<IntervalRateCollector> ();
                NS_ASSERT (c != 0);
                c->TraceSinkBoolean (false, isCollided);
                break;
              }

            default:
              NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
              break;

            } // end of `switch (GetOutputType ())`

        } // end of else of `if (it == m_identifierMap.end ())`

    } // end of else of `if (from.IsInvalid ())`

} // end of `void CollisionRxCallback (uint32_t, const Address &, bool);`


void
SatStatsPacketCollisionHelper::SaveAddressAndIdentifier (Ptr<Node> utNode)
{
  NS_LOG_FUNCTION (this << utNode->GetId ());

  const SatIdMapper * satIdMapper = Singleton<SatIdMapper>::Get ();
  const Address addr = satIdMapper->GetUtMacWithNode (utNode);

  if (addr.IsInvalid ())
    {
      NS_LOG_WARN (this << " Node " << utNode->GetId ()
                        << " is not a valid UT");
    }
  else
    {
      const uint32_t identifier = GetIdentifierForUt (utNode);
      m_identifierMap[addr] = identifier;
      NS_LOG_INFO (this << " associated address " << addr
                        << " with identifier " << identifier);

    }
}

// BASE CLASS FEEDER /////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFeederPacketCollisionHelper);

SatStatsFeederPacketCollisionHelper::SatStatsFeederPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFeederPacketCollisionHelper::~SatStatsFeederPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFeederPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFeederPacketCollisionHelper")
    .SetParent<SatStatsPacketCollisionHelper> ()
  ;
  return tid;
}


void
SatStatsFeederPacketCollisionHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_NONE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCALAR_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "MultiFileMode", BooleanValue (false),
                                         "EnableContextPrinting", BooleanValue (true),
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("collision_rate")));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write1d);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetTimeHeading ("collision_rate")));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (IntervalRateCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);
        m_terminalCollectors.ConnectToAggregator ("OutputString",
                                                  m_aggregator,
                                                  &MultiFileAggregator::AddContextHeading);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      /// \todo Add support for boxes in Gnuplot.
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                         "OutputPath", StringValue (GetOutputPath ()),
                                         "OutputFileName", StringValue (GetName ()));
        Ptr<MagisterGnuplotAggregator> plotAggregator
          = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
        NS_ASSERT (plotAggregator != 0);
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Packet collision rate");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (IntervalRateCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MagisterGnuplotAggregator::Write2d);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    default:
      NS_FATAL_ERROR ("SatStatsUserPacketCollisionHelper - Invalid output type");
      break;
    }

  // Create a map of UT addresses and identifiers.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at GW nodes.

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
  Callback<void, uint32_t, const Address &, bool> callback
    = MakeCallback (&SatStatsFeederPacketCollisionHelper::CollisionRxCallback,
                    this);

  for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
    {
      NetDeviceContainer devs = GetGwSatNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatNetDevice> satDev = (*itDev)->GetObject<SatNetDevice> ();
          NS_ASSERT (satDev != 0);
          Ptr<SatPhy> satPhy = satDev->GetPhy ();
          NS_ASSERT (satPhy != 0);
          Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
          NS_ASSERT (satPhyRx != 0);
          ObjectVectorValue carriers;
          satPhyRx->GetAttribute ("RxCarrierList", carriers);
          NS_LOG_DEBUG (this << " Node ID " << (*it)->GetId ()
                             << " device #" << (*itDev)->GetIfIndex ()
                             << " has " << carriers.GetN () << " RX carriers");

          for (ObjectVectorValue::Iterator itCarrier = carriers.Begin ();
               itCarrier != carriers.End (); ++itCarrier)
            {
              SatPhyRxCarrier::CarrierType ct = DynamicCast<SatPhyRxCarrier> (itCarrier->second)->GetCarrierType ();
              if (ct != GetValidCarrierType ())
                {
                  continue;
                }

              const bool ret = itCarrier->second->TraceConnectWithoutContext (GetTraceSourceName (), callback);
              if (ret)
                {
                  NS_LOG_INFO (this << " successfully connected with node ID "
                                    << (*it)->GetId ()
                                    << " device #" << (*itDev)->GetIfIndex ()
                                    << " RX carrier #" << itCarrier->first);
                }
              else
                {
                  NS_FATAL_ERROR ("Error connecting to "
                                  << GetTraceSourceName () << " trace source"
                                  << " of SatPhyRxCarrier"
                                  << " at node ID " << (*it)->GetId ()
                                  << " device #" << (*itDev)->GetIfIndex ()
                                  << " RX carrier #" << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = gws)`

} // end of `void DoInstall ();`

// BASE CLASS USER /////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsUserPacketCollisionHelper);

SatStatsUserPacketCollisionHelper::SatStatsUserPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsUserPacketCollisionHelper::~SatStatsUserPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsUserPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsUserPacketCollisionHelper")
    .SetParent<SatStatsPacketCollisionHelper> ()
  ;
  return tid;
}


void
SatStatsUserPacketCollisionHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_NONE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCALAR_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "MultiFileMode", BooleanValue (false),
                                         "EnableContextPrinting", BooleanValue (true),
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("collision_rate")));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::ScalarCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (ScalarCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("Output",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write1d);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetTimeHeading ("collision_rate")));

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (IntervalRateCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MultiFileAggregator::Write2d);
        m_terminalCollectors.ConnectToAggregator ("OutputString",
                                                  m_aggregator,
                                                  &MultiFileAggregator::AddContextHeading);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      /// \todo Add support for boxes in Gnuplot.
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                         "OutputPath", StringValue (GetOutputPath ()),
                                         "OutputFileName", StringValue (GetName ()));
        Ptr<MagisterGnuplotAggregator> plotAggregator
          = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
        NS_ASSERT (plotAggregator != 0);
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("Time (in seconds)",
                                   "Packet collision rate");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

        // Setup collectors.
        m_terminalCollectors.SetType ("ns3::IntervalRateCollector");
        m_terminalCollectors.SetAttribute ("InputDataType",
                                           EnumValue (IntervalRateCollector::INPUT_DATA_TYPE_BOOLEAN));
        m_terminalCollectors.SetAttribute ("OutputType",
                                           EnumValue (IntervalRateCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE));
        CreateCollectorPerIdentifier (m_terminalCollectors);
        for (CollectorMap::Iterator it = m_terminalCollectors.Begin ();
             it != m_terminalCollectors.End (); ++it)
          {
            const std::string context = it->second->GetName ();
            plotAggregator->Add2dDataset (context, context);
          }
        m_terminalCollectors.ConnectToAggregator ("OutputWithTime",
                                                  m_aggregator,
                                                  &MagisterGnuplotAggregator::Write2d);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    default:
      NS_FATAL_ERROR ("SatStatsFeederPacketCollisionHelper - Invalid output type");
      break;
    }

  // Create a map of UT addresses and identifiers.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      SaveAddressAndIdentifier (*it);
    }

  // Connect to trace sources at SAT nodes.

  NodeContainer sats = NodeContainer (GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ());
  Callback<void, uint32_t, const Address &, bool> callback
    = MakeCallback (&SatStatsUserPacketCollisionHelper::CollisionRxCallback,
                    this);

  for (NodeContainer::Iterator it = sats.Begin (); it != sats.End (); ++it)
    {
      NetDeviceContainer devs = GetSatSatGeoNetDevice (*it);

      for (NetDeviceContainer::Iterator itDev = devs.Begin ();
           itDev != devs.End (); ++itDev)
        {
          Ptr<SatPhy> satPhy;
          Ptr<SatGeoNetDevice> satGeoDev = (*itDev)->GetObject<SatGeoNetDevice> ();
          NS_ASSERT (satGeoDev != 0);
          std::map<uint32_t, Ptr<SatPhy> > satGeoUserPhys = satGeoDev->GetUserPhy ();
          for (std::map<uint32_t, Ptr<SatPhy>>::iterator itPhy = satGeoUserPhys.begin (); itPhy != satGeoUserPhys.end (); ++itPhy)
            {
              satPhy = itPhy->second;
              NS_ASSERT (satPhy != 0);
              Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
              NS_ASSERT (satPhyRx != 0);

              ObjectVectorValue carriers;
              satPhyRx->GetAttribute ("RxCarrierList", carriers);
              NS_LOG_DEBUG (this << " Node ID " << (*it)->GetId ()
                                 << " device #" << (*itDev)->GetIfIndex ()
                                 << " has " << carriers.GetN () << " RX carriers");

              for (ObjectVectorValue::Iterator itCarrier = carriers.Begin ();
                   itCarrier != carriers.End (); ++itCarrier)
                {
                  SatPhyRxCarrier::CarrierType ct = DynamicCast<SatPhyRxCarrier> (itCarrier->second)->GetCarrierType ();
                  if (ct != GetValidCarrierType ())
                    {
                      continue;
                    }

                  const bool ret = itCarrier->second->TraceConnectWithoutContext (GetTraceSourceName (), callback);
                  if (ret)
                    {
                      NS_LOG_INFO (this << " successfully connected with node ID "
                                        << (*it)->GetId ()
                                        << " device #" << (*itDev)->GetIfIndex ()
                                        << " RX carrier #" << itCarrier->first);
                    }
                  else
                    {
                      NS_FATAL_ERROR ("Error connecting to "
                                      << GetTraceSourceName () << " trace source"
                                      << " of SatPhyRxCarrier"
                                      << " at node ID " << (*it)->GetId ()
                                      << " device #" << (*itDev)->GetIfIndex ()
                                      << " RX carrier #" << itCarrier->first);
                    }

                } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`
            }
        } // end of `for (NetDeviceContainer::Iterator itDev = devs)`

    } // end of `for (NodeContainer::Iterator it = sats)`

} // end of `void DoInstall ();`


// SLOTTED ALOHA FEEDER //////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFeederSlottedAlohaPacketCollisionHelper);

SatStatsFeederSlottedAlohaPacketCollisionHelper::SatStatsFeederSlottedAlohaPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsFeederPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetTraceSourceName ("SlottedAlohaRxCollision");
  SetValidCarrierType (SatPhyRxCarrier::RA_SLOTTED_ALOHA);
}


SatStatsFeederSlottedAlohaPacketCollisionHelper::~SatStatsFeederSlottedAlohaPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFeederSlottedAlohaPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFeederSlottedAlohaPacketCollisionHelper")
    .SetParent<SatStatsFeederPacketCollisionHelper> ()
  ;
  return tid;
}


// CRDSA FEEDER //////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFeederCrdsaPacketCollisionHelper);

SatStatsFeederCrdsaPacketCollisionHelper::SatStatsFeederCrdsaPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsFeederPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetTraceSourceName ("CrdsaReplicaRx");
  SetValidCarrierType (SatPhyRxCarrier::RA_CRDSA);
}


SatStatsFeederCrdsaPacketCollisionHelper::~SatStatsFeederCrdsaPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFeederCrdsaPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFeederCrdsaPacketCollisionHelper")
    .SetParent<SatStatsFeederPacketCollisionHelper> ()
  ;
  return tid;
}


// E-SSA FEEDER //////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFeederEssaPacketCollisionHelper);

SatStatsFeederEssaPacketCollisionHelper::SatStatsFeederEssaPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsFeederPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetTraceSourceName ("EssaRxCollision");
  SetValidCarrierType (SatPhyRxCarrier::RA_ESSA);
}


SatStatsFeederEssaPacketCollisionHelper::~SatStatsFeederEssaPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFeederEssaPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFeederEssaPacketCollisionHelper")
    .SetParent<SatStatsFeederPacketCollisionHelper> ()
  ;
  return tid;
}


// SLOTTED ALOHA USER //////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsUserSlottedAlohaPacketCollisionHelper);

SatStatsUserSlottedAlohaPacketCollisionHelper::SatStatsUserSlottedAlohaPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsUserPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetTraceSourceName ("SlottedAlohaRxCollision");
  SetValidCarrierType (SatPhyRxCarrier::RA_SLOTTED_ALOHA);
}


SatStatsUserSlottedAlohaPacketCollisionHelper::~SatStatsUserSlottedAlohaPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsUserSlottedAlohaPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsUserSlottedAlohaPacketCollisionHelper")
    .SetParent<SatStatsUserPacketCollisionHelper> ()
  ;
  return tid;
}


// CRDSA USER //////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsUserCrdsaPacketCollisionHelper);

SatStatsUserCrdsaPacketCollisionHelper::SatStatsUserCrdsaPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsUserPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetTraceSourceName ("CrdsaReplicaRx");
  SetValidCarrierType (SatPhyRxCarrier::RA_CRDSA);
}


SatStatsUserCrdsaPacketCollisionHelper::~SatStatsUserCrdsaPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsUserCrdsaPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsUserCrdsaPacketCollisionHelper")
    .SetParent<SatStatsUserPacketCollisionHelper> ()
  ;
  return tid;
}


// E-SSA USER //////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsUserEssaPacketCollisionHelper);

SatStatsUserEssaPacketCollisionHelper::SatStatsUserEssaPacketCollisionHelper (Ptr<const SatHelper> satHelper)
  : SatStatsUserPacketCollisionHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
  SetTraceSourceName ("EssaRxCollision");
  SetValidCarrierType (SatPhyRxCarrier::RA_ESSA);
}


SatStatsUserEssaPacketCollisionHelper::~SatStatsUserEssaPacketCollisionHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsUserEssaPacketCollisionHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsUserEssaPacketCollisionHelper")
    .SetParent<SatStatsUserPacketCollisionHelper> ()
  ;
  return tid;
}


} // end of namespace ns3
