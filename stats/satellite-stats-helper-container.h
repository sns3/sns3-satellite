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

#ifndef SATELLITE_STATS_HELPER_CONTAINER_H
#define SATELLITE_STATS_HELPER_CONTAINER_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>

#include <list>
#include <set>

namespace ns3
{

/*
 * The macro definitions following this comment block are used to declare the
 * majority of methods in this class. Below is the list of the class methods
 * created using this C++ pre-processing approach.
 *
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppDelay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppDelay
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy]
 * LinkDelay
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkDelay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppJitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppJitter
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy]
 * LinkJitter
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] LinkJitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppPlt
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppPlt
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] Queue [Bytes, Packets]
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [RtnFeeder, FwdUser] Queue [Bytes,
 * Packets]
 * - AddAverage [Beam, Group, Ut, Sat] [RtnFeeder, FwdUser] Queue [Bytes, Packets]
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] SignallingLoad
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] CompositeSinr
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser, PerSat] [Fwd, Rtn] AppThroughput
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy]
 * Throughput
 * - AddAverage [Beam, Group, Ut, UtUser, Sat] [Fwd, Rtn] AppThroughput
 * - AddAverage [Beam, Group, Ut, Sat] [Fwd, Rtn] [Feeder, User] [Dev, Mac, Phy] Throughput
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Feeder, User] [FwdDa, RtnDa,
 * SlottedAloha, Crdsa, Essa] PacketError
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerSat] [Feeder, User] [SlottedAloha, Crdsa,
 * Essa] PacketCollision
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
 * - Add [Global, PerIsl] PacketDropRate
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION(id)                                             \
    void AddGlobal##id(SatStatsHelper::OutputType_t outputType);                                   \
    void AddPerGw##id(SatStatsHelper::OutputType_t outputType);                                    \
    void AddPerBeam##id(SatStatsHelper::OutputType_t outputType);

#define SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(id)                                              \
    void AddGlobal##id(SatStatsHelper::OutputType_t outputType);                                   \
    void AddPerGw##id(SatStatsHelper::OutputType_t outputType);                                    \
    void AddPerBeam##id(SatStatsHelper::OutputType_t outputType);                                  \
    void AddPerGroup##id(SatStatsHelper::OutputType_t outputType);                                 \
    void AddPerUt##id(SatStatsHelper::OutputType_t outputType);                                    \
    void AddPerSat##id(SatStatsHelper::OutputType_t outputType);

#define SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(id)                                                \
    void AddGlobal##id(SatStatsHelper::OutputType_t outputType);                                   \
    void AddPerGw##id(SatStatsHelper::OutputType_t outputType);                                    \
    void AddPerBeam##id(SatStatsHelper::OutputType_t outputType);                                  \
    void AddPerGroup##id(SatStatsHelper::OutputType_t outputType);                                 \
    void AddPerUt##id(SatStatsHelper::OutputType_t outputType);                                    \
    void AddPerUtUser##id(SatStatsHelper::OutputType_t outputType);                                \
    void AddPerSat##id(SatStatsHelper::OutputType_t outputType);

class SatHelper;

/**
 * \ingroup satstats
 * \brief Container of SatStatsHelper instances.
 *
 * The container is initially empty upon creation. SatStatsHelper instances can
 * be added into the container using attributes or class methods.
 *
 * The names of these attributes and class methods follow the convention below:
 * - identifier (e.g., per UT user, per UT, per beam, per GW, etc.);
 * - direction (forward link or return link, if relevant); and
 * - name of statistics.
 *
 * The value of the attributes and the arguments of the class methods are the
 * desired output type (e.g., scalar, scatter, histogram, files, plots, etc.).
 *
 * The output files will be named in a certain pattern using the name set in
 * the `Name` attribute or SetName() method. The default name is "stat", e.g.,
 * which will produce output files with the names such as
 * `stat-per-ut-fwd-app-delay-scalar.txt`,
 * `stat-per-ut-fwd-app-delay-cdf-ut-1.txt`, etc.
 */
class SatStatsHelperContainer : public Object
{
  public:
    /**
     * \brief Creates a new instance of container.
     * \param satHelper the satellite module helper which would be used to learn
     *                  the topology of the simulation.
     */
    SatStatsHelperContainer(Ptr<SatHelper> satHelper);

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \param name a string prefix to be prepended on every output file name.
     */
    void SetName(std::string name);

    /**
     * \return a string prefix prepended on every output file name.
     */
    std::string GetName() const;

    // Forward link application-level packet delay statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(FwdAppDelay)
    void AddAverageBeamFwdAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserFwdAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdAppDelay(SatStatsHelper::OutputType_t outputType);

    // Forward link device-level packet delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdDevDelay)
    void AddAverageBeamFwdDevDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdDevDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdDevDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdDevDelay(SatStatsHelper::OutputType_t outputType);

    // Forward link MAC-level packet delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdMacDelay)
    void AddAverageBeamFwdMacDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdMacDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdMacDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdMacDelay(SatStatsHelper::OutputType_t outputType);

    // Forward link PHY-level packet delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdPhyDelay)
    void AddAverageBeamFwdPhyDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdPhyDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdPhyDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdPhyDelay(SatStatsHelper::OutputType_t outputType);

    // Forward feeder link DEV-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederDevLinkDelay)
    void AddAverageBeamFwdFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Forward user link DEV-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserDevLinkDelay)
    void AddAverageBeamFwdUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Forward feeder link MAC-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederMacLinkDelay)
    void AddAverageBeamFwdFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Forward user link MAC-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserMacLinkDelay)
    void AddAverageBeamFwdUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Forward feeder link PHY-level  packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederPhyLinkDelay)
    void AddAverageBeamFwdFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Forward user link PHY-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserPhyLinkDelay)
    void AddAverageBeamFwdUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Forward link application-level packet jitter statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(FwdAppJitter)
    void AddAverageBeamFwdAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserFwdAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdAppJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link device-level packet Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdDevJitter)
    void AddAverageBeamFwdDevJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdDevJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdDevJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdDevJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link MAC-level packet Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdMacJitter)
    void AddAverageBeamFwdMacJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdMacJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdMacJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdMacJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link PHY-level packet Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdPhyJitter)
    void AddAverageBeamFwdPhyJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdPhyJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdPhyJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdPhyJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link feeder DEV-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederDevLinkJitter)
    void AddAverageBeamFwdFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link user DEV-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserDevLinkJitter)
    void AddAverageBeamFwdUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link feeder MAC-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederMacLinkJitter)
    void AddAverageBeamFwdFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link user MAC-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserMacLinkJitter)
    void AddAverageBeamFwdUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link feeder PHY-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederPhyLinkJitter)
    void AddAverageBeamFwdFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link user PHY-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserPhyLinkJitter)
    void AddAverageBeamFwdUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Forward link application-level packet PLT statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(FwdAppPlt)
    void AddAverageBeamFwdAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserFwdAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdAppPlt(SatStatsHelper::OutputType_t outputType);

    // Forward link queue size (in bytes) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdQueueBytes)

    // Forward link queue size (in number of packets) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdQueuePackets)

    // Forward link signalling load statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdSignallingLoad)

    // Forward link composite SINR statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdCompositeSinr)

    // Forward link application-level throughput statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(FwdAppThroughput)
    void AddAverageBeamFwdAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserFwdAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdAppThroughput(SatStatsHelper::OutputType_t outputType);

    // Forward feeder link device-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederDevThroughput)
    void AddAverageBeamFwdFeederDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederDevThroughput(SatStatsHelper::OutputType_t outputType);

    // Forward user link device-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserDevThroughput)
    void AddAverageBeamFwdUserDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserDevThroughput(SatStatsHelper::OutputType_t outputType);

    // Forward feeder link MAC-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederMacThroughput)
    void AddAverageBeamFwdFeederMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederMacThroughput(SatStatsHelper::OutputType_t outputType);

    // Forward user link MAC-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserMacThroughput)
    void AddAverageBeamFwdUserMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserMacThroughput(SatStatsHelper::OutputType_t outputType);

    // Forward feeder link PHY-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederPhyThroughput)
    void AddAverageBeamFwdFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);

    // Forward user link PHY-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserPhyThroughput)
    void AddAverageBeamFwdUserPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserPhyThroughput(SatStatsHelper::OutputType_t outputType);

    // Return link application-level packet delay statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(RtnAppDelay)
    void AddAverageBeamRtnAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserRtnAppDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnAppDelay(SatStatsHelper::OutputType_t outputType);

    // Return link device-level packet delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnDevDelay)
    void AddAverageBeamRtnDevDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnDevDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnDevDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnDevDelay(SatStatsHelper::OutputType_t outputType);

    // Return link MAC-level packet delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnMacDelay)
    void AddAverageBeamRtnMacDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnMacDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnMacDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnMacDelay(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnPhyDelay)
    void AddAverageBeamRtnPhyDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnPhyDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnPhyDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnPhyDelay(SatStatsHelper::OutputType_t outputType);

    // Return feeder link DEV-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederDevLinkDelay)
    void AddAverageBeamRtnFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederDevLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Return user link DEV-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserDevLinkDelay)
    void AddAverageBeamRtnUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserDevLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Return feeder link MAC-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederMacLinkDelay)
    void AddAverageBeamRtnFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederMacLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Return user link MAC-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserMacLinkDelay)
    void AddAverageBeamRtnUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserMacLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Return feeder link PHY-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederPhyLinkDelay)
    void AddAverageBeamRtnFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederPhyLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Return user link PHY-level packet link delay statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserPhyLinkDelay)
    void AddAverageBeamRtnUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserPhyLinkDelay(SatStatsHelper::OutputType_t outputType);

    // Return link application-level packet jitter statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(RtnAppJitter)
    void AddAverageBeamRtnAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserRtnAppJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnAppJitter(SatStatsHelper::OutputType_t outputType);

    // Return link device-level packet jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnDevJitter)
    void AddAverageBeamRtnDevJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnDevJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnDevJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnDevJitter(SatStatsHelper::OutputType_t outputType);

    // Return link MAC-level packet jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnMacJitter)
    void AddAverageBeamRtnMacJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnMacJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnMacJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnMacJitter(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnPhyJitter)
    void AddAverageBeamRtnPhyJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnPhyJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnPhyJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnPhyJitter(SatStatsHelper::OutputType_t outputType);

    // Return link feeder DEV-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederDevLinkJitter)
    void AddAverageBeamRtnFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederDevLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Return link user DEV-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserDevLinkJitter)
    void AddAverageBeamRtnUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserDevLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Return link feeder MAC-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederMacLinkJitter)
    void AddAverageBeamRtnFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederMacLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Return link user MAC-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserMacLinkJitter)
    void AddAverageBeamRtnUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserMacLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Return link feeder PHY-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederPhyLinkJitter)
    void AddAverageBeamRtnFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederPhyLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Return link user PHY-level packet link Jitter statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserPhyLinkJitter)
    void AddAverageBeamRtnUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserPhyLinkJitter(SatStatsHelper::OutputType_t outputType);

    // Return link application-level packet PLT statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(RtnAppPlt)
    void AddAverageBeamRtnAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserRtnAppPlt(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnAppPlt(SatStatsHelper::OutputType_t outputType);

    // Return link queue size (in bytes) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnQueueBytes)

    // Return link queue size (in number of packets) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnQueuePackets)

    // Return feeder link queue size (in bytes) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederQueueBytes)
    void AddAverageBeamRtnFeederQueueBytes(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederQueueBytes(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederQueueBytes(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederQueueBytes(SatStatsHelper::OutputType_t outputType);

    // Return feeder link queue size (in number of packets) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederQueuePackets)
    void AddAverageBeamRtnFeederQueuePackets(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederQueuePackets(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederQueuePackets(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederQueuePackets(SatStatsHelper::OutputType_t outputType);

    // Forward user link queue size (in bytes) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserQueueBytes)
    void AddAverageBeamFwdUserQueueBytes(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserQueueBytes(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserQueueBytes(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserQueueBytes(SatStatsHelper::OutputType_t outputType);

    // Forward user link queue size (in number of packets) statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserQueuePackets)
    void AddAverageBeamFwdUserQueuePackets(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserQueuePackets(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserQueuePackets(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserQueuePackets(SatStatsHelper::OutputType_t outputType);

    // Return link signalling load statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnSignallingLoad)

    // Return link composite SINR statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnCompositeSinr)

    // Return link application-level throughput statistics.
    SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(RtnAppThroughput)
    void AddAverageBeamRtnAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtUserRtnAppThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnAppThroughput(SatStatsHelper::OutputType_t outputType);

    // Return feeder link device-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederDevThroughput)
    void AddAverageBeamRtnFeederDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederDevThroughput(SatStatsHelper::OutputType_t outputType);

    // Return user link device-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserDevThroughput)
    void AddAverageBeamRtnUserDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserDevThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserDevThroughput(SatStatsHelper::OutputType_t outputType);

    // Return feeder link MAC-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederMacThroughput)
    void AddAverageBeamRtnFeederMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederMacThroughput(SatStatsHelper::OutputType_t outputType);

    // Return user link MAC-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserMacThroughput)
    void AddAverageBeamRtnUserMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserMacThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserMacThroughput(SatStatsHelper::OutputType_t outputType);

    // Return feeder link PHY-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederPhyThroughput)
    void AddAverageBeamRtnFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederPhyThroughput(SatStatsHelper::OutputType_t outputType);

    // Return user link PHY-level throughput statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserPhyThroughput)
    void AddAverageBeamRtnUserPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserPhyThroughput(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserPhyThroughput(SatStatsHelper::OutputType_t outputType);

    // Forward link Dedicated Access packet error rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederDaPacketError)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserDaPacketError)

    // Return link Dedicated Access packet error rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederDaPacketError)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserDaPacketError)

    // Random Access Slotted ALOHA packet error rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FeederSlottedAlohaPacketError)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(UserSlottedAlohaPacketError)

    // Random Access Slotted ALOHA packet collision rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FeederSlottedAlohaPacketCollision)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(UserSlottedAlohaPacketCollision)

    // Random Access CRDSA packet error rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FeederCrdsaPacketError)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(UserCrdsaPacketError)

    // Random Access CRDSA packet collision rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FeederCrdsaPacketCollision)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(UserCrdsaPacketCollision)

    // Random Access Marsala packet correlation rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(MarsalaCorrelation)

    // Random Access E-SSA packet error rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FeederEssaPacketError)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(UserEssaPacketError)

    // Random Access E-SSA packet collision rate statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FeederEssaPacketCollision)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(UserEssaPacketCollision)

    // Dedicated Access carrier ID statistics
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(CarrierId)

    // Capacity request statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(CapacityRequest)
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RbdcRequest)
    void AddAverageBeamRbdcRequest(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRbdcRequest(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRbdcRequest(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRbdcRequest(SatStatsHelper::OutputType_t outputType);

    // Resources granted statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(ResourcesGranted)

    // Backlogged request statistics.
    SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION(BackloggedRequest)

    // Frame load statistics.
    SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION(FrameSymbolLoad)
    SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION(FrameUserLoad)

    // Waveform usage statistics.
    SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION(WaveformUsage)

    // Return link PHY-level packet link sinr statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederLinkSinr)
    void AddAverageBeamFwdFeederLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederLinkSinr(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link sinr statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserLinkSinr)
    void AddAverageBeamFwdUserLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserLinkSinr(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link sinr statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederLinkSinr)
    void AddAverageBeamRtnFeederLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederLinkSinr(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link sinr statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserLinkSinr)
    void AddAverageBeamRtnUserLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserLinkSinr(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserLinkSinr(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link RX power statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederLinkRxPower)
    void AddAverageBeamFwdFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link RX power statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserLinkRxPower)
    void AddAverageBeamFwdUserLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserLinkRxPower(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link RX power statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederLinkRxPower)
    void AddAverageBeamRtnFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederLinkRxPower(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link RX power statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserLinkRxPower)
    void AddAverageBeamRtnUserLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserLinkRxPower(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserLinkRxPower(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link MODCOD statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdFeederLinkModcod)
    void AddAverageBeamFwdFeederLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdFeederLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdFeederLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdFeederLinkModcod(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link MODCOD statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(FwdUserLinkModcod)
    void AddAverageBeamFwdUserLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupFwdUserLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtFwdUserLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatFwdUserLinkModcod(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link MODCOD statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnFeederLinkModcod)
    void AddAverageBeamRtnFeederLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnFeederLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnFeederLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnFeederLinkModcod(SatStatsHelper::OutputType_t outputType);

    // Return link PHY-level packet link MODCOD statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(RtnUserLinkModcod)
    void AddAverageBeamRtnUserLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupRtnUserLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtRtnUserLinkModcod(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatRtnUserLinkModcod(SatStatsHelper::OutputType_t outputType);

    // Frame type usage statistics.
    SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION(FrameTypeUsage)

    // Beam service time statistics
    void AddPerBeamBeamServiceTime(SatStatsHelper::OutputType_t outputType);

    // Antenna Gain statistics.
    SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(AntennaGain)
    void AddAverageBeamAntennaGain(SatStatsHelper::OutputType_t outputType);
    void AddAverageGroupAntennaGain(SatStatsHelper::OutputType_t outputType);
    void AddAverageUtAntennaGain(SatStatsHelper::OutputType_t outputType);
    void AddAverageSatAntennaGain(SatStatsHelper::OutputType_t outputType);

    // Fwd Link Scheduler SymbolRate statistics.
    void AddPerSliceFwdLinkSchedulerSymbolRate(SatStatsHelper::OutputType_t outputType);
    void AddGlobalFwdLinkSchedulerSymbolRate(SatStatsHelper::OutputType_t outputType);

    // Window load statistics
    void AddGlobalRtnFeederWindowLoad(SatStatsHelper::OutputType_t outputType);
    void AddPerGwRtnFeederWindowLoad(SatStatsHelper::OutputType_t outputType);
    void AddPerBeamRtnFeederWindowLoad(SatStatsHelper::OutputType_t outputType);

    // ISL queue drop statistics
    void AddGlobalPacketDropRate(SatStatsHelper::OutputType_t outputType);
    void AddPerIslPacketDropRate(SatStatsHelper::OutputType_t outputType);

    /**
     * \param outputType an arbitrary output type.
     * \return a string suffix to be appended at the end of the corresponding
     *         output file for this output type.
     */
    static std::string GetOutputTypeSuffix(SatStatsHelper::OutputType_t outputType);

    /**
     * \brief Update the address and the proper identifier from the given UT node.
     * \param utNode a UT node.
     *
     * Used for UT handovers. Do it for all statistic instances.
     */
    void UpdateAddressAndIdentifier(Ptr<Node> utNode);

  protected:
    /**
     * Inherited from Object base class
     */
    virtual void DoDispose();

  private:
    /// Satellite module helper for reference.
    Ptr<SatHelper> m_satHelper;

    /// Prefix of every SatStatsHelper instance names and every output file.
    std::string m_name;

    /// Maintains the active SatStatsHelper instances which have created.
    std::list<Ptr<SatStatsHelper>> m_stats;

    /// Set of names of statisticscreated. Used to avoid creating same stat twice.
    std::set<std::string> m_names;

}; // end of class StatStatsHelperContainer

} // end of namespace ns3

#endif /* SATELLITE_STATS_HELPER_CONTAINER_H */
