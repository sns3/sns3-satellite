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

#ifndef SATELLITE_STATS_WAVEFORM_HELPER_H
#define SATELLITE_STATS_WAVEFORM_HELPER_H

#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>
#include <ns3/collector-map.h>
#include <map>


namespace ns3 {


class SatHelper;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsWaveformUsageHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsWaveformUsageHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsWaveformUsageHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();
  std::string GetIdentifierHeading (std::string dataLabel) const;

private:
  /**
   * \param context
   * \param waveformId
   */
  void WaveformUsageCallback (std::string context, uint32_t waveformId);

  /**
   * \brief Two-dimensional map of collectors, indexed by the waveform ID and
   *        then by the identifier.
   */
  std::map<uint32_t, CollectorMap> m_collectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

}; // end of class SatStatsWaveformUsageHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_WAVEFORM_HELPER_H */
