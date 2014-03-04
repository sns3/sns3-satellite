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

#ifndef SATELLITE_STATS_FWD_THROUGHPUT_HELPER_H
#define SATELLITE_STATS_FWD_THROUGHPUT_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/ptr.h>
#include <list>


namespace ns3 {

class SatHelper;
class Probe;
class DataCollectionObject;

/**
 * \brief
 */
class SatStatsFwdThroughputHelper : public SatStatsHelper
{
public:
  /**
   * \brief
   * \param satHelper
   */
  SatStatsFwdThroughputHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsFwdThroughputHelper ();

protected:
  // inherited from SatStatsHelper base class
  virtual void DoInstall ();

private:
  ///
  std::list<Ptr<Probe> > m_probes;

  // key: identifier ID
  SatStatsHelper::CollectorMap_t m_terminalCollectors;

  ///
  Ptr<DataCollectionObject> m_aggregator;

  /**
   * \brief Create a probe for each user node's application and connect it to
   *        a collector.
   * \param userNodes
   * \param collectorMap
   * \param collectorTraceSink
   *
   * Add the created probes to #m_probes.
   */
  template<typename R, typename C, typename P1, typename P2>
  void InstallProbes (NodeContainer userNodes,
                      CollectorMap_t &collectorMap,
                      R (C::*collectorTraceSink) (P1, P2));

  /**
   * \brief Connect the collectors to the aggregator.
   * \param collectorMap
   * \param collectorTraceSourceName
   * \param aggregator
   * \param aggregatorTraceSink
   */
  template<typename R, typename C, typename P1, typename V1>
  void ConnectCollectorsToAggregator (CollectorMap_t &collectorMap,
                                      std::string collectorTraceSourceName,
                                      Ptr<DataCollectionObject> aggregator,
                                      R (C::*aggregatorTraceSink) (P1, V1)) const;

  /**
   * \brief Connect the collectors to the aggregator.
   * \param collectorMap
   * \param collectorTraceSourceName
   * \param aggregator
   * \param aggregatorTraceSink
   */
  template<typename R, typename C, typename P1, typename V1, typename V2>
  void ConnectCollectorsToAggregator (CollectorMap_t &collectorMap,
                                      std::string collectorTraceSourceName,
                                      Ptr<DataCollectionObject> aggregator,
                                      R (C::*aggregatorTraceSink) (P1, V1, V2)) const;

}; // end of class SatStatsFwdThroughputHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_FWD_THROUGHPUT_HELPER_H */
