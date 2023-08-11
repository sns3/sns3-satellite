/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_STATS_ANTENNA_GAIN_HELPER_H
#define SATELLITE_STATS_ANTENNA_GAIN_HELPER_H

#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>

#include <list>
#include <map>

namespace ns3
{

class SatHelper;
class Node;
class Time;
class DataCollectionObject;
class DistributionCollector;

/**
 * \ingroup satstats
 * \brief Base class for antenna gain statistics helpers.
 */
class SatStatsAntennaGainHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsAntennaGainHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsAntennaGainHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \param averagingMode average all samples before passing them to aggregator.
     */
    void SetAveragingMode(bool averagingMode);

    /**
     * \return the currently active averaging mode.
     */
    bool GetAveragingMode() const;

    /**
     * \brief Set up several probes or other means of listeners
     *        and connect them to the collectors.
     */
    void InstallProbes();

    /**
     * \brief Receive inputs from trace sources
     * \param identifier the source ID as a context string.
     * \param gain the antenna gain collected.
     */
    void AntennaGainCallback(std::string identifier, double gain);

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

  private:
    bool m_averagingMode; ///< `AveragingMode` attribute.

}; // end of class SatStatsAntennaGainHelper

} // namespace ns3

#endif /* SATELLITE_STATS_ANTENNA_GAIN_HELPER_H */
