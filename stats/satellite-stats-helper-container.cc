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

#include "satellite-stats-helper-container.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-stats-backlogged-request-helper.h>
#include <ns3/satellite-stats-capacity-request-helper.h>
#include <ns3/satellite-stats-composite-sinr-helper.h>
#include <ns3/satellite-stats-delay-helper.h>
#include <ns3/satellite-stats-frame-load-helper.h>
#include <ns3/satellite-stats-link-rx-power-helper.h>
#include <ns3/satellite-stats-link-sinr-helper.h>
#include <ns3/satellite-stats-packet-collision-helper.h>
#include <ns3/satellite-stats-packet-error-helper.h>
#include <ns3/satellite-stats-queue-helper.h>
#include <ns3/satellite-stats-resources-granted-helper.h>
#include <ns3/satellite-stats-signalling-load-helper.h>
#include <ns3/satellite-stats-throughput-helper.h>
#include <ns3/satellite-stats-waveform-usage-helper.h>
#include <ns3/satellite-stats-frame-type-usage-helper.h>
#include <ns3/satellite-stats-beam-service-time-helper.h>

NS_LOG_COMPONENT_DEFINE ("SatStatsHelperContainer");


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatStatsHelperContainer);


SatStatsHelperContainer::SatStatsHelperContainer (Ptr<const SatHelper> satHelper)
  : m_satHelper (satHelper)
{
  NS_LOG_FUNCTION (this);
}

void
SatStatsHelperContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}


/*
 * The macro definitions following this comment block are used to define most
 * attributes of this class. Below is the list of attributes created using this
 * C++ pre-processing approach.
 *
 * - [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppDelay
 * - [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Delay
 * - Average [Beam,Ut,UtUser] [Fwd,Rtn] AppDelay
 * - Average [Beam,Ut] [Fwd,Rtn] [Dev,Mac,Phy] Delay
 * - [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] Queue [Bytes,Packets]
 * - [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] SignallingLoad
 * - [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] CompositeSinr
 * - [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppThroughput
 * - [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Throughput
 * - Average [Beam,Ut,UtUser] [Fwd,Rtn] AppThroughput
 * - Average [Beam,Ut] [Fwd,Rtn] [Dev,Mac,Phy] Throughput
 * - [Global,PerGw,PerBeam,PerUt] [FwdDa,RtnDa,SlottedAloha,Crdsa] PacketError
 * - [Global,PerGw,PerBeam,PerUt] [SlottedAloha,Crdsa] PacketCollision
 * - [Global,PerGw,PerBeam,PerUt] CapacityRequest
 * - [Global,PerGw,PerBeam,PerUt] ResourcesGranted
 * - [Global,PerGw,PerBeam] BackloggedRequest
 * - [Global,PerGw,PerBeam] Frame [Symbol,User] Load
 * - [Global,PerGw,PerBeam] WaveformUsage
 * - Global [Fwd,Rtn] [Feeder,User] LinkSinr
 * - Global [Fwd,Rtn] [Feeder,User] LinkRxPower
 * - [Global,PerGw,PerBeam] FrameTypeUsage
 * - [PerBeam] BeamServiceTime
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                    \
  MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",             \
                   SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",      \
                   SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",     \
                   SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT"))

#define ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                             \
  MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",             \
                   SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",      \
                   SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",     \
                   SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",   \
                   SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",         \
                   SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",         \
                   SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",     \
                   SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",   \
                   SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",         \
                   SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))

#define ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER                    \
  MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",             \
                   SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",   \
                   SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",         \
                   SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",         \
                   SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",   \
                   SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",         \
                   SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))

#define ADD_SAT_STATS_ATTRIBUTE_HEAD(id, desc)                                \
  .AddAttribute (# id,                                                         \
                 std::string ("Enable the output of ") + desc,                \
                 EnumValue (SatStatsHelper::OUTPUT_NONE),                     \
                 MakeEnumAccessor (&SatStatsHelperContainer::Add ## id),      \

#define ADD_SAT_STATS_ATTRIBUTES_BASIC_SET(id, desc)                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id,                                 \
                                std::string ("global ") + desc)               \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id,                                  \
                                std::string ("per GW ") + desc)               \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id,                                \
                                std::string ("per beam ") + desc)             \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id,                                  \
                                std::string ("per UT ") + desc)               \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER

#define ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET(id, desc)                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id,                                 \
                                std::string ("global ") + desc)               \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id,                                  \
                                std::string ("per GW ") + desc)               \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id,                                \
                                std::string ("per beam ") + desc)             \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id,                                  \
                                std::string ("per UT ") + desc)               \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

#define ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET(id, desc)          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageBeam ## id,                            \
                                std::string ("average beam ") + desc)         \
  ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUt ## id,                              \
                                std::string ("average UT ") + desc)           \
  ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER


TypeId // static
SatStatsHelperContainer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsHelperContainer")
    .SetParent<Object> ()
    .AddAttribute ("Name",
                   "String to be prepended on every output file name",
                   StringValue ("stat"),
                   MakeStringAccessor (&SatStatsHelperContainer::SetName,
                                       &SatStatsHelperContainer::GetName),
                   MakeStringChecker ())

    // Forward link application-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdAppDelay,
                                               "forward link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppDelay,
                                  "per UT user forward link application-level delay statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdAppDelay,
                                                        "forward link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserFwdAppDelay,
                                  "average UT user forward link application-level delay statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

    // Forward link device-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdDevDelay,
                                               "forward link device-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdDevDelay,
                                                        "forward link device-level delay statistics")

    // Forward link MAC-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdMacDelay,
                                               "forward link MAC-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdMacDelay,
                                                        "forward link MAC-level delay statistics")

    // Forward link PHY-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdPhyDelay,
                                               "forward link PHY-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdPhyDelay,
                                                        "forward link PHY-level delay statistics")

    // Forward link queue size (in bytes) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdQueueBytes,
                                               "forward link queue size (in bytes) statistics")

    // Forward link queue size (in number of packets) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdQueuePackets,
                                               "forward link queue size (in number of packets) statistics")

    // Forward link signalling load statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdSignallingLoad,
                                        "forward link signalling load statistics")

    // Forward link composite SINR statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdCompositeSinr,
                                               "forward link composite SINR statistics")

    // Forward link application-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdAppThroughput,
                                        "forward link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppThroughput,
                                  "per UT user forward link application-level throughput statistics")
    ADD_SAT_STATS_BASIC_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdAppThroughput,
                                                        "forward link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserFwdAppThroughput,
                                  "average UT user forward link application-level throughput statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

    // Forward link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdDevThroughput,
                                        "forward link device-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdDevThroughput,
                                                        "forward link device-level throughput statistics")

    // Forward link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdMacThroughput,
                                        "forward link MAC-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdMacThroughput,
                                                        "forward link MAC-level throughput statistics")

    // Forward link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdPhyThroughput,
                                        "forward link PHY-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdPhyThroughput,
                                                        "forward link PHY-level throughput statistics")

    // Return link application-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnAppDelay,
                                               "return link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppDelay,
                                  "per UT user return link application-level delay statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnAppDelay,
                                                        "return link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserRtnAppDelay,
                                  "average UT user return link application-level delay statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

    // Return link device-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnDevDelay,
                                               "return link device-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnDevDelay,
                                                        "return link device-level delay statistics")

    // Return link MAC-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnMacDelay,
                                               "return link MAC-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnMacDelay,
                                                        "return link MAC-level delay statistics")

    // Return link PHY-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnPhyDelay,
                                               "return link PHY-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnPhyDelay,
                                                        "return link PHY-level delay statistics")

    // Return link queue size (in bytes) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnQueueBytes,
                                               "return link queue size (in bytes) statistics")

    // Return link queue size (in number of packets) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnQueuePackets,
                                               "return link queue size (in number of packets) statistics")

    // Return link signalling load statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnSignallingLoad,
                                        "return link signalling load statistics")

    // Return link composite SINR statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnCompositeSinr,
                                               "return link composite SINR statistics")

    // Return link application-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnAppThroughput,
                                        "return link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppThroughput,
                                  "per UT user return link application-level throughput statistics")
    ADD_SAT_STATS_BASIC_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnAppThroughput,
                                                        "return link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserRtnAppThroughput,
                                  "average UT user return link application-level throughput statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

    // Return link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnDevThroughput,
                                        "return link device-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnDevThroughput,
                                                        "return link device-level throughput statistics")

    // Return link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnMacThroughput,
                                        "return link MAC-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnMacThroughput,
                                                        "return link MAC-level throughput statistics")

    // Return link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnPhyThroughput,
                                        "return link PHY-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnPhyThroughput,
                                                        "return link PHY-level throughput statistics")

    // Forward link Dedicated Access packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdDaPacketError,
                                        "Forward link Dedicated Access packet error rate statistics")

    // Return link Dedicated Access packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnDaPacketError,
                                        "Return link Dedicated Access packet error rate statistics")

    // Random Access Slotted ALOHA packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (SlottedAlohaPacketError,
                                        "Random Access Slotted ALOHA packet error rate statistics")

    // Random Access Slotted ALOHA packet collision rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (SlottedAlohaPacketCollision,
                                        "Random Access Slotted ALOHA packet collision rate statistics")

    // Random Access CRDSA packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (CrdsaPacketError,
                                        "Random Access CRDSA packet error rate statistics")

    // Random Access CRDSA packet collision rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (CrdsaPacketCollision,
                                        "Random Access CRDSA packet collision rate statistics")

    // Capacity request statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalCapacityRequest,
                                  "global capacity request statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGwCapacityRequest,
                                  "per GW capacity request statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamCapacityRequest,
                                  "per beam capacity request statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtCapacityRequest,
                                  "per UT capacity request statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))

    // Resources granted statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (ResourcesGranted,
                                               "resources granted statistics")

    // Backlogged request statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalBackloggedRequest,
                                  "global backlogged request statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGwBackloggedRequest,
                                  "per GW backlogged request statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamBackloggedRequest,
                                  "per beam backlogged request statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))

    // Frame load statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalFrameSymbolLoad,
                                  "global frame load (in ratio of allocated symbols) statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGwFrameSymbolLoad,
                                  "per GW frame load (in ratio of allocated symbols) statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamFrameSymbolLoad,
                                  "per beam frame load (in ratio of allocated symbols) statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalFrameUserLoad,
                                  "global frame load (in number of scheduled users) statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGwFrameUserLoad,
                                  "per GW frame load (in number of scheduled users) statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamFrameUserLoad,
                                  "per UT frame load (in number of scheduled users) statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,         "NONE",
                     SatStatsHelper::OUTPUT_SCATTER_FILE, "SCATTER_FILE"))

    // Waveform usage statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalWaveformUsage,
                                  "global waveform usage statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,        "NONE",
                     SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGwWaveformUsage,
                                  "per GW waveform usage statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,        "NONE",
                     SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamWaveformUsage,
                                  "per beam waveform usage statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,        "NONE",
                     SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE"))

    // Link SINR statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalFwdFeederLinkSinr,
                                  "global forward feeder link SINR statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalFwdUserLinkSinr,
                                  "global forward user link SINR statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalRtnFeederLinkSinr,
                                  "global return feeder link SINR statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalRtnUserLinkSinr,
                                  "global return user link SINR statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

    // Link Rx power statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalFwdFeederLinkRxPower,
                                  "global forward feeder link Rx power statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalFwdUserLinkRxPower,
                                  "global forward user link Rx power statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalRtnFeederLinkRxPower,
                                  "global return feeder link Rx power statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalRtnUserLinkRxPower,
                                  "global return user link Rx power statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

    // Frame type usage statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalFrameTypeUsage,
                                  "global frame type usage statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,        "NONE",
                     SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGwFrameTypeUsage,
                                  "per GW frame type usage statistics")
    MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,        "NONE",
                     SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE"))
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamFrameTypeUsage,
                                  "per beam frame type usage statistics")
		MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,        "NONE",
										 SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE"))

		// Beam service time statistics
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamBeamServiceTime,
                                  "per beam service time statistics")
		MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,        "NONE",
										 SatStatsHelper::OUTPUT_SCALAR_FILE, "SCALAR_FILE"))
  ;
  return tid;
}


void
SatStatsHelperContainer::SetName (std::string name)
{
  NS_LOG_FUNCTION (this << name);

  // convert all spaces and slashes in the name to underscores
  for (size_t pos = name.find_first_of (" /");
       pos != std::string::npos;
       pos = name.find_first_of (" /", pos + 1, 1))
  {
    name[pos] = '_';
  }

  m_name = name;
}


std::string
SatStatsHelperContainer::GetName () const
{
  return m_name;
}


/*
 * The macro definitions following this comment block are used to declare the
 * majority of methods in this class. Below is the list of the class methods
 * created using this C++ pre-processing approach.
 *
 * - Add [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppDelay
 * - Add [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Delay
 * - AddAverage [Beam,Ut,UtUser] [Fwd,Rtn] AppDelay
 * - AddAverage [Beam,Ut] [Fwd,Rtn] [Dev,Mac,Phy] Delay
 * - Add [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] Queue [Bytes,Packets]
 * - Add [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] SignallingLoad
 * - Add [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] CompositeSinr
 * - Add [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppThroughput
 * - Add [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Throughput
 * - AddAverage [Beam,Ut,UtUser] [Fwd,Rtn] AppThroughput
 * - AddAverage [Beam,Ut] [Fwd,Rtn] [Dev,Mac,Phy] Throughput
 * - Add [Global,PerGw,PerBeam,PerUt] [FwdDa,RtnDa,SlottedAloha,Crdsa] PacketError
 * - Add [Global,PerGw,PerBeam,PerUt] [SlottedAloha,Crdsa] PacketCollision
 * - Add [Global,PerGw,PerBeam,PerUt] CapacityRequest
 * - Add [Global,PerGw,PerBeam,PerUt] ResourcesGranted
 * - Add [Global,PerGw,PerBeam] BackloggedRequest
 * - Add [Global,PerGw,PerBeam] Frame [Symbol,User] Load
 * - Add [Global,PerGw,PerBeam] WaveformUsage
 * - AddGlobal [Fwd,Rtn] [Feeder,User] LinkSinr
 * - AddGlobal [Fwd,Rtn] [Feeder,User] LinkRxPower
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define SAT_STATS_GLOBAL_METHOD_DEFINITION(id, name)                          \
  void                                                                          \
  SatStatsHelperContainer::AddGlobal ## id (SatStatsHelper::OutputType_t type)  \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-global-" + name                               \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);            \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

#define SAT_STATS_PER_GW_METHOD_DEFINITION(id, name)                          \
  void                                                                          \
  SatStatsHelperContainer::AddPerGw ## id (SatStatsHelper::OutputType_t type)   \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-per-gw-" + name                               \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);                \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

#define SAT_STATS_PER_BEAM_METHOD_DEFINITION(id, name)                        \
  void                                                                          \
  SatStatsHelperContainer::AddPerBeam ## id (SatStatsHelper::OutputType_t type) \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-per-beam-" + name                             \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);              \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

#define SAT_STATS_PER_UT_METHOD_DEFINITION(id, name)                          \
  void                                                                          \
  SatStatsHelperContainer::AddPerUt ## id (SatStatsHelper::OutputType_t type)   \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-per-ut-" + name                               \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);                \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

#define SAT_STATS_PER_UT_USER_METHOD_DEFINITION(id, name)                     \
  void                                                                          \
  SatStatsHelperContainer::AddPerUtUser ## id (SatStatsHelper::OutputType_t type) \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-per-ut-user-" + name                          \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);           \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

#define SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION(id, name)                    \
  void                                                                          \
  SatStatsHelperContainer::AddAverageBeam ## id (SatStatsHelper::OutputType_t type) \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-average-beam-" + name                             \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);              \
      stat->SetOutputType (type);                                             \
      stat->SetAveragingMode (true);                                          \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

#define SAT_STATS_AVERAGE_UT_METHOD_DEFINITION(id, name)                      \
  void                                                                          \
  SatStatsHelperContainer::AddAverageUt ## id (SatStatsHelper::OutputType_t type) \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-average-ut-" + name                           \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);                \
      stat->SetOutputType (type);                                             \
      stat->SetAveragingMode (true);                                          \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

#define SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION(id, name)                 \
  void                                                                          \
  SatStatsHelperContainer::AddAverageUtUser ## id (SatStatsHelper::OutputType_t type) \
  {                                                                             \
    NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
    if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = CreateObject<SatStats ## id ## Helper> (m_satHelper);               \
      stat->SetName (m_name + "-average-ut-user-" + name                      \
                     + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);           \
      stat->SetOutputType (type);                                             \
      stat->SetAveragingMode (true);                                          \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
  }

// Forward link application-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (FwdAppDelay, "fwd-app-delay")

// Forward link device-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")

// Forward link MAC-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")

// Forward link PHY-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")

// Forward link queue size (in bytes) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdQueueBytes, "fwd-queue-bytes")

// Forward link queue size (in number of packets) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdQueuePackets, "fwd-queue-packets")

// Forward link signalling load statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdSignallingLoad, "fwd-signalling-load")

// Forward link composite SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdCompositeSinr, "fwd-composite-sinr")

// Forward link application-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (FwdAppThroughput, "fwd-app-throughput")

// Forward link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdDevThroughput, "fwd-dev-throughput")

// Forward link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdMacThroughput, "fwd-mac-throughput")

// Forward link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (FwdPhyThroughput, "fwd-phy-throughput")

// Return link application-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (RtnAppDelay, "rtn-app-delay")

// Return link device-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")

// Return link MAC-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")

// Return link PHY-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")

// Return link queue size (in bytes) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnQueueBytes, "rtn-queue-bytes")

// Return link queue size (in number of packets) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnQueuePackets, "rtn-queue-packets")

// Return link signalling load statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnSignallingLoad, "rtn-signalling-load")

// Return link composite SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnCompositeSinr, "rtn-composite-sinr")

// Return link application-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (RtnAppThroughput, "rtn-app-throughput")

// Return link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnDevThroughput, "rtn-dev-throughput")

// Return link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnMacThroughput, "rtn-mac-throughput")

// Return link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION    (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION      (RtnPhyThroughput, "rtn-phy-throughput")

// Forward link Dedicated Access packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdDaPacketError, "fwd-da-error")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdDaPacketError, "fwd-da-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdDaPacketError, "fwd-da-error")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdDaPacketError, "fwd-da-error")

// Return link Dedicated Access packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnDaPacketError, "rtn-da-error")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnDaPacketError, "rtn-da-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnDaPacketError, "rtn-da-error")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnDaPacketError, "rtn-da-error")

// Random Access Slotted ALOHA packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (SlottedAlohaPacketError, "slotted-aloha-error")
SAT_STATS_PER_GW_METHOD_DEFINITION      (SlottedAlohaPacketError, "slotted-aloha-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (SlottedAlohaPacketError, "slotted-aloha-error")
SAT_STATS_PER_UT_METHOD_DEFINITION      (SlottedAlohaPacketError, "slotted-aloha-error")

// Random Access Slotted ALOHA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (SlottedAlohaPacketCollision, "slotted-aloha-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION      (SlottedAlohaPacketCollision, "slotted-aloha-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (SlottedAlohaPacketCollision, "slotted-aloha-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION      (SlottedAlohaPacketCollision, "slotted-aloha-collision")

// Random Access CRDSA packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (CrdsaPacketError, "crdsa-error")
SAT_STATS_PER_GW_METHOD_DEFINITION      (CrdsaPacketError, "crdsa-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (CrdsaPacketError, "crdsa-error")
SAT_STATS_PER_UT_METHOD_DEFINITION      (CrdsaPacketError, "crdsa-error")

// Random Access CRDSA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (CrdsaPacketCollision, "crdsa-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION      (CrdsaPacketCollision, "crdsa-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (CrdsaPacketCollision, "crdsa-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION      (CrdsaPacketCollision, "crdsa-collision")

// Capacity request statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (CapacityRequest, "capacity-request")
SAT_STATS_PER_GW_METHOD_DEFINITION      (CapacityRequest, "capacity-request")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (CapacityRequest, "capacity-request")
SAT_STATS_PER_UT_METHOD_DEFINITION      (CapacityRequest, "capacity-request")

// Resources granted statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (ResourcesGranted, "resources-granted")
SAT_STATS_PER_GW_METHOD_DEFINITION      (ResourcesGranted, "resources-granted")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (ResourcesGranted, "resources-granted")
SAT_STATS_PER_UT_METHOD_DEFINITION      (ResourcesGranted, "resources-granted")

// Backlogged request statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (BackloggedRequest, "backlogged-request")
SAT_STATS_PER_GW_METHOD_DEFINITION      (BackloggedRequest, "backlogged-request")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (BackloggedRequest, "backlogged-request")

// Frame load statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FrameSymbolLoad, "frame-symbol-load")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FrameSymbolLoad, "frame-symbol-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FrameSymbolLoad, "frame-symbol-load")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FrameUserLoad, "frame-user-load")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FrameUserLoad, "frame-user-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FrameUserLoad, "frame-user-load")

// Waveform usage statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (WaveformUsage, "waveform-usage")
SAT_STATS_PER_GW_METHOD_DEFINITION      (WaveformUsage, "waveform-usage")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (WaveformUsage, "waveform-usage")

// Link SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdUserLinkSinr,   "fwd-user-link-sinr")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnUserLinkSinr,   "rtn-user-link-sinr")

// Link Rx power statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdUserLinkRxPower,   "fwd-user-link-rx-power")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnUserLinkRxPower,   "rtn-user-link-rx-power")

// Frame type usage statistics
SAT_STATS_GLOBAL_METHOD_DEFINITION			(FrameTypeUsage, "frame-type-usage")
SAT_STATS_PER_GW_METHOD_DEFINITION			(FrameTypeUsage, "frame-type-usage")
SAT_STATS_PER_BEAM_METHOD_DEFINITION		(FrameTypeUsage, "frame-type-usage")

// Beam service time statistics
SAT_STATS_PER_BEAM_METHOD_DEFINITION 		(BeamServiceTime, "beam-service-time")


std::string // static
SatStatsHelperContainer::GetOutputTypeSuffix (SatStatsHelper::OutputType_t outputType)
{
  switch (outputType)
  {
  case SatStatsHelper::OUTPUT_NONE:
    return "";

  case SatStatsHelper::OUTPUT_SCALAR_FILE:
  case SatStatsHelper::OUTPUT_SCALAR_PLOT:
    return "-scalar";

  case SatStatsHelper::OUTPUT_SCATTER_FILE:
  case SatStatsHelper::OUTPUT_SCATTER_PLOT:
    return "-scatter";

  case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
  case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
    return "-histogram";

  case SatStatsHelper::OUTPUT_PDF_FILE:
  case SatStatsHelper::OUTPUT_PDF_PLOT:
    return "-pdf";

  case SatStatsHelper::OUTPUT_CDF_FILE:
  case SatStatsHelper::OUTPUT_CDF_PLOT:
    return "-cdf";

  default:
    NS_FATAL_ERROR ("SatStatsHelperContainer - Invalid output type");
    break;
  }

  NS_FATAL_ERROR ("SatStatsHelperContainer - Invalid output type");
  return "";
}


} // end of namespace ns3
