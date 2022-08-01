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


namespace ns3 {

/*
 * The macro definitions following this comment block are used to declare the
 * majority of methods in this class. Below is the list of the class methods
 * created using this C++ pre-processing approach.
 *
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser] [Fwd, Rtn] AppDelay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - AddAverage [Beam, Group, Ut, UtUser] [Fwd, Rtn] AppDelay
 * - AddAverage [Beam, Group, Ut] [Fwd, Rtn] [Dev, Mac, Phy] Delay
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser] [Fwd, Rtn] AppJitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - AddAverage [Beam, Group, Ut, UtUser] [Fwd, Rtn] AppJitter
 * - AddAverage [Beam, Group, Ut] [Fwd, Rtn] [Dev, Mac, Phy] Jitter
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser] [Fwd, Rtn] AppPlt
 * - AddAverage [Beam, Group, Ut, UtUser] [Fwd, Rtn] AppPlt
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [Fwd, Rtn] Queue [Bytes, Packets]
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [Fwd, Rtn] SignallingLoad
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [Fwd, Rtn] CompositeSinr
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt, PerUtUser] [Fwd, Rtn] AppThroughput
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [Fwd, Rtn] [Dev, Mac, Phy] Throughput
 * - AddAverage [Beam, Group, Ut, UtUser] [Fwd, Rtn] AppThroughput
 * - AddAverage [Beam, Group, Ut] [Fwd, Rtn] [Dev, Mac, Phy] Throughput
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [FwdDa, RtnDa, SlottedAloha, Crdsa, Essa] PacketError
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] [SlottedAloha, Crdsa, Essa] PacketCollision
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] CapacityRequest
 * - Add [Global, PerGw, PerBeam, PerGroup, PerUt] ResourcesGranted
 * - Add [Global, PerGw, PerBeam] BackloggedRequest
 * - Add [Global, PerGw, PerBeam] Frame [Symbol, User] Load
 * - Add [Global, PerGw, PerBeam] WaveformUsage
 * - Add [Global, PerSlice] FwdLinkSchedulerSymbolRate
 * - AddGlobal [Fwd, Rtn] [Feeder, User] LinkSinr
 * - AddGlobal [Fwd, Rtn] [Feeder, User] LinkRxPower
 * - Add [Global, PerGw, PerBeam] FrameTypeUsage
 * - Add [Global, PerGw, PerBeam] RtnFeederWindowLoad
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION(id)                        \
  void AddGlobal ## id (SatStatsHelper::OutputType_t outputType);             \
  void AddPerGw ## id (SatStatsHelper::OutputType_t outputType);              \
  void AddPerBeam ## id (SatStatsHelper::OutputType_t outputType);

#define SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION(id)                         \
  void AddGlobal ## id (SatStatsHelper::OutputType_t outputType);             \
  void AddPerGw ## id (SatStatsHelper::OutputType_t outputType);              \
  void AddPerBeam ## id (SatStatsHelper::OutputType_t outputType);            \
  void AddPerGroup ## id (SatStatsHelper::OutputType_t outputType);           \
  void AddPerUt ## id (SatStatsHelper::OutputType_t outputType);              \
  void AddPerSat ## id (SatStatsHelper::OutputType_t outputType);

#define SAT_STATS_FULL_SCOPE_METHOD_DECLARATION(id)                           \
  void AddGlobal ## id (SatStatsHelper::OutputType_t outputType);             \
  void AddPerGw ## id (SatStatsHelper::OutputType_t outputType);              \
  void AddPerBeam ## id (SatStatsHelper::OutputType_t outputType);            \
  void AddPerGroup ## id (SatStatsHelper::OutputType_t outputType);           \
  void AddPerUt ## id (SatStatsHelper::OutputType_t outputType);              \
  void AddPerUtUser ## id (SatStatsHelper::OutputType_t outputType);          \
  void AddPerSat ## id (SatStatsHelper::OutputType_t outputType);

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
  SatStatsHelperContainer (Ptr<const SatHelper> satHelper);

  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \param name a string prefix to be prepended on every output file name.
   */
  void SetName (std::string name);

  /**
   * \return a string prefix prepended on every output file name.
   */
  std::string GetName () const;

  // Forward link application-level packet delay statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (FwdAppDelay)
  void AddAverageBeamFwdAppDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdAppDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdAppDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserFwdAppDelay (SatStatsHelper::OutputType_t outputType);

  // Forward link device-level packet delay statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdDevDelay)
  void AddAverageBeamFwdDevDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdDevDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdDevDelay (SatStatsHelper::OutputType_t outputType);

  // Forward link MAC-level packet delay statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdMacDelay)
  void AddAverageBeamFwdMacDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdMacDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdMacDelay (SatStatsHelper::OutputType_t outputType);

  // Forward link PHY-level packet delay statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdPhyDelay)
  void AddAverageBeamFwdPhyDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdPhyDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdPhyDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageSatFwdPhyDelay (SatStatsHelper::OutputType_t outputType);

  // Forward link application-level packet jitter statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (FwdAppJitter)
  void AddAverageBeamFwdAppJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdAppJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdAppJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserFwdAppJitter (SatStatsHelper::OutputType_t outputType);

  // Forward link device-level packet Jitter statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdDevJitter)
  void AddAverageBeamFwdDevJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdDevJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdDevJitter (SatStatsHelper::OutputType_t outputType);

  // Forward link MAC-level packet Jitter statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdMacJitter)
  void AddAverageBeamFwdMacJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdMacJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdMacJitter (SatStatsHelper::OutputType_t outputType);

  // Forward link PHY-level packet Jitter statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdPhyJitter)
  void AddAverageBeamFwdPhyJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdPhyJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdPhyJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageSatFwdPhyJitter (SatStatsHelper::OutputType_t outputType);

  // Forward link application-level packet PLT statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (FwdAppPlt)
  void AddAverageBeamFwdAppPlt (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdAppPlt (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdAppPlt (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserFwdAppPlt (SatStatsHelper::OutputType_t outputType);

  // Forward link queue size (in bytes) statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdQueueBytes)

  // Forward link queue size (in number of packets) statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdQueuePackets)

  // Forward link signalling load statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdSignallingLoad)

  // Forward link composite SINR statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdCompositeSinr)

  // Forward link application-level throughput statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (FwdAppThroughput)
  void AddAverageBeamFwdAppThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdAppThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdAppThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserFwdAppThroughput (SatStatsHelper::OutputType_t outputType);

  // Forward link device-level throughput statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdDevThroughput)
  void AddAverageBeamFwdDevThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdDevThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdDevThroughput (SatStatsHelper::OutputType_t outputType);

  // Forward link MAC-level throughput statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdMacThroughput)
  void AddAverageBeamFwdMacThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdMacThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdMacThroughput (SatStatsHelper::OutputType_t outputType);

  // Forward link PHY-level throughput statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdPhyThroughput)
  void AddAverageBeamFwdPhyThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupFwdPhyThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtFwdPhyThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageSatFwdPhyThroughput (SatStatsHelper::OutputType_t outputType);

  // Return link application-level packet delay statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (RtnAppDelay)
  void AddAverageBeamRtnAppDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnAppDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnAppDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserRtnAppDelay (SatStatsHelper::OutputType_t outputType);

  // Return link device-level packet delay statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnDevDelay)
  void AddAverageBeamRtnDevDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnDevDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnDevDelay (SatStatsHelper::OutputType_t outputType);

  // Return link MAC-level packet delay statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnMacDelay)
  void AddAverageBeamRtnMacDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnMacDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnMacDelay (SatStatsHelper::OutputType_t outputType);

  // Return link PHY-level packet delay statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnPhyDelay)
  void AddAverageBeamRtnPhyDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnPhyDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnPhyDelay (SatStatsHelper::OutputType_t outputType);
  void AddAverageSatRtnPhyDelay (SatStatsHelper::OutputType_t outputType);

  // Return link application-level packet jitter statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (RtnAppJitter)
  void AddAverageBeamRtnAppJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnAppJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnAppJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserRtnAppJitter (SatStatsHelper::OutputType_t outputType);

  // Return link device-level packet jitter statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnDevJitter)
  void AddAverageBeamRtnDevJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnDevJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnDevJitter (SatStatsHelper::OutputType_t outputType);

  // Return link MAC-level packet jitter statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnMacJitter)
  void AddAverageBeamRtnMacJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnMacJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnMacJitter (SatStatsHelper::OutputType_t outputType);

  // Return link PHY-level packet jitter statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnPhyJitter)
  void AddAverageBeamRtnPhyJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnPhyJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnPhyJitter (SatStatsHelper::OutputType_t outputType);
  void AddAverageSatRtnPhyJitter (SatStatsHelper::OutputType_t outputType);

  // Return link application-level packet PLT statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (RtnAppPlt)
  void AddAverageBeamRtnAppPlt (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnAppPlt (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnAppPlt (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserRtnAppPlt (SatStatsHelper::OutputType_t outputType);

  // Return link queue size (in bytes) statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnQueueBytes)

  // Return link queue size (in number of packets) statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnQueuePackets)

  // Return link signalling load statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnSignallingLoad)

  // Return link composite SINR statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnCompositeSinr)

  // Return link application-level throughput statistics.
  SAT_STATS_FULL_SCOPE_METHOD_DECLARATION (RtnAppThroughput)
  void AddAverageBeamRtnAppThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnAppThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnAppThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtUserRtnAppThroughput (SatStatsHelper::OutputType_t outputType);

  // Return link device-level throughput statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnDevThroughput)
  void AddAverageBeamRtnDevThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnDevThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnDevThroughput (SatStatsHelper::OutputType_t outputType);

  // Return link MAC-level throughput statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnMacThroughput)
  void AddAverageBeamRtnMacThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnMacThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnMacThroughput (SatStatsHelper::OutputType_t outputType);

  // Return link PHY-level throughput statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnPhyThroughput)
  void AddAverageBeamRtnPhyThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRtnPhyThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRtnPhyThroughput (SatStatsHelper::OutputType_t outputType);
  void AddAverageSatRtnPhyThroughput (SatStatsHelper::OutputType_t outputType);

  // Forward link Dedicated Access packet error rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (FwdDaPacketError)

  // Return link Dedicated Access packet error rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RtnDaPacketError)

  // Random Access Slotted ALOHA packet error rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (SlottedAlohaPacketError)

  // Random Access Slotted ALOHA packet collision rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (SlottedAlohaPacketCollision)

  // Random Access CRDSA packet error rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (CrdsaPacketError)

  // Random Access CRDSA packet collision rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (CrdsaPacketCollision)

  // Random Access Marsala packet collision rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (MarsalaCorrelation)

  // Random Access E-SSA packet error rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (EssaPacketError)

  // Random Access E-SSA packet collision rate statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (EssaPacketCollision)

  // Dedicated Access carrier ID statistics
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (CarrierId)

  // Capacity request statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (CapacityRequest)
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (RbdcRequest)
  void AddAverageBeamRbdcRequest (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupRbdcRequest (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtRbdcRequest (SatStatsHelper::OutputType_t outputType);

  // Resources granted statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (ResourcesGranted)

  // Backlogged request statistics.
  SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION (BackloggedRequest)

  // Frame load statistics.
  SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION (FrameSymbolLoad)
  SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION (FrameUserLoad)

  // Waveform usage statistics.
  SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION (WaveformUsage)

  // Link SINR statistics.
  void AddGlobalFwdFeederLinkSinr (SatStatsHelper::OutputType_t outputType);
  void AddGlobalFwdUserLinkSinr (SatStatsHelper::OutputType_t outputType);
  void AddGlobalRtnFeederLinkSinr (SatStatsHelper::OutputType_t outputType);
  void AddGlobalRtnUserLinkSinr (SatStatsHelper::OutputType_t outputType);

  // Link Rx power statistics.
  void AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OutputType_t outputType);
  void AddGlobalFwdUserLinkRxPower (SatStatsHelper::OutputType_t outputType);
  void AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OutputType_t outputType);
  void AddGlobalRtnUserLinkRxPower (SatStatsHelper::OutputType_t outputType);

  // Frame type usage statistics.
  SAT_STATS_REDUCED_SCOPE_METHOD_DECLARATION (FrameTypeUsage)

  // Beam service time statistics
  void AddPerBeamBeamServiceTime (SatStatsHelper::OutputType_t outputType);

  // Antenna Gain statistics.
  SAT_STATS_NORMAL_SCOPE_METHOD_DECLARATION (AntennaGain)
  void AddAverageBeamAntennaGain (SatStatsHelper::OutputType_t outputType);
  void AddAverageGroupAntennaGain (SatStatsHelper::OutputType_t outputType);
  void AddAverageUtAntennaGain (SatStatsHelper::OutputType_t outputType);

  // Fwd Link Scheduler SymbolRate statistics.
  void AddPerSliceFwdLinkSchedulerSymbolRate (SatStatsHelper::OutputType_t outputType);
  void AddGlobalFwdLinkSchedulerSymbolRate (SatStatsHelper::OutputType_t outputType);

  // Window load statistics
  void AddGlobalRtnFeederWindowLoad (SatStatsHelper::OutputType_t outputType);
  void AddPerGwRtnFeederWindowLoad (SatStatsHelper::OutputType_t outputType);
  void AddPerBeamRtnFeederWindowLoad (SatStatsHelper::OutputType_t outputType);

  /**
   * \param outputType an arbitrary output type.
   * \return a string suffix to be appended at the end of the corresponding
   *         output file for this output type.
   */
  static std::string GetOutputTypeSuffix (SatStatsHelper::OutputType_t outputType);

protected:
  /**
   * Inherited from Object base class
   */
  virtual void DoDispose ();

private:
  /// Satellite module helper for reference.
  Ptr<const SatHelper> m_satHelper;

  /// Prefix of every SatStatsHelper instance names and every output file.
  std::string m_name;

  /// Maintains the active SatStatsHelper instances which have created.
  std::list<Ptr<const SatStatsHelper> > m_stats;

}; // end of class StatStatsHelperContainer


} // end of namespace ns3


#endif /* SATELLITE_STATS_HELPER_CONTAINER_H */
