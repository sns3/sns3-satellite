/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
#include <ns3/object-map.h>
#include <ns3/object-vector.h>

#include <ns3/node.h>
#include <ns3/satellite-geo-net-device.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/satellite-helper.h>

#include <ns3/data-collection-object.h>
#include <ns3/unit-conversion-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/scalar-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <ns3/magister-gnuplot-aggregator.h>

#include <sstream>
#include "satellite-stats-link-sinr-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsLinkSinrHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsLinkSinrHelper);

SatStatsLinkSinrHelper::SatStatsLinkSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
    m_traceSinkCallback (MakeCallback (&SatStatsLinkSinrHelper::SinrCallback, this))
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsLinkSinrHelper::~SatStatsLinkSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsLinkSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsLinkSinrHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsLinkSinrHelper::SinrCallback (double sinrDb)
{
  NS_LOG_FUNCTION (this << sinrDb);

  switch (GetOutputType ())
    {
    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      {
        Ptr<ScalarCollector> c = m_collector->GetObject<ScalarCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkDouble (0.0, sinrDb);
        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      {
        Ptr<UnitConversionCollector> c = m_collector->GetObject<UnitConversionCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkDouble (0.0, sinrDb);
        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      {
        Ptr<DistributionCollector> c = m_collector->GetObject<DistributionCollector> ();
        NS_ASSERT (c != 0);
        c->TraceSinkDouble (0.0, sinrDb);
        break;
      }

    default:
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ()) << " is not a valid output type for this statistics.");
      break;

    } // end of `switch (GetOutputType ())`

} // end of `void SinrCallback (double);`


Callback<void, double>
SatStatsLinkSinrHelper::GetTraceSinkCallback () const
{
  return m_traceSinkCallback;
}


void
SatStatsLinkSinrHelper::DoInstall ()
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
                                         "GeneralHeading", StringValue (GetIdentifierHeading ("sinr_db")));
        Ptr<MultiFileAggregator> aggregator = m_aggregator->GetObject<MultiFileAggregator> ();

        // Setup collector.
        Ptr<ScalarCollector> collector = CreateObject<ScalarCollector> ();
        collector->SetName ("0");
        collector->SetInputDataType (ScalarCollector::INPUT_DATA_TYPE_DOUBLE);
        collector->SetOutputType (ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE);
        collector->TraceConnect ("Output", "0",
                                 MakeCallback (&MultiFileAggregator::Write1d,
                                               aggregator));
        m_collector = collector->GetObject<DataCollectionObject> ();

        break;
      }

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetTimeHeading ("sinr_db")));
        Ptr<MultiFileAggregator> aggregator = m_aggregator->GetObject<MultiFileAggregator> ();

        // Setup collector.
        Ptr<UnitConversionCollector> collector = CreateObject<UnitConversionCollector> ();
        collector->SetName ("0");
        collector->SetConversionType (UnitConversionCollector::TRANSPARENT);
        collector->TraceConnect ("OutputTimeValue", "0",
                                 MakeCallback (&MultiFileAggregator::Write2d,
                                               aggregator));
        m_collector = collector->GetObject<DataCollectionObject> ();

        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_FILE:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                         "OutputFileName", StringValue (GetOutputFileName ()),
                                         "GeneralHeading", StringValue (GetDistributionHeading ("sinr_db")));
        Ptr<MultiFileAggregator> aggregator = m_aggregator->GetObject<MultiFileAggregator> ();

        // Setup collector.
        Ptr<DistributionCollector> collector = CreateObject<DistributionCollector> ();
        collector->SetName ("0");
        if (GetOutputType () == SatStatsHelper::OUTPUT_HISTOGRAM_FILE)
          {
            collector->SetOutputType (DistributionCollector::OUTPUT_TYPE_HISTOGRAM);
          }
        else if (GetOutputType () == SatStatsHelper::OUTPUT_PDF_FILE)
          {
            collector->SetOutputType (DistributionCollector::OUTPUT_TYPE_PROBABILITY);
          }
        else if (GetOutputType () == SatStatsHelper::OUTPUT_CDF_FILE)
          {
            collector->SetOutputType (DistributionCollector::OUTPUT_TYPE_CUMULATIVE);
          }
        collector->TraceConnect ("Output", "0",
                                 MakeCallback (&MultiFileAggregator::Write2d,
                                               aggregator));
        collector->TraceConnect ("OutputString", "0",
                                 MakeCallback (&MultiFileAggregator::AddContextHeading,
                                               aggregator));
        collector->TraceConnect ("Warning", "0",
                                 MakeCallback (&MultiFileAggregator::EnableContextWarning,
                                               aggregator));
        m_collector = collector->GetObject<DataCollectionObject> ();

        break;
      }

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
                                   "SINR (in dB)");
        plotAggregator->Add2dDataset ("0", "0");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

        // Setup collector.
        Ptr<UnitConversionCollector> collector = CreateObject<UnitConversionCollector> ();
        collector->SetName ("0");
        collector->SetConversionType (UnitConversionCollector::TRANSPARENT);
        collector->TraceConnect ("OutputTimeValue", "0",
                                 MakeCallback (&MagisterGnuplotAggregator::Write2d,
                                               plotAggregator));
        m_collector = collector->GetObject<DataCollectionObject> ();

        break;
      }

    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      {
        // Setup aggregator.
        m_aggregator = CreateAggregator ("ns3::MagisterGnuplotAggregator",
                                         "OutputPath", StringValue (GetOutputPath ()),
                                         "OutputFileName", StringValue (GetName ()));
        Ptr<MagisterGnuplotAggregator> plotAggregator
          = m_aggregator->GetObject<MagisterGnuplotAggregator> ();
        NS_ASSERT (plotAggregator != 0);
        //plot->SetTitle ("");
        plotAggregator->SetLegend ("SINR (in dB)",
                                   "Frequency");
        plotAggregator->Add2dDataset ("0", "0");
        plotAggregator->Set2dDatasetDefaultStyle (Gnuplot2dDataset::LINES);

        // Setup collector.
        Ptr<DistributionCollector> collector = CreateObject<DistributionCollector> ();
        collector->SetName ("0");
        if (GetOutputType () == SatStatsHelper::OUTPUT_HISTOGRAM_PLOT)
          {
            collector->SetOutputType (DistributionCollector::OUTPUT_TYPE_HISTOGRAM);
          }
        else if (GetOutputType () == SatStatsHelper::OUTPUT_PDF_PLOT)
          {
            collector->SetOutputType (DistributionCollector::OUTPUT_TYPE_PROBABILITY);
          }
        else if (GetOutputType () == SatStatsHelper::OUTPUT_CDF_PLOT)
          {
            collector->SetOutputType (DistributionCollector::OUTPUT_TYPE_CUMULATIVE);
          }
        collector->TraceConnect ("Output", "0",
                                 MakeCallback (&MagisterGnuplotAggregator::Write2d,
                                               plotAggregator));
        m_collector = collector->GetObject<DataCollectionObject> ();

        break;
      }

    default:
      NS_FATAL_ERROR ("SatStatsLinkSinrHelper - Invalid output type");
      break;
    }

  // Setup probes and connect them to the collectors.
  InstallProbes ();

} // end of `void DoInstall ();`


void
SatStatsLinkSinrHelper::InstallProbes ()
{
  // The method below is supposed to be implemented by the child class.
  DoInstallProbes ();
}


// FORWARD FEEDER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdFeederLinkSinrHelper);

SatStatsFwdFeederLinkSinrHelper::SatStatsFwdFeederLinkSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkSinrHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdFeederLinkSinrHelper::~SatStatsFwdFeederLinkSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdFeederLinkSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdFeederLinkSinrHelper")
    .SetParent<SatStatsLinkSinrHelper> ()
  ;
  return tid;
}


void
SatStatsFwdFeederLinkSinrHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Node> geoSat = GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ();
  NS_ASSERT (geoSat->GetNDevices () == 1);
  Ptr<NetDevice> dev = geoSat->GetDevice (0);
  Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
  NS_ASSERT (satGeoDev != 0);
  ObjectMapValue phy;
  satGeoDev->GetAttribute ("FeederPhy", phy);
  NS_LOG_DEBUG (this << " GeoSat Node ID " << geoSat->GetId ()
                     << " device #" << dev->GetIfIndex ()
                     << " has " << phy.GetN () << " PHY instance(s)");

  for (ObjectMapValue::Iterator itPhy = phy.Begin ();
       itPhy != phy.End (); ++itPhy)
    {
      Ptr<SatPhy> satPhy = itPhy->second->GetObject<SatPhy> ();
      NS_ASSERT (satPhy != 0);
      Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
      NS_ASSERT (satPhyRx != 0);
      ObjectVectorValue carriers;
      satPhyRx->GetAttribute ("RxCarrierList", carriers);
      NS_LOG_DEBUG (this << " PHY #" << itPhy->first
                         << " has " << carriers.GetN () << " RX carrier(s)");

      for (ObjectVectorValue::Iterator itCarrier = carriers.Begin ();
           itCarrier != carriers.End (); ++itCarrier)
        {
          //NS_ASSERT (itCarrier->second->m_channelType == SatEnums::FORWARD_FEEDER_CH)
          if (!itCarrier->second->TraceConnectWithoutContext ("LinkSinr",
                                                              GetTraceSinkCallback ()))
            {
              NS_FATAL_ERROR ("Error connecting to LinkSinr trace source"
                              << " of SatPhyRxCarrier"
                              << " at GeoSat node ID " << geoSat->GetId ()
                              << " device #" << dev->GetIfIndex ()
                              << " PHY #" << itPhy->first
                              << " RX carrier #" << itCarrier->first);
            }

        } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

    } // end of `for (ObjectMapValue::Iterator itPhy = phys)`

} // end of `void DoInstallProbes ();`


// FORWARD USER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsFwdUserLinkSinrHelper);

SatStatsFwdUserLinkSinrHelper::SatStatsFwdUserLinkSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkSinrHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFwdUserLinkSinrHelper::~SatStatsFwdUserLinkSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFwdUserLinkSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFwdUserLinkSinrHelper")
    .SetParent<SatStatsLinkSinrHelper> ()
  ;
  return tid;
}


void
SatStatsFwdUserLinkSinrHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      //const int32_t utId = GetUtId (*it);
      //NS_ASSERT_MSG (utId > 0,
      //               "Node " << (*it)->GetId () << " is not a valid UT");
      //const uint32_t identifier = GetIdentifierForUt (*it);
      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatPhy> satPhy = satDev->GetPhy ();
      NS_ASSERT (satPhy != 0);
      Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
      NS_ASSERT (satPhyRx != 0);
      ObjectVectorValue carriers;
      satPhyRx->GetAttribute ("RxCarrierList", carriers);
      NS_LOG_DEBUG (this << " Node ID " << (*it)->GetId ()
                         << " device #" << dev->GetIfIndex ()
                         << " has " << carriers.GetN () << " RX carriers");

      for (ObjectVectorValue::Iterator itCarrier = carriers.Begin ();
           itCarrier != carriers.End (); ++itCarrier)
        {
          //NS_ASSERT (itCarrier->second->m_channelType == SatEnums::FORWARD_USER_CH)
          if (!itCarrier->second->TraceConnectWithoutContext ("LinkSinr",
                                                              GetTraceSinkCallback ()))
            {
              NS_FATAL_ERROR ("Error connecting to LinkSinr trace source"
                              << " of SatPhyRxCarrier"
                              << " at node ID " << (*it)->GetId ()
                              << " device #" << dev->GetIfIndex ()
                              << " RX carrier #" << itCarrier->first);
            }

        } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

    } // end of `for (it = uts.Begin(); it != uts.End (); ++it)`

} // end of `void DoInstallProbes ();`


// RETURN FEEDER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnFeederLinkSinrHelper);

SatStatsRtnFeederLinkSinrHelper::SatStatsRtnFeederLinkSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkSinrHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnFeederLinkSinrHelper::~SatStatsRtnFeederLinkSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnFeederLinkSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnFeederLinkSinrHelper")
    .SetParent<SatStatsLinkSinrHelper> ()
  ;
  return tid;
}


void
SatStatsRtnFeederLinkSinrHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
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
              //NS_ASSERT (itCarrier->second->m_channelType == SatEnums::RETURN_FEEDER_CH)
              if (!itCarrier->second->TraceConnectWithoutContext ("LinkSinr",
                                                                  GetTraceSinkCallback ()))
                {
                  NS_FATAL_ERROR ("Error connecting to LinkSinr trace source"
                                  << " of SatPhyRxCarrier"
                                  << " at node ID " << (*it)->GetId ()
                                  << " device #" << (*itDev)->GetIfIndex ()
                                  << " RX carrier #" << itCarrier->first);
                }

            } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

        } // end of `for (it = gws.Begin(); it != gws.End (); ++it)`

    } // end of `for (itDev = devs.Begin (); itDev != devs.End (); ++itDev)`

} // end of `void DoInstallProbes ();`


// RETURN USER LINK ////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (SatStatsRtnUserLinkSinrHelper);

SatStatsRtnUserLinkSinrHelper::SatStatsRtnUserLinkSinrHelper (Ptr<const SatHelper> satHelper)
  : SatStatsLinkSinrHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsRtnUserLinkSinrHelper::~SatStatsRtnUserLinkSinrHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsRtnUserLinkSinrHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsRtnUserLinkSinrHelper")
    .SetParent<SatStatsLinkSinrHelper> ()
  ;
  return tid;
}


void
SatStatsRtnUserLinkSinrHelper::DoInstallProbes ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Node> geoSat = GetSatHelper ()->GetBeamHelper ()->GetGeoSatNode ();
  NS_ASSERT (geoSat->GetNDevices () == 1);
  Ptr<NetDevice> dev = geoSat->GetDevice (0);
  Ptr<SatGeoNetDevice> satGeoDev = dev->GetObject<SatGeoNetDevice> ();
  NS_ASSERT (satGeoDev != 0);
  ObjectMapValue phy;
  satGeoDev->GetAttribute ("UserPhy", phy);
  NS_LOG_DEBUG (this << " GeoSat Node ID " << geoSat->GetId ()
                     << " device #" << dev->GetIfIndex ()
                     << " has " << phy.GetN () << " PHY instance(s)");

  for (ObjectMapValue::Iterator itPhy = phy.Begin ();
       itPhy != phy.End (); ++itPhy)
    {
      Ptr<SatPhy> satPhy = itPhy->second->GetObject<SatPhy> ();
      NS_ASSERT (satPhy != 0);
      Ptr<SatPhyRx> satPhyRx = satPhy->GetPhyRx ();
      NS_ASSERT (satPhyRx != 0);
      ObjectVectorValue carriers;
      satPhyRx->GetAttribute ("RxCarrierList", carriers);
      NS_LOG_DEBUG (this << " PHY #" << itPhy->first
                         << " has " << carriers.GetN () << " RX carrier(s)");

      for (ObjectVectorValue::Iterator itCarrier = carriers.Begin ();
           itCarrier != carriers.End (); ++itCarrier)
        {
          //NS_ASSERT (itCarrier->second->m_channelType == SatEnums::RETURN_USER_CH)
          if (!itCarrier->second->TraceConnectWithoutContext ("LinkSinr",
                                                              GetTraceSinkCallback ()))
            {
              NS_FATAL_ERROR ("Error connecting to LinkSinr trace source"
                              << " of SatPhyRxCarrier"
                              << " at GeoSat node ID " << geoSat->GetId ()
                              << " device #" << dev->GetIfIndex ()
                              << " PHY #" << itPhy->first
                              << " RX carrier #" << itCarrier->first);
            }

        } // end of `for (ObjectVectorValue::Iterator itCarrier = carriers)`

    } // end of `for (ObjectMapValue::Iterator itPhy = phys)`

} // end of `void DoInstallProbes ();`


} // end of namespace ns3
