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
#include <ns3/fatal-error.h>
#include <ns3/string.h>

#include <ns3/satellite-beam-scheduler.h>
#include <ns3/satellite-ncc.h>
#include <ns3/satellite-beam-helper.h>
#include <ns3/satellite-helper.h>

#include <ns3/data-collection-object.h>
#include <ns3/multi-file-aggregator.h>
#include <list>

#include "satellite-stats-backlogged-request-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsBackloggedRequestHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsBackloggedRequestHelper);

SatStatsBackloggedRequestHelper::SatStatsBackloggedRequestHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsBackloggedRequestHelper::~SatStatsBackloggedRequestHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsBackloggedRequestHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsBackloggedRequestHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsBackloggedRequestHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  if (GetOutputType () != SatStatsHelper::OUTPUT_SCATTER_FILE)
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
  m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                   "OutputFileName", StringValue (GetOutputFileName ()),
                                   "GeneralHeading",
                                   StringValue ("% time_sec, beam_id, ut_id, type, requests"));
  Ptr<MultiFileAggregator> multiFileAggregator = m_aggregator->GetObject<MultiFileAggregator> ();
  NS_ASSERT (multiFileAggregator != 0);
  Callback<void, std::string, std::string> aggregatorSink
    = MakeCallback (&MultiFileAggregator::WriteString, multiFileAggregator);

  // Setup probes.
  Ptr<SatBeamHelper> beamHelper = GetSatHelper ()->GetBeamHelper ();
  NS_ASSERT (beamHelper != 0);
  Ptr<SatNcc> ncc = beamHelper->GetNcc ();
  NS_ASSERT (ncc != 0);
  std::list<uint32_t> beams = beamHelper->GetBeams ();

  for (std::list<uint32_t>::const_iterator it = beams.begin ();
       it != beams.end (); ++it)
    {
      std::ostringstream context;
      switch (GetIdentifierType ())
        {
        case SatStatsHelper::IDENTIFIER_GLOBAL:
          context << "0";
          break;
        case SatStatsHelper::IDENTIFIER_GW:
          context << GetIdentifierForBeam (*it);
          break;
        case SatStatsHelper::IDENTIFIER_BEAM:
          context << GetIdentifierForBeam (*it);
          break;
        default:
          NS_FATAL_ERROR ("SatStatsBackloggedRequestHelper - Invalid identifier type");
          break;
        }

      Ptr<SatBeamScheduler> s = ncc->GetBeamScheduler (*it);
      NS_ASSERT_MSG (s != 0, "Error finding beam " << *it);
      const bool ret = s->TraceConnect ("BacklogRequestsTrace",
                                        context.str (), aggregatorSink);
      NS_ASSERT_MSG (ret,
                     "Error connecting to BacklogRequestsTrace of beam " << *it);
      NS_UNUSED (ret);
      NS_LOG_INFO (this << " successfully connected"
                        << " with beam " << *it);
    }

} // end of `void DoInstall ();`


} // end of namespace ns3
