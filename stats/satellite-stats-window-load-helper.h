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

#ifndef SATELLITE_STATS_WINDOW_LOAD_HELPER_H
#define SATELLITE_STATS_WINDOW_LOAD_HELPER_H

#include <ns3/callback.h>
#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>

namespace ns3
{

// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief Abstract class inherited by SatStatsRtnFeederWindowLoadHelper.
 */
class SatStatsWindowLoadHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsWindowLoadHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsWindowLoadHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Set up several probes or other means of listeners and connect them
     *        to the collectors.
     */
    void InstallProbes();

    /**
     * \brief Receive inputs from trace sources and forward them to the collector.
     * \param windowLoad the normalized window load in bps/Hz.
     */
    void WindowLoadCallback(double windowLoad);

    /**
     * \return
     */
    Callback<void, double> GetTraceSinkCallback() const;

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

    /**
     * \brief
     */
    virtual void DoInstallProbes() = 0;

    /// The collector created by this helper.
    Ptr<DataCollectionObject> m_collector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

  private:
    ///
    Callback<void, double> m_traceSinkCallback;

}; // end of class SatStatsWindowLoadHelper

// RETURN FEEDER LINK /////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder window load statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederWindowLoadHelper> s = Create<SatStatsRtnFeederWindowLoadHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederWindowLoadHelper : public SatStatsWindowLoadHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederWindowLoadHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederWindowLoadHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsWindowLoadHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederWindowLoadHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_WINDOW_LOAD_HELPER_H */
