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

#include "satellite-stats-frame-type-usage-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsFrameTypeUsageHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsFrameTypeUsageHelper);

const std::map<SatEnums::SatBbFrameType_t, uint32_t>
SatStatsFrameTypeUsageHelper::frameTypeIdMap = {{SatEnums::UNDEFINED_FRAME, 0},
																								{SatEnums::SHORT_FRAME, 1},
																								{SatEnums::NORMAL_FRAME, 2},
																								{SatEnums::DUMMY_FRAME, 3}};

SatStatsFrameTypeUsageHelper::SatStatsFrameTypeUsageHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper),
		m_usePercentage (false)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsFrameTypeUsageHelper::~SatStatsFrameTypeUsageHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsFrameTypeUsageHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsFrameTypeUsageHelper")
    .SetParent<SatStatsHelper> ()
		.AddAttribute ("Percentage", "Use percentage of each frame type instead of sum per identifier.",
									 BooleanValue (false),
									 MakeBooleanAccessor (&SatStatsFrameTypeUsageHelper::m_usePercentage),
									 MakeBooleanChecker ())
  ;
  return tid;
}


void
SatStatsFrameTypeUsageHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  if (GetOutputType () != SatStatsHelper::OUTPUT_SCALAR_FILE)
    {
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ())
                      << " is not a valid output type for this statistics.");
    }

  if (GetIdentifierType () == SatStatsHelper::IDENTIFIER_UT
      || GetIdentifierType () == SatStatsHelper::IDENTIFIER_UT_USER)
    {
      NS_FATAL_ERROR (GetIdentifierTypeName (GetIdentifierType ())
                      << " is not a valid identifier type for this statistics.");
    }

  // Setup aggregator.
  std::string dataLabel;
  if (m_usePercentage) dataLabel = "usage_percentage";
  else dataLabel = "usage_count";

  m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                   "OutputFileName", StringValue (GetOutputFileName ()),
                                   "MultiFileMode", BooleanValue (false),
                                   "EnableContextPrinting", BooleanValue (true),
                                   "GeneralHeading", StringValue (GetIdentifierHeading (dataLabel)));

  // For each frame type ID, create a CollectorMap instance
  for (auto it : frameTypeIdMap)
  {
  	uint32_t frameTypeId = it.second;
    // Newly discovered waveform ID
    NS_LOG_INFO (this << " Creating new collectors for frame type ID " << frameTypeId);

    CollectorMap collectorMap;
    ScalarCollector::OutputType_t opType;
    if (m_usePercentage) opType = ScalarCollector::OUTPUT_TYPE_AVERAGE_PER_SAMPLE;
    else opType = ScalarCollector::OUTPUT_TYPE_SUM;

		collectorMap.SetType ("ns3::ScalarCollector");
		collectorMap.SetAttribute ("OutputType",
															 EnumValue (opType));
		collectorMap.SetAttribute ("InputDataType",
															 EnumValue (ScalarCollector::INPUT_DATA_TYPE_UINTEGER));
		uint32_t n = 0;
    /*
     * Create a new set of collectors. Its name consists of two integers:
     *   - the first is the identifier ID (beam ID, GW ID, or simply zero for
     *     global);
     *   - the second is the frame ID.
     */
    switch (GetIdentifierType ())
      {
      case SatStatsHelper::IDENTIFIER_GLOBAL:
        {
          std::ostringstream name;
          name << "0 " << frameTypeId;
          collectorMap.SetAttribute ("Name", StringValue (name.str ()));
          collectorMap.Create (0);
          n++;
          break;
        }

      case SatStatsHelper::IDENTIFIER_GW:
        {
          NodeContainer gws = GetSatHelper ()->GetBeamHelper ()->GetGwNodes ();
          for (NodeContainer::Iterator it = gws.Begin (); it != gws.End (); ++it)
            {
              const uint32_t gwId = GetGwId (*it);
              std::ostringstream name;
              name << gwId << " " << frameTypeId;
              collectorMap.SetAttribute ("Name", StringValue (name.str ()));
              collectorMap.Create (gwId);
              n++;
            }
          break;
        }

      case SatStatsHelper::IDENTIFIER_BEAM:
        {
          std::list<uint32_t> beams = GetSatHelper ()->GetBeamHelper ()->GetBeams ();
          for (std::list<uint32_t>::const_iterator it = beams.begin ();
               it != beams.end (); ++it)
            {
              const uint32_t beamId = (*it);
              std::ostringstream name;
              name << beamId << " " << frameTypeId;
              collectorMap.SetAttribute ("Name", StringValue (name.str ()));
              collectorMap.Create (beamId);
              n++;
            }
          break;
        }

      default:
        NS_FATAL_ERROR ("SatStatsWaveformUsageHelper - Invalid identifier type");
        break;
      }

		collectorMap.ConnectToAggregator ("Output",
																			m_aggregator,
																			&MultiFileAggregator::Write1d);

    NS_LOG_INFO (this << " created " << n << " instance(s)"
                      << " of " << collectorMap.GetType ().GetName ()
                      << " for " << GetIdentifierTypeName (GetIdentifierType ()));

    std::pair<std::map<uint32_t, CollectorMap>::iterator, bool> ret;
    ret = m_collectors.insert (std::make_pair (frameTypeId, collectorMap));
    NS_ASSERT (ret.second);
  }

  // Create the callback for trace source
  Callback<void, std::string, SatEnums::SatBbFrameType_t> frameTypeUsageCallback
    = MakeCallback (&SatStatsFrameTypeUsageHelper::FrameTypeUsageCallback, this);

  // Connect SatGwMac of each beam by identifier (global, gw, beam) to callback
  NodeContainer gwNodes = GetSatHelper ()->GetBeamHelper ()->GetGwNodes();
  for (auto node = gwNodes.Begin (); node != gwNodes.End (); node++)
  {
  	for (uint32_t i = 0; i < (*node)->GetNDevices (); i++)
  	  {
				Ptr<SatNetDevice> dev = DynamicCast<SatNetDevice> ((*node)->GetDevice (i));
				if (dev == NULL) continue;
				Ptr<SatGwMac> mac = DynamicCast<SatGwMac> (dev->GetMac ());
				if (mac == NULL) continue;

				// Connect the trace source
				uint32_t beamId = mac->GetBeamId ();
				std::ostringstream context;
				context << GetIdentifierForBeam (beamId);
	      const bool ret = mac->TraceConnect ("BBFrameTxTrace",
	                                        context.str (), frameTypeUsageCallback);
	      NS_ASSERT_MSG (ret,
	                     "Error connecting to BBFrameTxTrace of beam " << beamId);
	      NS_UNUSED (ret);
	      NS_LOG_INFO (this << " successfully connected"
	                        << " with beam " << beamId);
  	  }
  }

} // end of `void DoInstall ();`


std::string
SatStatsFrameTypeUsageHelper::GetIdentifierHeading (std::string dataLabel) const
{
  switch (GetIdentifierType ())
    {
    case SatStatsHelper::IDENTIFIER_GLOBAL:
      return "% global frame_type " + dataLabel;

    case SatStatsHelper::IDENTIFIER_GW:
      return "% gw_id frame_type " + dataLabel;

    case SatStatsHelper::IDENTIFIER_BEAM:
      return "% beam_id frame_type " + dataLabel;

    default:
      NS_FATAL_ERROR ("SatStatsFrameTypeUsageHelper - Invalid identifier type");
      break;
    }
  return "";
}

// static
uint32_t
SatStatsFrameTypeUsageHelper::GetFrameTypeId (SatEnums::SatBbFrameType_t frameType)
{
	auto it = frameTypeIdMap.find (frameType);
	NS_ASSERT (it != frameTypeIdMap.end ());
	return it->second;
}


void
SatStatsFrameTypeUsageHelper::FrameTypeUsageCallback (std::string context,
                                                      SatEnums::SatBbFrameType_t frameType)
{
  NS_LOG_FUNCTION (this << context << SatEnums::GetFrameTypeName (frameType));

  // convert context to number
  std::stringstream ss (context);
  uint32_t identifier;
  if (!(ss >> identifier))
    {
      NS_FATAL_ERROR ("Cannot convert '" << context << "' to number");
    }
  uint32_t frameTypeId = GetFrameTypeId (frameType);


  std::map<uint32_t, CollectorMap>::iterator it = m_collectors.find (frameTypeId);

  NS_ASSERT (it != m_collectors.end ());
  if (!m_usePercentage)
		{
			// Find the collector with the right identifier.
			Ptr<DataCollectionObject> collector = it->second.Get (identifier);
			NS_ASSERT_MSG (collector != 0,
										 "Unable to find collector with identifier " << identifier);

			Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
			NS_ASSERT (c != 0);

			// Pass the sample to the collector.
			c->TraceSinkUinteger32 (0, 1);
		}
  else
    {
  		// Push 0 to all other frame type collectors of the beam/gw/global identifier
  	  // and 1 to the collector of the right frame type
  	  for (auto it : m_collectors)
  	  {
  	  	Ptr<DataCollectionObject> collector = it.second.Get (identifier);
  			NS_ASSERT_MSG (collector != 0,
  										 "Unable to find collector with identifier " << identifier);
  			Ptr<ScalarCollector> c = collector->GetObject<ScalarCollector> ();
  			NS_ASSERT (c != 0);
  	  	if (it.first == frameTypeId) c->TraceSinkUinteger32 (0, 1);
  	  	else c->TraceSinkUinteger32 (0, 0);
  	  }
    }

} // end of `void FrameTypeUsageCallback (std::string, uint32_t)`


} // end of namespace ns3
