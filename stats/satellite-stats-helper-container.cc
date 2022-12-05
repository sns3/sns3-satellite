/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#include "satellite-stats-helper-container.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-stats-antenna-gain-helper.h>
#include <ns3/satellite-stats-backlogged-request-helper.h>
#include <ns3/satellite-stats-capacity-request-helper.h>
#include <ns3/satellite-stats-carrier-id-helper.h>
#include <ns3/satellite-stats-composite-sinr-helper.h>
#include <ns3/satellite-stats-delay-helper.h>
#include <ns3/satellite-stats-link-delay-helper.h>
#include <ns3/satellite-stats-jitter-helper.h>
#include <ns3/satellite-stats-link-jitter-helper.h>
#include <ns3/satellite-stats-plt-helper.h>
#include <ns3/satellite-stats-frame-load-helper.h>
#include <ns3/satellite-stats-link-rx-power-helper.h>
#include <ns3/satellite-stats-link-sinr-helper.h>
#include <ns3/satellite-stats-link-modcod-helper.h>
#include <ns3/satellite-stats-marsala-correlation-helper.h>
#include <ns3/satellite-stats-packet-collision-helper.h>
#include <ns3/satellite-stats-packet-error-helper.h>
#include <ns3/satellite-stats-queue-helper.h>
#include <ns3/satellite-stats-satellite-queue-helper.h>
#include <ns3/satellite-stats-rbdc-request-helper.h>
#include <ns3/satellite-stats-resources-granted-helper.h>
#include <ns3/satellite-stats-signalling-load-helper.h>
#include <ns3/satellite-stats-throughput-helper.h>
#include <ns3/satellite-stats-waveform-usage-helper.h>
#include <ns3/satellite-stats-fwd-link-scheduler-symbol-rate-helper.h>
#include <ns3/satellite-stats-frame-type-usage-helper.h>
#include <ns3/satellite-stats-beam-service-time-helper.h>
#include <ns3/satellite-stats-window-load-helper.h>

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
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppDelay
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - Average [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppDelay
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkDelay
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkDelay
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppJitter
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - Average [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppJitter
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkJitter
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkJitter
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppPlt
 * - Average [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppPlt
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] Queue [Bytes, Packets]
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [RtnFeeder, FwdUser] Queue [Bytes, Packets]
 * - Average [Beam, Group, Ut, Sat] [RtnFeeder, FwdUser] Queue [Bytes, Packets]
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] SignallingLoad
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] CompositeSinr
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppThroughput
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] Throughput
 * - Average [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppThroughput
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] Throughput
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Feeder, User] [FwdDa, RtnDa, SlottedAloha, Crdsa, Essa] PacketError
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Feeder, User] [SlottedAloha, Crdsa, Essa] PacketCollision
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] CapacityRequest
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] ResourcesGranted
 * - [Global, PerGw, PerBeam] BackloggedRequest
 * - [Global, PerGw, PerBeam] Frame [Symbol, User] Load
 * - [Global, PerGw, PerBeam] WaveformUsage
 * - [Global, PerSlice] FwdLinkSchedulerSymbolRate
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] LinkSinr
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] LinkSinr
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] LinkRxPower
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] LinkRxPower
 * - [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] LinkModcod
 * - Average [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] LinkModcod
 * - [Global, PerGw, PerBeam] FrameTypeUsage
 * - [Global, PerGw, PerBeam] RtnFeederWindowLoad
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                            \
  MakeEnumChecker (                                                   \
      SatStatsHelper::OUTPUT_NONE,           "NONE",                  \
      SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",           \
      SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",          \
      SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT"))

#define ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                     \
  MakeEnumChecker (                                                   \
      SatStatsHelper::OUTPUT_NONE,           "NONE",                  \
      SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",           \
      SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",          \
      SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",        \
      SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",              \
      SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",              \
      SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",          \
      SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",        \
      SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",              \
      SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))

#define ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER            \
  MakeEnumChecker (                                                   \
      SatStatsHelper::OUTPUT_NONE,           "NONE",                  \
      SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",        \
      SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",              \
      SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",              \
      SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",        \
      SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",              \
      SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))

#define ADD_SAT_STATS_ATTRIBUTE_HEAD(id, desc)                        \
  .AddAttribute (# id,                                                \
      std::string ("Enable the output of ") + desc,                   \
      EnumValue (SatStatsHelper::OUTPUT_NONE),                        \
      MakeEnumAccessor (&SatStatsHelperContainer::Add ## id),         \

#define ADD_SAT_STATS_ATTRIBUTES_BASIC_SET(id, desc)                  \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id,                     \
        std::string ("global ") + desc)                               \
      ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                              \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id,                      \
        std::string ("per GW ") + desc)                               \
      ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                              \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerSat ## id,                     \
        std::string ("per SAT ") + desc)                              \
      ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                       \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id,                    \
        std::string ("per beam ") + desc)                             \
      ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                              \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGroup ## id,                   \
        std::string ("per group ") + desc)                            \
      ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                              \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id,                      \
        std::string ("per UT ") + desc)                               \
      ADD_SAT_STATS_BASIC_OUTPUT_CHECKER

#define ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET(id, desc)           \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id,                     \
        std::string ("global ") + desc)                               \
      ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                       \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id,                      \
        std::string ("per GW ") + desc)                               \
      ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                       \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerSat ## id,                     \
        std::string ("per SAT ") + desc)                              \
      ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                       \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id,                    \
        std::string ("per beam ") + desc)                             \
      ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                       \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGroup ## id,                   \
        std::string ("per group ") + desc)                            \
      ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                       \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id,                      \
        std::string ("per UT ") + desc)                               \
      ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

#define ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET(id, desc)  \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageBeam ## id,                \
        std::string ("average beam ") + desc)                         \
      ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER              \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageGroup ## id,               \
        std::string ("average group ") + desc)                        \
      ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER              \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUt ## id,                  \
        std::string ("average UT ") + desc)                           \
      ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER              \
      ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageSat ## id,                 \
        std::string ("average SAT ") + desc)                          \
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

    // Forward feeder link DEV-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederDevLinkDelay,
        "forward feeder link DEV-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederDevLinkDelay,
        "forward feeder link DEV-level link delay statistics")

    // Forward user link DEV-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserDevLinkDelay,
        "forward user link DEV-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserDevLinkDelay,
        "forward user link DEV-level link delay statistics")

    // Forward feeder link MAC-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederMacLinkDelay,
        "forward feeder link MAC-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederMacLinkDelay,
        "forward feeder link MAC-level link delay statistics")

    // Forward user link MAC-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserMacLinkDelay,
        "forward user link MAC-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserMacLinkDelay,
        "forward user link MAC-level link delay statistics")

    // Forward feeder link PHY-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederPhyLinkDelay,
        "forward feeder link PHY-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederPhyLinkDelay,
        "forward feeder link PHY-level link delay statistics")

    // Forward user link PHY-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserPhyLinkDelay,
        "forward user link PHY-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserPhyLinkDelay,
        "forward user link PHY-level link delay statistics")

    // Forward link application-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdAppJitter,
        "forward link application-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppJitter,
        "per UT user forward link application-level jitter statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdAppJitter,
        "forward link application-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserFwdAppJitter,
        "average UT user forward link application-level jitter statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

    // Forward link device-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdDevJitter,
        "forward link device-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdDevJitter,
        "forward link device-level jitter statistics")

    // Forward link MAC-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdMacJitter,
        "forward link MAC-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdMacJitter,
        "forward link MAC-level jitter statistics")

    // Forward link PHY-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdPhyJitter,
        "forward link PHY-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdPhyJitter,
        "forward link PHY-level jitter statistics")

    // Forward feeder link DEV-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederDevLinkJitter,
        "forward feeder link DEV-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederDevLinkJitter,
        "forward feeder link DEV-level link jitter statistics")

    // Forward user link DEV-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserDevLinkJitter,
        "forward user link DEV-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserDevLinkJitter,
        "forward user link DEV-level link jitter statistics")

    // Forward feeder link MAC-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederMacLinkJitter,
        "forward feeder link MAC-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederMacLinkJitter,
        "forward feeder link MAC-level link jitter statistics")

    // Forward user link MAC-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserMacLinkJitter,
        "forward user link MAC-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserMacLinkJitter,
        "forward user link MAC-level link jitter statistics")

    // Forward feeder link PHY-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederPhyLinkJitter,
        "forward feeder link PHY-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederPhyLinkJitter,
        "forward feeder link PHY-level link jitter statistics")

    // Forward user link PHY-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserPhyLinkJitter,
        "forward user link PHY-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserPhyLinkJitter,
        "forward user link PHY-level link jitter statistics")

    // Forward feeder link sinr statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederLinkSinr,
        "forward feeder link SINR statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederLinkSinr,
        "forward feeder link SINR statistics")

    // Forward user link sinr statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserLinkSinr,
        "forward user link SINR statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserLinkSinr,
        "forward user link SINR statistics")

    // Return feeder link sinr statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederLinkSinr,
        "return feeder link SINR statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederLinkSinr,
        "return feeder link SINR statistics")

    // Return user link sinr statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserLinkSinr,
        "return user link SINR statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserLinkSinr,
        "return user link SINR statistics")

    // Forward feeder link RX power statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederLinkRxPower,
        "forward feeder link RX power statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederLinkRxPower,
        "forward feeder link RX power statistics")

    // Forward user link RX power statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserLinkRxPower,
        "forward user link RX power statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserLinkRxPower,
        "forward user link RX power statistics")

    // Return feeder link RX power statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederLinkRxPower,
        "return feeder link RX power statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederLinkRxPower,
        "return feeder link RX power statistics")

    // Return user link RX power statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserLinkRxPower,
        "return user link RX power statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserLinkRxPower,
        "return user link RX power statistics")

    // Forward feeder link MODCOD statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdFeederLinkModcod,
        "forward feeder link MODCOD statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederLinkModcod,
        "forward feeder link MODCOD statistics")

    // Forward user link MODCOD statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserLinkModcod,
        "forward user link MODCOD statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserLinkModcod,
        "forward user link MODCOD statistics")

    // Return feeder link MODCOD statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederLinkModcod,
        "return feeder link MODCOD statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederLinkModcod,
        "return feeder link MODCOD statistics")

    // Return user link MODCOD statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserLinkModcod,
        "return user link MODCOD statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserLinkModcod,
        "return user link MODCOD statistics")

    // Forward link application-level packet PLT statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdAppPlt,
        "forward link application-level PLT statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppPlt,
        "per UT user forward link application-level PLT statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdAppPlt,
        "forward link application-level PLT statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserFwdAppPlt,
        "average UT user forward link application-level PLT statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

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

    // Forward feeder link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdFeederDevThroughput,
        "forward feeder link device-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederDevThroughput,
        "forward feeder link device-level throughput statistics")

    // Forward user link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdUserDevThroughput,
        "forward user link device-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserDevThroughput,
        "forward user link device-level throughput statistics")

    // Forward feeder link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdFeederMacThroughput,
        "forward feeder link MAC-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederMacThroughput,
        "forward feeder link MAC-level throughput statistics")

    // Forward user link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdUserMacThroughput,
        "forward user link MAC-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserMacThroughput,
        "forward user link MAC-level throughput statistics")

    // Forward feeder link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdFeederPhyThroughput,
        "forward feeder link PHY-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdFeederPhyThroughput,
        "forward feeder link PHY-level throughput statistics")

    // Forward user link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdUserPhyThroughput,
        "forward user link PHY-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserPhyThroughput,
        "forward user link PHY-level throughput statistics")

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

    // Return feeder link DEV-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederDevLinkDelay,
        "return feeder link DEV-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederDevLinkDelay,
        "return feeder link DEV-level link delay statistics")

    // Return user link DEV-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserDevLinkDelay,
        "return user link DEV-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserDevLinkDelay,
        "return user link DEV-level link delay statistics")

    // Return feeder link MAC-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederMacLinkDelay,
        "return feeder link MAC-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederMacLinkDelay,
        "return feeder link MAC-level link delay statistics")

    // Return user link MAC-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserMacLinkDelay,
        "return user link MAC-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserMacLinkDelay,
        "return user link MAC-level link delay statistics")

    // Return feeder link PHY-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederPhyLinkDelay,
        "return feeder link PHY-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederPhyLinkDelay,
        "return feeder link PHY-level link delay statistics")

    // Return user link PHY-level packet link delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserPhyLinkDelay,
        "return user link PHY-level link delay statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserPhyLinkDelay,
        "return user link PHY-level link delay statistics")

    // Return feeder link DEV-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederDevLinkJitter,
        "forward feeder link DEV-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederDevLinkJitter,
        "forward feeder link DEV-level link jitter statistics")

    // Return user link DEV-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserDevLinkJitter,
        "forward user link DEV-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserDevLinkJitter,
        "forward user link DEV-level link jitter statistics")

    // Return feeder link MAC-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederMacLinkJitter,
        "forward feeder link MAC-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederMacLinkJitter,
        "forward feeder link MAC-level link jitter statistics")

    // Return user link MAC-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserMacLinkJitter,
        "forward user link MAC-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserMacLinkJitter,
        "forward user link MAC-level link jitter statistics")

    // Return feeder link PHY-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederPhyLinkJitter,
        "forward feeder link PHY-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederPhyLinkJitter,
        "forward feeder link PHY-level link jitter statistics")

    // Return user link PHY-level packet link jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnUserPhyLinkJitter,
        "forward user link PHY-level link jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserPhyLinkJitter,
        "forward user link PHY-level link jitter statistics")

    // Return link application-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnAppJitter,
        "return link application-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppJitter,
        "per UT user return link application-level jitter statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnAppJitter,
        "return link application-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserRtnAppJitter,
        "average UT user return link application-level jitter statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

    // Return link device-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnDevJitter,
        "return link device-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnDevJitter,
        "return link device-level jitter statistics")

    // Return link MAC-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnMacJitter,
        "return link MAC-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnMacJitter,
        "return link MAC-level jitter statistics")

    // Return link PHY-level packet jitter statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnPhyJitter,
        "return link PHY-level jitter statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnPhyJitter,
        "return link PHY-level jitter statistics")

    // Return link application-level packet PLT statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnAppPlt,
        "return link application-level PLT statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppPlt,
        "per UT user return link application-level PLT statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnAppPlt,
        "return link application-level PLT statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (AverageUtUserRtnAppPlt,
        "average UT user return link application-level PLT statistics")
    ADD_SAT_STATS_AVERAGED_DISTRIBUTION_OUTPUT_CHECKER

    // Return link queue size (in bytes) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnQueueBytes,
        "return link queue size (in bytes) statistics")

    // Return link queue size (in number of packets) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnQueuePackets,
        "return link queue size (in number of packets) statistics")

    // Return feeder link queue size (in bytes) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederQueueBytes,
        "return feeder link queue size (in bytes) statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederQueueBytes,
        "return feeder link queue size (in bytes) statistics")

    // Return feeder link queue size (in number of packets) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnFeederQueuePackets,
        "return feeder link queue size (in number of packets) statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederQueuePackets,
        "return feeder link queue size (in number of packets) statistics")

    // Forward user link queue size (in bytes) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserQueueBytes,
        "forward user link queue size (in bytes) statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserQueueBytes,
        "forward user link queue size (in bytes) statistics")

    // Forward user link queue size (in number of packets) statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdUserQueuePackets,
        "forward user link queue size (in number of packets) statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (FwdUserQueuePackets,
        "forward user link queue size (in number of packets) statistics")

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

    // Return feeder link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnFeederDevThroughput,
        "return feeder link device-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederDevThroughput,
        "return feeder link device-level throughput statistics")

    // Return user link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnUserDevThroughput,
        "return user link device-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserDevThroughput,
        "return user link device-level throughput statistics")

    // Return feeder link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnFeederMacThroughput,
        "return feeder link MAC-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederMacThroughput,
        "return feeder link MAC-level throughput statistics")

    // Return user link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnUserMacThroughput,
        "return user link MAC-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserMacThroughput,
        "return user link MAC-level throughput statistics")

    // Return feeder link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnFeederPhyThroughput,
        "return feeder link PHY-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnFeederPhyThroughput,
        "return feeder link PHY-level throughput statistics")

    // Return user link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnUserPhyThroughput,
        "return user link PHY-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTES_AVERAGED_DISTRIBUTION_SET (RtnUserPhyThroughput,
        "return user link PHY-level throughput statistics")

    // Forward link Dedicated Access packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdFeederDaPacketError,
        "Forward feeder link Dedicated Access packet error rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdUserDaPacketError,
        "Forward user link Dedicated Access packet error rate statistics")

    // Return link Dedicated Access packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnFeederDaPacketError,
        "Return feeder link Dedicated Access packet error rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnUserDaPacketError,
        "Return user link Dedicated Access packet error rate statistics")

    // Random Access Slotted ALOHA packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FeederSlottedAlohaPacketError,
        "Feeder link Random Access Slotted ALOHA packet error rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (UserSlottedAlohaPacketError,
        "User link Random Access Slotted ALOHA packet error rate statistics")

    // Random Access Slotted ALOHA packet collision rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FeederSlottedAlohaPacketCollision,
        "Random Access Feeder Slotted ALOHA packet collision rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (UserSlottedAlohaPacketCollision,
        "Random Access User Slotted ALOHA packet collision rate statistics")

    // Random Access CRDSA packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FeederCrdsaPacketError,
        "Feeder link Random Access CRDSA packet error rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (UserCrdsaPacketError,
        "User link Random Access CRDSA packet error rate statistics")

    // Random Access CRDSA packet collision rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FeederCrdsaPacketCollision,
        "Random Access Feeder CRDSA packet collision rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (UserCrdsaPacketCollision,
        "Random Access User CRDSA packet collision rate statistics")

    // Random Access E-SSA packet error rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FeederEssaPacketError,
                                        "Feeder link Random Access E-SSA packet error rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (UserEssaPacketError,
                                        "User link Random Access E-SSA packet error rate statistics")

    // Random Access E-SSA packet collision rate statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FeederEssaPacketCollision,
                                        "Random Access Feeder E-SSA packet collision rate statistics")
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (UserEssaPacketCollision,
                                        "Random Access User E-SSA packet collision rate statistics")

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
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGroupCapacityRequest,
                                  "per group capacity request statistics")
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

    // Window load statistics.
    ADD_SAT_STATS_ATTRIBUTE_HEAD (GlobalRtnFeederWindowLoad,
                                  "global return feeder window load statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGwRtnFeederWindowLoad,
                                  "per gw return feeder window load statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeamRtnFeederWindowLoad,
                                  "per beam return feeder window load statistics")
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
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppDelay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppDelay
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkDelay
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkDelay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppJitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppJitter
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkJitter
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkJitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppPlt
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppPlt
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] Queue [Bytes, Packets]
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [RtnFeeder, FwdUser] Queue [Bytes, Packets
 * - AddAverage [Beam, Group, Ut, Sat] [RtnFeeder, FwdUser] Queue [Bytes, Packets] * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] SignallingLoad
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] CompositeSinr
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppThroughput
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] Throughput
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppThroughput
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] Throughput
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Feeder, User] [FwdDa, RtnDa, SlottedAloha, Crdsa, Essa] PacketError
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Feeder, User] [SlottedAloha, Crdsa, Essa] PacketCollision
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] CapacityRequest
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] ResourcesGranted
 * - Add [Global, PerGw, PerBeam] BackloggedRequest
 * - Add [Global, PerGw, PerBeam] Frame [Symbol, User] Load
 * - Add [Global, PerGw, PerBeam] WaveformUsage
 * - Add [Global, PerSlice] FwdLinkSchedulerSymbolRate
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] LinkSinr
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] LinkSinr
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] LinkRxPower
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] LinkRxPower
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] LinkModcod
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] LinkModcod
 * - Add [Global, PerGw, PerBeam] FrameTypeUsage
 * - Add [Global, PerGw, PerBeam] RtnFeederWindowLoad
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define SAT_STATS_GLOBAL_METHOD_DEFINITION(id, name)                                                  \
  void                                                                                                \
  SatStatsHelperContainer::AddGlobal ## id (SatStatsHelper::OutputType_t type)                        \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-global-" + name + GetOutputTypeSuffix (type);                 \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);                                \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_PER_GW_METHOD_DEFINITION(id, name)                                                  \
  void                                                                                                \
  SatStatsHelperContainer::AddPerGw ## id (SatStatsHelper::OutputType_t type)                         \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-per-gw-" + name + GetOutputTypeSuffix (type);                 \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);                                    \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_PER_BEAM_METHOD_DEFINITION(id, name)                                                \
  void                                                                                                \
  SatStatsHelperContainer::AddPerBeam ## id (SatStatsHelper::OutputType_t type)                       \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-per-beam-" + name + GetOutputTypeSuffix (type);               \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);                                  \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_PER_GROUP_METHOD_DEFINITION(id, name)                                               \
  void                                                                                                \
  SatStatsHelperContainer::AddPerGroup ## id (SatStatsHelper::OutputType_t type)                      \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-per-group-" + name + GetOutputTypeSuffix (type);              \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GROUP);                                 \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_PER_UT_METHOD_DEFINITION(id, name)                                                  \
  void                                                                                                \
  SatStatsHelperContainer::AddPerUt ## id (SatStatsHelper::OutputType_t type)                         \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-per-ut-" + name + GetOutputTypeSuffix (type);                 \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);                                    \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_PER_UT_USER_METHOD_DEFINITION(id, name)                                             \
  void                                                                                                \
  SatStatsHelperContainer::AddPerUtUser ## id (SatStatsHelper::OutputType_t type)                     \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-per-ut-user-" + name + GetOutputTypeSuffix (type);            \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);                               \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION(id, name)                                            \
  void                                                                                                \
  SatStatsHelperContainer::AddAverageBeam ## id (SatStatsHelper::OutputType_t type)                   \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-average-beam-" + name + GetOutputTypeSuffix (type);           \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);                                  \
          stat->SetOutputType (type);                                                                 \
          stat->SetAveragingMode (true);                                                              \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(id, name)                                           \
  void                                                                                                \
  SatStatsHelperContainer::AddAverageGroup ## id (SatStatsHelper::OutputType_t type)                  \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-average-group-" + name + GetOutputTypeSuffix (type);          \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GROUP);                                 \
          stat->SetOutputType (type);                                                                 \
          stat->SetAveragingMode (true);                                                              \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_AVERAGE_UT_METHOD_DEFINITION(id, name)                                              \
  void                                                                                                \
  SatStatsHelperContainer::AddAverageUt ## id (SatStatsHelper::OutputType_t type)                     \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-average-ut-" + name + GetOutputTypeSuffix (type);             \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);                                    \
          stat->SetOutputType (type);                                                                 \
          stat->SetAveragingMode (true);                                                              \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION(id, name)                                         \
  void                                                                                                \
  SatStatsHelperContainer::AddAverageUtUser ## id (SatStatsHelper::OutputType_t type)                 \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-average-ut-user-" + name + GetOutputTypeSuffix (type);        \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);                               \
          stat->SetOutputType (type);                                                                 \
          stat->SetAveragingMode (true);                                                              \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION(id, name)                                             \
  void                                                                                                \
  SatStatsHelperContainer::AddAverageSat ## id (SatStatsHelper::OutputType_t type)                    \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-average-sat-" + name + GetOutputTypeSuffix (type);            \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_SAT);                                   \
          stat->SetOutputType (type);                                                                 \
          stat->SetAveragingMode (true);                                                              \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_PER_SLICE_METHOD_DEFINITION(id, name)                                               \
  void                                                                                                \
  SatStatsHelperContainer::AddPerSlice ## id (SatStatsHelper::OutputType_t type)                      \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-per-slice-" + name + GetOutputTypeSuffix (type);              \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_SLICE);                                 \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }

#define SAT_STATS_PER_SAT_METHOD_DEFINITION(id, name)                                                 \
  void                                                                                                \
  SatStatsHelperContainer::AddPerSat ## id (SatStatsHelper::OutputType_t type)                        \
    {                                                                                                 \
      NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));                             \
      std::string statName = m_name + "-per-sat-" + name + GetOutputTypeSuffix (type);                \
      if (type != SatStatsHelper::OUTPUT_NONE && m_names.count (statName) == 0)                       \
        {                                                                                             \
          Ptr<SatStats ## id ## Helper> stat = CreateObject<SatStats ## id ## Helper> (m_satHelper);  \
          stat->SetName (statName);                                                                   \
          stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_SAT);                                   \
          stat->SetOutputType (type);                                                                 \
          stat->Install ();                                                                           \
          m_stats.push_back (stat);                                                                   \
          m_names.insert (statName);                                                                  \
        }                                                                                             \
    }


// Forward link application-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (FwdAppDelay, "fwd-app-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdAppDelay, "fwd-app-delay")

// Forward link device-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdDevDelay, "fwd-dev-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdDevDelay, "fwd-dev-delay")

// Forward link MAC-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdMacDelay, "fwd-mac-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdMacDelay, "fwd-mac-delay")

// Forward link PHY-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdPhyDelay, "fwd-phy-delay")

// Forward feeder link DEV-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederDevLinkDelay, "fwd-feeder-dev-link-delay")

// Forward user link DEV-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserDevLinkDelay, "fwd-user-dev-link-delay")

// Forward feeder link MAC-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederMacLinkDelay, "fwd-feeder-mac-link-delay")

// Forward user link MAC-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserMacLinkDelay, "fwd-user-mac-link-delay")

// Forward feeder link PHY-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederPhyLinkDelay, "fwd-feeder-phy-link-delay")

// Forward user link PHY-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserPhyLinkDelay, "fwd-user-phy-link-delay")

// Forward link application-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdAppJitter, "fwd-app-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (FwdAppJitter, "fwd-app-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdAppJitter, "fwd-app-jitter")

// Forward link device-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdDevJitter, "fwd-dev-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdDevJitter, "fwd-dev-jitter")

// Forward link MAC-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdMacJitter, "fwd-mac-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdMacJitter, "fwd-mac-jitter")

// Forward link PHY-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdPhyJitter, "fwd-phy-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdPhyJitter, "fwd-phy-jitter")

// Forward feeder link DEV-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederDevLinkJitter, "fwd-feeder-dev-link-jitter")

// Forward user link DEV-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserDevLinkJitter, "fwd-user-dev-link-jitter")

// Forward feeder link MAC-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederMacLinkJitter, "fwd-feeder-mac-link-jitter")

// Forward user link MAC-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserMacLinkJitter, "fwd-user-mac-link-jitter")

// Forward feeder link PHY-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederPhyLinkJitter, "fwd-feeder-phy-link-jitter")

// Forward user link PHY-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserPhyLinkJitter, "fwd-user-phy-link-jitter")

// Forward link application-level packet PLT statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdAppPlt, "fwd-app-plt")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdAppPlt, "fwd-app-plt")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdAppPlt, "fwd-app-plt")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdAppPlt, "fwd-app-plt")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdAppPlt, "fwd-app-plt")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (FwdAppPlt, "fwd-app-plt")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdAppPlt, "fwd-app-plt")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdAppPlt, "fwd-app-plt")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdAppPlt, "fwd-app-plt")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdAppPlt, "fwd-app-plt")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (FwdAppPlt, "fwd-app-plt")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdAppPlt, "fwd-app-plt")

// Forward link queue size (in bytes) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdQueueBytes, "fwd-queue-bytes")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdQueueBytes, "fwd-queue-bytes")

// Forward link queue size (in number of packets) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdQueuePackets, "fwd-queue-packets")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdQueuePackets, "fwd-queue-packets")

// Forward link signalling load statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdSignallingLoad, "fwd-signalling-load")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdSignallingLoad, "fwd-signalling-load")

// Forward link composite SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdCompositeSinr, "fwd-composite-sinr")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdCompositeSinr, "fwd-composite-sinr")

// Forward link application-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdAppThroughput, "fwd-app-throughput")

// Forward feeder link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederDevThroughput, "fwd-feeder-dev-throughput")

// Forward user link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserDevThroughput, "fwd-user-dev-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserDevThroughput, "fwd-user-dev-throughput")

// Forward feeder link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederMacThroughput, "fwd-feeder-mac-throughput")

// Forward user link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserMacThroughput, "fwd-user-mac-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserMacThroughput, "fwd-user-mac-throughput")

// Forward feeder link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederPhyThroughput, "fwd-feeder-phy-throughput")

// Forward user link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserPhyThroughput, "fwd-user-phy-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserPhyThroughput, "fwd-user-phy-throughput")

// Return link application-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (RtnAppDelay, "rtn-app-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnAppDelay, "rtn-app-delay")

// Return link device-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnDevDelay, "rtn-dev-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnDevDelay, "rtn-dev-delay")

// Return link MAC-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnMacDelay, "rtn-mac-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnMacDelay, "rtn-mac-delay")

// Return link PHY-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnPhyDelay, "rtn-phy-delay")

// Return feeder link DEV-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederDevLinkDelay, "rtn-feeder-dev-link-delay")

// Return user link DEV-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserDevLinkDelay, "rtn-user-dev-link-delay")

// Return feeder link MAC-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederMacLinkDelay, "rtn-feeder-mac-link-delay")

// Return user link MAC-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserMacLinkDelay, "rtn-user-mac-link-delay")

// Return feeder link PHY-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederPhyLinkDelay, "rtn-feeder-phy-link-delay")

// Return user link PHY-level packet link delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserPhyLinkDelay, "rtn-user-phy-link-delay")

// Return link application-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnAppJitter, "rtn-app-jitter")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (RtnAppJitter, "rtn-app-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnAppJitter, "rtn-app-jitter")

// Return link device-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnDevJitter, "rtn-dev-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnDevJitter, "rtn-dev-jitter")

// Return link MAC-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnMacJitter, "rtn-mac-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnMacJitter, "rtn-mac-jitter")

// Return link PHY-level packet jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnPhyJitter, "rtn-phy-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnPhyJitter, "rtn-phy-jitter")

// Forward feeder link DEV-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederDevLinkJitter, "rtn-feeder-dev-link-jitter")

// Forward user link DEV-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserDevLinkJitter, "rtn-user-dev-link-jitter")

// Forward feeder link MAC-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederMacLinkJitter, "rtn-feeder-mac-link-jitter")

// Forward user link MAC-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserMacLinkJitter, "rtn-user-mac-link-jitter")

// Forward feeder link PHY-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederPhyLinkJitter, "rtn-feeder-phy-link-jitter")

// Forward user link PHY-level packet link jitter statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserPhyLinkJitter, "rtn-user-phy-link-jitter")

// Return link application-level packet PLT statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnAppPlt, "rtn-app-plt")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnAppPlt, "rtn-app-plt")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnAppPlt, "rtn-app-plt")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnAppPlt, "rtn-app-plt")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnAppPlt, "rtn-app-plt")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (RtnAppPlt, "rtn-app-plt")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnAppPlt, "rtn-app-plt")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnAppPlt, "rtn-app-plt")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnAppPlt, "rtn-app-plt")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnAppPlt, "rtn-app-plt")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (RtnAppPlt, "rtn-app-plt")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnAppPlt, "rtn-app-plt")

// Return link queue size (in bytes) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnQueueBytes, "rtn-queue-bytes")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnQueueBytes, "rtn-queue-bytes")

// Return link queue size (in number of packets) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnQueuePackets, "rtn-queue-packets")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnQueuePackets, "rtn-queue-packets")

// Return feeder link queue size (in bytes) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederQueueBytes, "rtn-feeder-queue-bytes")

// Return feeder link queue size (in number of packets) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederQueuePackets, "rtn-feeder-queue-packets")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederQueuePackets, "rtn-feeder-queue-packets")

// Forward user link queue size (in bytes) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserQueueBytes, "fwd-user-queue-bytes")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserQueueBytes, "fwd-user-queue-bytes")

// Forward user link queue size (in number of packets) statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserQueuePackets, "fwd-user-queue-packets")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserQueuePackets, "fwd-user-queue-packets")

// Return link signalling load statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnSignallingLoad, "rtn-signalling-load")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnSignallingLoad, "rtn-signalling-load")

// Return link composite SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnCompositeSinr, "rtn-composite-sinr")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnCompositeSinr, "rtn-composite-sinr")

// Return link application-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION  (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_UT_USER_METHOD_DEFINITION (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnAppThroughput, "rtn-app-throughput")

// Return feeder link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederDevThroughput, "rtn-feeder-dev-throughput")

// Return user link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserDevThroughput, "rtn-user-dev-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserDevThroughput, "rtn-user-dev-throughput")

// Return feeder link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederMacThroughput, "rtn-feeder-mac-throughput")

// Return user link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserMacThroughput, "rtn-user-mac-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserMacThroughput, "rtn-user-mac-throughput")

// Return feeder link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederPhyThroughput, "rtn-feeder-phy-throughput")

// Return user link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserPhyThroughput, "rtn-user-phy-throughput")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserPhyThroughput, "rtn-user-phy-throughput")

// Forward link Dedicated Access packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederDaPacketError, "fwd-feeder-da-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederDaPacketError, "fwd-feeder-da-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederDaPacketError, "fwd-feeder-da-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederDaPacketError, "fwd-feeder-da-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederDaPacketError, "fwd-feeder-da-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederDaPacketError, "fwd-feeder-da-error")

// Forward link Dedicated Access packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserDaPacketError, "fwd-user-da-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserDaPacketError, "fwd-user-da-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserDaPacketError, "fwd-user-da-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserDaPacketError, "fwd-user-da-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserDaPacketError, "fwd-user-da-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserDaPacketError, "fwd-user-da-error")

// Return link Dedicated Access packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederDaPacketError, "rtn-feeder-da-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederDaPacketError, "rtn-feeder-da-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederDaPacketError, "rtn-feeder-da-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederDaPacketError, "rtn-feeder-da-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederDaPacketError, "rtn-feeder-da-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederDaPacketError, "rtn-feeder-da-error")

// Return link Dedicated Access packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserDaPacketError, "rtn-user-da-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserDaPacketError, "rtn-user-da-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserDaPacketError, "rtn-user-da-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserDaPacketError, "rtn-user-da-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserDaPacketError, "rtn-user-da-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserDaPacketError, "rtn-user-da-error")

// Random Access Slotted ALOHA packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FeederSlottedAlohaPacketError, "feeder-slotted-aloha-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FeederSlottedAlohaPacketError, "feeder-slotted-aloha-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FeederSlottedAlohaPacketError, "feeder-slotted-aloha-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FeederSlottedAlohaPacketError, "feeder-slotted-aloha-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FeederSlottedAlohaPacketError, "feeder-slotted-aloha-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FeederSlottedAlohaPacketError, "feeder-slotted-aloha-error")

// Random Access Slotted ALOHA packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (UserSlottedAlohaPacketError, "user-slotted-aloha-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (UserSlottedAlohaPacketError, "user-slotted-aloha-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (UserSlottedAlohaPacketError, "user-slotted-aloha-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (UserSlottedAlohaPacketError, "user-slotted-aloha-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (UserSlottedAlohaPacketError, "user-slotted-aloha-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (UserSlottedAlohaPacketError, "user-slotted-aloha-error")

// Random Access Feeder Slotted ALOHA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FeederSlottedAlohaPacketCollision, "feeder-slotted-aloha-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FeederSlottedAlohaPacketCollision, "feeder-slotted-aloha-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FeederSlottedAlohaPacketCollision, "feeder-slotted-aloha-collision")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FeederSlottedAlohaPacketCollision, "feeder-slotted-aloha-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FeederSlottedAlohaPacketCollision, "feeder-slotted-aloha-collision")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FeederSlottedAlohaPacketCollision, "feeder-slotted-aloha-collision")

// Random Access User Slotted ALOHA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (UserSlottedAlohaPacketCollision, "user-slotted-aloha-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION       (UserSlottedAlohaPacketCollision, "user-slotted-aloha-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (UserSlottedAlohaPacketCollision, "user-slotted-aloha-collision")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (UserSlottedAlohaPacketCollision, "user-slotted-aloha-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION       (UserSlottedAlohaPacketCollision, "user-slotted-aloha-collision")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (UserSlottedAlohaPacketCollision, "user-slotted-aloha-collision")

// Random Access CRDSA packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FeederCrdsaPacketError, "feeder-crdsa-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FeederCrdsaPacketError, "feeder-crdsa-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FeederCrdsaPacketError, "feeder-crdsa-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FeederCrdsaPacketError, "feeder-crdsa-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FeederCrdsaPacketError, "feeder-crdsa-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FeederCrdsaPacketError, "feeder-crdsa-error")

// Random Access CRDSA packet error rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (UserCrdsaPacketError, "user-crdsa-error")
SAT_STATS_PER_GW_METHOD_DEFINITION       (UserCrdsaPacketError, "user-crdsa-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (UserCrdsaPacketError, "user-crdsa-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (UserCrdsaPacketError, "user-crdsa-error")
SAT_STATS_PER_UT_METHOD_DEFINITION       (UserCrdsaPacketError, "user-crdsa-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (UserCrdsaPacketError, "user-crdsa-error")

// Random Access Feeder CRDSA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FeederCrdsaPacketCollision, "feeder-crdsa-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FeederCrdsaPacketCollision, "feeder-crdsa-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FeederCrdsaPacketCollision, "feeder-crdsa-collision")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FeederCrdsaPacketCollision, "feeder-crdsa-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FeederCrdsaPacketCollision, "feeder-crdsa-collision")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FeederCrdsaPacketCollision, "feeder-crdsa-collision")

// Random Access User CRDSA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (UserCrdsaPacketCollision, "user-crdsa-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION       (UserCrdsaPacketCollision, "user-crdsa-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (UserCrdsaPacketCollision, "user-crdsa-collision")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (UserCrdsaPacketCollision, "user-crdsa-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION       (UserCrdsaPacketCollision, "user-crdsa-collision")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (UserCrdsaPacketCollision, "user-crdsa-collision")

// Random Access MARSALA packet correlation statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (MarsalaCorrelation, "marsala-correlation")
SAT_STATS_PER_GW_METHOD_DEFINITION       (MarsalaCorrelation, "marsala-correlation")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (MarsalaCorrelation, "marsala-correlation")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (MarsalaCorrelation, "marsala-correlation")
SAT_STATS_PER_UT_METHOD_DEFINITION       (MarsalaCorrelation, "marsala-correlation")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (MarsalaCorrelation, "marsala-correlation")

// Random Access E-SSA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FeederEssaPacketError, "feeder-essa-error")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FeederEssaPacketError, "feeder-essa-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FeederEssaPacketError, "feeder-essa-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION   (FeederEssaPacketError, "feeder-essa-error")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FeederEssaPacketError, "feeder-essa-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION     (FeederEssaPacketError, "feeder-essa-error")

// Random Access E-SSA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (UserEssaPacketError, "user-essa-error")
SAT_STATS_PER_GW_METHOD_DEFINITION      (UserEssaPacketError, "user-essa-error")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (UserEssaPacketError, "user-essa-error")
SAT_STATS_PER_GROUP_METHOD_DEFINITION   (UserEssaPacketError, "user-essa-error")
SAT_STATS_PER_UT_METHOD_DEFINITION      (UserEssaPacketError, "user-essa-error")
SAT_STATS_PER_SAT_METHOD_DEFINITION     (UserEssaPacketError, "user-essa-error")

// Random Access Feeder E-SSA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FeederEssaPacketCollision, "feeder-essa-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FeederEssaPacketCollision, "feeder-essa-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FeederEssaPacketCollision, "feeder-essa-collision")
SAT_STATS_PER_GROUP_METHOD_DEFINITION   (FeederEssaPacketCollision, "feeder-essa-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FeederEssaPacketCollision, "feeder-essa-collision")
SAT_STATS_PER_SAT_METHOD_DEFINITION     (FeederEssaPacketCollision, "feeder-essa-collision")

// Random Access User E-SSA packet collision rate statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (UserEssaPacketCollision, "user-essa-collision")
SAT_STATS_PER_GW_METHOD_DEFINITION      (UserEssaPacketCollision, "user-essa-collision")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (UserEssaPacketCollision, "user-essa-collision")
SAT_STATS_PER_GROUP_METHOD_DEFINITION   (UserEssaPacketCollision, "user-essa-collision")
SAT_STATS_PER_UT_METHOD_DEFINITION      (UserEssaPacketCollision, "user-essa-collision")
SAT_STATS_PER_SAT_METHOD_DEFINITION     (UserEssaPacketCollision, "user-essa-collision")

// Dedicated Access carrier ID statistics
SAT_STATS_GLOBAL_METHOD_DEFINITION       (CarrierId, "carrier-id")
SAT_STATS_PER_GW_METHOD_DEFINITION       (CarrierId, "carrier-id")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (CarrierId, "carrier-id")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (CarrierId, "carrier-id")
SAT_STATS_PER_UT_METHOD_DEFINITION       (CarrierId, "carrier-id")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (CarrierId, "carrier-id")

// Capacity request statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (CapacityRequest, "capacity-request")
SAT_STATS_PER_GW_METHOD_DEFINITION       (CapacityRequest, "capacity-request")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (CapacityRequest, "capacity-request")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (CapacityRequest, "capacity-request")
SAT_STATS_PER_UT_METHOD_DEFINITION       (CapacityRequest, "capacity-request")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (CapacityRequest, "capacity-request")
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RbdcRequest, "rbdc-request")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RbdcRequest, "rbdc-request")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RbdcRequest, "rbdc-request")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RbdcRequest, "rbdc-request")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RbdcRequest, "rbdc-request")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RbdcRequest, "rbdc-request")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RbdcRequest, "rbdc-request")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RbdcRequest, "rbdc-request")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RbdcRequest, "rbdc-request")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RbdcRequest, "rbdc-request")

// Resources granted statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (ResourcesGranted, "resources-granted")
SAT_STATS_PER_GW_METHOD_DEFINITION       (ResourcesGranted, "resources-granted")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (ResourcesGranted, "resources-granted")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (ResourcesGranted, "resources-granted")
SAT_STATS_PER_UT_METHOD_DEFINITION       (ResourcesGranted, "resources-granted")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (ResourcesGranted, "resources-granted")

// Backlogged request statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (BackloggedRequest, "backlogged-request")
SAT_STATS_PER_GW_METHOD_DEFINITION       (BackloggedRequest, "backlogged-request")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (BackloggedRequest, "backlogged-request")

// Frame load statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FrameSymbolLoad, "frame-symbol-load")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FrameSymbolLoad, "frame-symbol-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FrameSymbolLoad, "frame-symbol-load")
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FrameUserLoad, "frame-user-load")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FrameUserLoad, "frame-user-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FrameUserLoad, "frame-user-load")

// Waveform usage statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (WaveformUsage, "waveform-usage")
SAT_STATS_PER_GW_METHOD_DEFINITION       (WaveformUsage, "waveform-usage")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (WaveformUsage, "waveform-usage")

// Forward Feeder Link SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederLinkSinr, "fwd-feeder-link-sinr")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederLinkSinr, "fwd-feeder-link-sinr")

// Forward User Link SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserLinkSinr, "fwd-user-link-sinr")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserLinkSinr, "fwd-user-link-sinr")

// Return Feeder Link SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederLinkSinr, "rtn-feeder-link-sinr")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederLinkSinr, "rtn-feeder-link-sinr")

// Return User Link SINR statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserLinkSinr, "rtn-user-link-sinr")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserLinkSinr, "rtn-user-link-sinr")

// Forward Feeder Link RX power statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederLinkRxPower, "fwd-feeder-link-rx-power")

// Forward User Link RX power statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserLinkRxPower, "fwd-user-link-rx-power")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserLinkRxPower, "fwd-user-link-rx-power")

// Return Feeder Link RX power statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederLinkRxPower, "rtn-feeder-link-rx-power")

// Return User Link RX power statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserLinkRxPower, "rtn-user-link-rx-power")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserLinkRxPower, "rtn-user-link-rx-power")

// Forward Feeder Link MODCOD statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdFeederLinkModcod, "fwd-feeder-link-modcod")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdFeederLinkModcod, "fwd-feeder-link-modcod")

// Forward User Link MODCOD statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_PER_UT_METHOD_DEFINITION       (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (FwdUserLinkModcod, "fwd-user-link-modcod")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (FwdUserLinkModcod, "fwd-user-link-modcod")

// Return Feeder Link MODCOD statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnFeederLinkModcod, "rtn-feeder-link-modcod")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnFeederLinkModcod, "rtn-feeder-link-modcod")

// Return User Link MODCOD statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_PER_GW_METHOD_DEFINITION       (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_PER_UT_METHOD_DEFINITION       (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (RtnUserLinkModcod, "rtn-user-link-modcod")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (RtnUserLinkModcod, "rtn-user-link-modcod")

// Frame type usage statistics
SAT_STATS_GLOBAL_METHOD_DEFINITION       (FrameTypeUsage, "frame-type-usage")
SAT_STATS_PER_GW_METHOD_DEFINITION       (FrameTypeUsage, "frame-type-usage")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (FrameTypeUsage, "frame-type-usage")

// Beam service time statistics
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (BeamServiceTime, "beam-service-time")

// Antenna Gain statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION       (AntennaGain, "antenna-gain")
SAT_STATS_PER_GW_METHOD_DEFINITION       (AntennaGain, "antenna-gain")
SAT_STATS_PER_BEAM_METHOD_DEFINITION     (AntennaGain, "antenna-gain")
SAT_STATS_PER_GROUP_METHOD_DEFINITION    (AntennaGain, "antenna-gain")
SAT_STATS_PER_UT_METHOD_DEFINITION       (AntennaGain, "antenna-gain")
SAT_STATS_PER_SAT_METHOD_DEFINITION      (AntennaGain, "antenna-gain")
SAT_STATS_AVERAGE_BEAM_METHOD_DEFINITION (AntennaGain, "antenna-gain")
SAT_STATS_AVERAGE_GROUP_METHOD_DEFINITION(AntennaGain, "antenna-gain")
SAT_STATS_AVERAGE_UT_METHOD_DEFINITION   (AntennaGain, "antenna-gain")
SAT_STATS_AVERAGE_SAT_METHOD_DEFINITION  (AntennaGain, "antenna-gain")

// Fwd Link Scheduler symbol rate statistics
SAT_STATS_PER_SLICE_METHOD_DEFINITION   (FwdLinkSchedulerSymbolRate, "fwd-link-scheduler-symbol-rate")
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdLinkSchedulerSymbolRate, "fwd-link-scheduler-symbol-rate")


// Link Window load statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnFeederWindowLoad, "rtn-feeder-window-load")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnFeederWindowLoad, "rtn-feeder-window-load")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnFeederWindowLoad, "rtn-feeder-window-load")

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
