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

#include <ns3/node-container.h>
#include <ns3/satellite-net-device.h>
#include <ns3/satellite-ut-llc.h>
#include <ns3/satellite-request-manager.h>
#include <ns3/satellite-helper.h>

#include <ns3/data-collection-object.h>
#include <ns3/multi-file-aggregator.h>

#include "satellite-stats-capacity-request-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatStatsCapacityRequestHelper");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatStatsCapacityRequestHelper);

SatStatsCapacityRequestHelper::SatStatsCapacityRequestHelper (Ptr<const SatHelper> satHelper)
  : SatStatsHelper (satHelper)
{
  NS_LOG_FUNCTION (this << satHelper);
}


SatStatsCapacityRequestHelper::~SatStatsCapacityRequestHelper ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsCapacityRequestHelper::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsCapacityRequestHelper")
    .SetParent<SatStatsHelper> ()
  ;
  return tid;
}


void
SatStatsCapacityRequestHelper::DoInstall ()
{
  NS_LOG_FUNCTION (this);

  if (GetOutputType () != SatStatsHelper::OUTPUT_SCATTER_FILE)
    {
      NS_FATAL_ERROR (GetOutputTypeName (GetOutputType ())
                      << " is not a valid output type for this statistics.");
    }

  if (GetIdentifierType () == SatStatsHelper::IDENTIFIER_UT_USER)
    {
      NS_FATAL_ERROR (GetIdentifierTypeName (GetIdentifierType ())
                      << " is not a valid identifier type for this statistics.");
    }

  // Setup aggregator.
  m_aggregator = CreateAggregator ("ns3::MultiFileAggregator",
                                   "OutputFileName", StringValue (GetOutputFileName ()),
                                   "GeneralHeading",
                                   StringValue ("% time_sec, node_id, rc_id, type, kbps / bytes, queue_bytes"));
  Ptr<MultiFileAggregator> multiFileAggregator = m_aggregator->GetObject<MultiFileAggregator> ();
  NS_ASSERT (multiFileAggregator != 0);
  Callback<void, std::string, std::string> aggregatorSink
    = MakeCallback (&MultiFileAggregator::WriteString, multiFileAggregator);

  // Setup probes.
  NodeContainer uts = GetSatHelper ()->GetBeamHelper ()->GetUtNodes ();
  for (NodeContainer::Iterator it = uts.Begin (); it != uts.End (); ++it)
    {
      std::ostringstream context;
      switch (GetIdentifierType ())
        {
        case SatStatsHelper::IDENTIFIER_GLOBAL:
          context << "0";
          break;
        case SatStatsHelper::IDENTIFIER_GW:
          context << GetIdentifierForUt (*it);
          break;
        case SatStatsHelper::IDENTIFIER_BEAM:
          context << GetIdentifierForUt (*it);
          break;
        case SatStatsHelper::IDENTIFIER_UT:
          context << GetIdentifierForUt (*it);
          break;
        default:
          NS_FATAL_ERROR ("SatStatsCapacityRequestHelper - Invalid identifier type");
          break;
        }

      Ptr<NetDevice> dev = GetUtSatNetDevice (*it);
      Ptr<SatNetDevice> satDev = dev->GetObject<SatNetDevice> ();
      NS_ASSERT (satDev != 0);
      Ptr<SatLlc> satLlc = satDev->GetLlc ();
      NS_ASSERT (satLlc != 0);
      Ptr<SatUtLlc> utLlc = satLlc->GetObject<SatUtLlc> ();
      NS_ASSERT (utLlc != 0);
      Ptr<SatRequestManager> requestManager = utLlc->GetRequestManager ();

      const bool ret = requestManager->TraceConnect ("CrTraceLog",
                                                     context.str (),
                                                     aggregatorSink);
      NS_ASSERT_MSG (ret,
                     "Error connecting to CrTraceLog of node " << (*it)->GetId ());
      NS_UNUSED (ret);
      NS_LOG_INFO (this << " successfully connected"
                        << " with node ID " << (*it)->GetId ());

    } // end of `for (NodeContainer::Iterator it = uts.Begin () -> uts.End ())`

} // end of `void DoInstall ();`


} // end of namespace ns3
