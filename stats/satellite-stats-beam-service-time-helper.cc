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
 * Author: Lauri Sormunen <lauri.sormunen@magister.fi>
 *
 */

#include <ns3/log.h>
#include <ns3/fatal-error.h>
#include <ns3/boolean.h>
#include <ns3/string.h>
#include <ns3/enum.h>
#include <ns3/callback.h>

#include <ns3/node-container.h>
#include <ns3/satellite-beam-scheduler.h>
#include <ns3/satellite-ncc.h>
#include <ns3/satellite-beam-helper.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-gw-mac.h>

#include <ns3/data-collection-object.h>
#include <ns3/scalar-collector.h>
#include <ns3/distribution-collector.h>
#include <ns3/multi-file-aggregator.h>
#include <sstream>
#include <utility>
#include <list>

#include "satellite-stats-beam-service-time-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsBeamServiceTimeHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsBeamServiceTimeHelper);

SatStatsBeamServiceTimeHelper::SatStatsBeamServiceTimeHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsBeamServiceTimeHelper::~SatStatsBeamServiceTimeHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsBeamServiceTimeHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsBeamServiceTimeHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsBeamServiceTimeHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  if (GetOutputType () != SatStatsHelper::OUTPUT_SCALAR_FILE)
    {
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ())
                      << " is not a valid output type for this statistics.");
    }

  if (GetIdentifierType () != SatStatsHelper::IDENTIFIER_BEAM)
    {
      NS_FATAL_ERROR (GetIdentifierTypeName (GetIdentifierType ())
                      << " is not a valid identifier type for this statistics.");
    }

  // Setup aggregator.
  std::string dataLabel = "service_time";

  m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                   "OutputFileName", StringValue (GetOutputFileName ()),
                                   "MultiFileMode", BooleanValue (false),
                                   "EnableContextPrinting", BooleanValue (true),
                                   "GeneralHeading", StringValue ("% beam_id service_time"));

	// Prepare collector map
	NS_LOG_INFO (this << " Creating new collectors for beams");

	m_collectorMap.SetType ("ns3::ScalarCollector");
	m_collectorMap.SetAttribute ("OutputType",
															 EnumValue (ScalarCollector::OUTPUT_TYPE_SUM));
	m_collectorMap.SetAttribute ("InputDataType",
															 EnumValue (ScalarCollector::INPUT_DATA_TYPE_DOUBLE));


  // Create the callback for trace source
  Callback<void, std::string, Time> beamServiceCallback
    = MakeCallback (&SatStatsBeamServiceTimeHelper::BeamServiceCallback, this);

  // Connect SatGwMac of each beam by identifier (global, gw, beam) to callback
  NodeContainer gwNodes = GetSatHelper ()->GetBeamHelper ()->GetGwNodes();
  for (auto node = gwNodes.Begin (); node != gwNodes.End (); node++)
  {
  	for (uint32_t i = 0; i < (*node)->GetNDevices (); i++)
  	  {
				Ptr<SatNetDevice> dev = DynamicCast<SatNetDevice> ((*node)->GetDevice (i));
				if (dev == NULL) continue;
				Ptr<SatMac> mac = dev->GetMac ();
				if (mac == NULL) continue;

				// Connect the trace source
				uint32_t beamId = mac->GetBeamId ();
				std::ostringstream context;
				context << beamId;
	      const bool ret = mac->TraceConnect ("BeamServiceTime",
	                                        context.str (), beamServiceCallback);
	      NS_ASSERT_MSG (ret,
	                     "Error connecting to BeamServiceTime of beam " << beamId);
	      NS_UNUSED (ret);
	      NS_LOG_INFO (this << " successfully connected"
	                        << " with beam " << beamId);
  	  }
  }

} // end of `void DoInstall ();`


void
SatStatsBeamServiceTimeHelper::BeamServiceCallback (std::string context,
                                                      Time time)
{
  NS_LOG_FUNCTION (this << context << time.GetSeconds ());

  // convert context to number
  std::stringstream ss (context);
  uint32_t beamId;
  if (!(ss >> beamId))
    {
      NS_FATAL_ERROR ("Cannot convert '" << context << "' to number");
    }

  Ptr<DataCollectionObject> o = m_collectorMap.Get (beamId);
  if (o == 0)
  {
		std::ostringstream name;
		name << beamId;
		m_collectorMap.SetAttribute ("Name", StringValue (name.str ()));
		m_collectorMap.Create (beamId);
		o = m_collectorMap.Get (beamId);

		bool success = o->TraceConnect ("Output", context,
																		MakeCallback (&MultiFileAggregator::Write1d,
																		DynamicCast<MultiFileAggregator> (m_aggregator)));
		NS_ASSERT (success);
  }

  Ptr<ScalarCollector> s = o->GetObject<ScalarCollector> ();
  NS_ASSERT (s != 0);

	// Pass the sample to the collector.
	s->TraceSinkDouble (0, time.GetSeconds ());

} // end of `void BeamServiceCallback (std::string, uint32_t)`


} // end of namespace ns3
