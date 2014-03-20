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

#ifndef SATELLITE_STATS_FWD_MAC_THROUGHPUT_HELPER_H
#define SATELLITE_STATS_FWD_MAC_THROUGHPUT_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/ptr.h>
#include <ns3/collector-map.h>
#include <list>


namespace ns3 {

class SatHelper;
class Probe;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief Produce forward link MAC-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdMacThroughputHelper> s = Create<SatStatsFwdMacThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdMacThroughputHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdMacThroughputHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsFwdMacThroughputHelper ();

protected:
  // inherited from SatStatsHelper base class
  virtual void DoInstall ();

private:
  /**
   * \brief Create a probe for each UT satellite device and connect it
   *        to a collector.
   * \param collectorMap list of collectors which the created probes will be
   *                     connected to.
   * \param collectorTraceSink a pointer to a function of the collector which
   *                           acts as a trace sink.
   *
   * The collector's trace sink function must be an accessible (e.g., public)
   * class method which accepts two input arguments of same type and returns
   * nothing. For example, it is specified as
   * `&IntervalRateCollector::TraceSinkDouble`.
   *
   * The created probes will be added to #m_probes.
   */
  template<typename R, typename C, typename P>
  void InstallProbes (CollectorMap &collectorMap,
                      R (C::*collectorTraceSink) (P, P));

  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

  /// Maintains a list of first-level collectors created by this helper.
  CollectorMap m_conversionCollectors;

  /// Maintains a list of second-level collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

}; // end of class SatStatsFwdMacThroughputHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_FWD_MAC_THROUGHPUT_HELPER_H */
