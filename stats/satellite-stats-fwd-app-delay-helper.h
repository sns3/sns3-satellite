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

#ifndef SATELLITE_STATS_FWD_APP_DELAY_HELPER_H
#define SATELLITE_STATS_FWD_APP_DELAY_HELPER_H

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
class SatStatsFwdAppDelayHelper : public SatStatsHelper
{
public:
  /**
   * \brief
   * \param satHelper
   */
  SatStatsFwdAppDelayHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsFwdAppDelayHelper ();

protected:
  // inherited from SatStatsHelper base class
  virtual void DoInstall ();

private:
  /**
   * \brief Create a probe for each UT user node's application and connect it
   *        to a collector.
   * \param collectorTraceSink
   *
   * Add the created probes to #m_probes.
   */
  template<typename R, typename C, typename P1, typename P2>
  void InstallProbes (CollectorMap_t &collectorMap,
                      R (C::*collectorTraceSink) (P1, P2));

  ///
  std::list<Ptr<Probe> > m_probes;

  // key: identifier ID
  SatStatsHelper::CollectorMap_t m_terminalCollectors;

  ///
  Ptr<DataCollectionObject> m_aggregator;

}; // end of class SatStatsFwdAppDelayHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_FWD_APP_DELAY_HELPER_H */
