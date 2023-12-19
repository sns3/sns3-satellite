/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#ifndef SATELLITE_STATS_LINK_RX_POWER_HELPER_H
#define SATELLITE_STATS_LINK_RX_POWER_HELPER_H

#include <ns3/callback.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>

namespace ns3
{

// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class DataCollectionObject;
class DistributionCollector;

/**
 * \ingroup satstats
 * \brief Base class for link RX power statistics helpers.
 */
class SatStatsLinkRxPowerHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsLinkRxPowerHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsLinkRxPowerHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \param averagingMode average all samples before passing them to aggregator.
     */
    void SetAveragingMode(bool averagingMode);

    /**
     * \brief Set up several probes or other means of listeners and connect them
     *        to the collectors.
     */
    void InstallProbes();

    /**
     * \brief Receive inputs from trace sources and forward them to the collector.
     * \param rxPowerDb RX power value in dB.
     * \param addr Address of UT
     */
    void RxPowerCallback(double rxPowerDb, const Address& addr);

    /**
     * \return
     */
    Callback<void, double, const Address&> GetTraceSinkCallback() const;

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

    /**
     * \brief
     */
    virtual void DoInstallProbes() = 0;

    /**
     * \brief Connect the probe to the right collector.
     * \param probe
     * \param identifier
     */
    bool ConnectProbeToCollector(Ptr<Probe> probe, uint32_t identifier);

    /**
     * \brief Find a collector with the right identifier and pass a sample data
     *        to it.
     * \param rxPowerDb
     * \param identifier
     */
    void PassSampleToCollector(double rxPowerDb, uint32_t identifier);

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The collector created by this helper.
    Ptr<DataCollectionObject> m_collector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

  private:
    ///
    Callback<void, double, const Address&> m_traceSinkCallback;

    bool m_averagingMode; ///< `AveragingMode` attribute.

}; // end of class SatStatsLinkRxPowerHelper

// FORWARD FEEDER LINK ////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link RX power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederLinkRxPowerHelper> s = Create<SatStatsFwdFeederLinkRxPowerHelper>
 * (satHelper); s->SetName ("name"); s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederLinkRxPowerHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederLinkRxPowerHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkRxPowerHelper base class
    void DoInstallProbes();

}; // end of class SatStatsFwdFeederLinkRxPowerHelper

// FORWARD USER LINK //////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link RX power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserLinkRxPowerHelper> s = Create<SatStatsFwdUserLinkRxPowerHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserLinkRxPowerHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserLinkRxPowerHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkRxPowerHelper base class
    void DoInstallProbes();

}; // end of class SatStatsFwdUserLinkRxPowerHelper

// RETURN FEEDER LINK /////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link RX power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederLinkRxPowerHelper> s = Create<SatStatsRtnFeederLinkRxPowerHelper>
 * (satHelper); s->SetName ("name"); s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederLinkRxPowerHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederLinkRxPowerHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkRxPowerHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederLinkRxPowerHelper

// RETURN USER LINK ///////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link RX power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserLinkRxPowerHelper> s = Create<SatStatsRtnUserLinkRxPowerHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserLinkRxPowerHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserLinkRxPowerHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkRxPowerHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnUserLinkRxPowerHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_LINK_RX_POWER_HELPER_H */
