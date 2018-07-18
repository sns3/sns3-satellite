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

#ifndef SATELLITE_STATS_FRAME_TYPE_USAGE_HELPER_H
#define SATELLITE_STATS_FRAME_TYPE_USAGE_HELPER_H

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
class SatStatsFrameTypeUsageHelper : public SatStatsHelper
{
public:
  /**
   * Constructor.
   * \param satHelper Satellite helper instance
   */
  SatStatsFrameTypeUsageHelper (Ptr<const SatHelper> satHelper);

  /**
   * / Destructor.
   */
  virtual ~SatStatsFrameTypeUsageHelper ();

  /**
   * inherited from ObjectBase base class
   * \return Type ID of the helper
   */
  static TypeId GetTypeId ();

  /**
   * Mapping for frame type IDs to integers, in case
   * that SatBbFrameType_t enums are assigned.
   */
  static const std::map<SatEnums::SatBbFrameType_t, uint32_t> frameTypeIdMap;

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

  /**
   * Get identifier header for file.
   * \param dataLabel Label for data column
   * \return String containing identifier type (global, gw_id, beam_id),
   * 				 frame_type and data label.
   */
  std::string GetIdentifierHeading (std::string dataLabel) const;

private:
  /**
   * \param context Beam ID, GW ID or global ID (0) as string
   * \param frameType Type of the frame
   */
  void FrameTypeUsageCallback (std::string context, SatEnums::SatBbFrameType_t frameType);

  /**
   * \brief Get frame type ID.
   * \param frameType Frame type enum
   * \return ID as integer
   */
  static uint32_t GetFrameTypeId (SatEnums::SatBbFrameType_t frameType);

  /**
   * Flag for using percentage of the frame types in beam/in gw/globally
   * instead of sum by type.
   */
  bool m_usePercentage;

  /**
   * \brief Two-dimensional map of collectors, indexed first by the
   * the frame type identifier and second by the global/gw/beam identifier.
   */
  std::map<uint32_t, CollectorMap> m_collectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;


}; // end of class SatStatsFrameTypeUsageHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_FRAME_TYPE_USAGE_HELPER_H */
