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
 * Author: Lauri Sormunen <lauri.sormunen@magister.fi>
 *
 */

#ifndef SATELLITE_STATS_BEAM_SERVICE_TIME_HELPER_H
#define SATELLITE_STATS_BEAM_SERVICE_TIME_HELPER_H

#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>
#include <ns3/collector-map.h>
#include <ns3/satellite-enums.h>
#include <map>


namespace ns3 {


class SatHelper;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsBeamServiceTimeHelper : public SatStatsHelper
{
public:
  /**
   * Constructor.
   * \param satHelper Satellite helper instance
   */
  SatStatsBeamServiceTimeHelper (Ptr<const SatHelper> satHelper);

  /**
   * / Destructor.
   */
  virtual ~SatStatsBeamServiceTimeHelper ();

  /**
   * inherited from ObjectBase base class
   * \return Type ID of the helper
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

private:
  /**
   * \param context Beam ID, GW ID or global ID (0) as string
   * \param time The time beam was enabled
   */
  void BeamServiceCallback (std::string context, Time time);

  /**
   * \brief Two-dimensional map of collectors, indexed first by the
   * the frame type identifier and second by the global/gw/beam identifier.
   */
  CollectorMap m_collectorMap;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;


}; // end of class SatStatsBeamServiceTimeHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_BEAM_SERVICE_TIME_HELPER_H */
