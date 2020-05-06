/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#ifndef SATELLITE_STATS_FWD_LINK_SCHEDULER_SYMBOL_RATE_HELPER_H
#define SATELLITE_STATS_FWD_LINK_SCHEDULER_SYMBOL_RATE_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/satellite-enums.h>
#include <ns3/ptr.h>
#include <ns3/callback.h>
#include <ns3/collector-map.h>


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class Time;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief Helper for forward link scheduler symbol rate statistic. Base class.
 */
class SatStatsFwdLinkSchedulerSymbolRateHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdLinkSchedulerSymbolRateHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdLinkSchedulerSymbolRateHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \brief Set up several probes or other means of listeners and connect them
   *        to the collectors.
   */
  void InstallProbes ();

  /**
   * \brief Receive inputs from trace sources and forward them to the collector.
   * \param sliceId the sliceId ID of the BBFrame.
   * \param symbolRate the symbol rate during the allocation cycle.
   */
  void SymbolRateCallback (uint8_t sliceId, double symbolRate);

  /**
   * \return
   */
  Callback<void, uint8_t, double> GetTraceSinkCallback () const;

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

  /// Maintains a list of collectors created by this helper.
  CollectorMap m_collectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

private:
  ///
  Callback<void, uint8_t, double> m_traceSinkCallback;

}; // end of class SatStatsFwdLinkSchedulerSymbolRateHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_FWD_LINK_SCHEDULER_SYMBOL_RATE_HELPER_H */
