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

#ifndef SATELLITE_STATS_SAT_QUEUE_HELPER_H
#define SATELLITE_STATS_SAT_QUEUE_HELPER_H

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
 * \brief Base class for sat queue statistics helpers.
 */
class SatStatsSatelliteQueueHelper : public SatStatsHelper
{
  public:
    /**
     * \enum UnitType_t
     * \brief
     */
    typedef enum
    {
        UNIT_BYTES = 0,
        UNIT_NUMBER_OF_PACKETS,
    } UnitType_t;

    /**
     * \param unitType
     * \return
     */
    static std::string GetUnitTypeName(UnitType_t unitType);

    // inherited from SatStatsHelper base class
    SatStatsSatelliteQueueHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsSatelliteQueueHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \param unitType
     */
    void SetUnitType(UnitType_t unitType);

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
     * \param size Queue size (in bytes or packets)
     * \param addr Address of UT
     */
    void QueueSizeCallback(uint32_t size, const Address& addr);

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

    /**
     * \brief
     */
    virtual void DoInstallProbes() = 0;

    /**
     * \brief Save the address and the proper identifier from the given UT node.
     * \param utNode a UT node.
     *
     * The address of the given node will be saved in the #m_identifierMap
     * member variable.
     *
     * Used in return link statistics. DoInstallProbes() is expected to pass the
     * the UT node of interest into this method.
     */
    void SaveAddressAndIdentifier(Ptr<Node> utNode);

    /**
     * \brief Connect the probe to the right collector.
     * \param probe
     * \param identifier
     */
    bool ConnectProbeToCollector(Ptr<Probe> probe, uint32_t identifier);

    /**
     * \brief Find a collector with the right identifier and pass a sample data
     *        to it.
     * \param size
     * \param identifier
     */
    void PassSampleToCollector(uint32_t size, uint32_t identifier);

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The collector created by this helper.
    Ptr<DataCollectionObject> m_collector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

    /// Map of address and the identifier associated with it (for return link).
    std::map<const Address, uint32_t> m_identifierMap;

  private:
    bool m_averagingMode;     ///< `AveragingMode` attribute.
    UnitType_t m_unitType;    ///<
    std::string m_shortLabel; ///<
    std::string m_longLabel;  ///<

}; // end of class SatStatsLinkSinrHelper

// RTN FEEDER QUEUE IN PACKETS ////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce queue size statistics in packets for return feeder link.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederQueuePacketsHelper> s = Create<SatStatsRtnFeederQueuePacketsHelper>
 * (satHelper); s->SetName ("name"); s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederQueuePacketsHelper : public SatStatsSatelliteQueueHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederQueuePacketsHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederQueuePacketsHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkSinrHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederQueuePacketsHelper

// RTN FEEDER QUEUE IN BYTES ////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce queue size statistics in packets for return feeder link.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederQueueBytesHelper> s = Create<SatStatsRtnFeederQueueBytesHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederQueueBytesHelper : public SatStatsSatelliteQueueHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederQueueBytesHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederQueueBytesHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkSinrHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederQueueBytesHelper

// FWD USER QUEUE IN PACKETS ////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce queue size statistics in packets for return feeder link.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserQueuePacketsHelper> s = Create<SatStatsFwdUserQueuePacketsHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserQueuePacketsHelper : public SatStatsSatelliteQueueHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserQueuePacketsHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserQueuePacketsHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkSinrHelper base class
    void DoInstallProbes();

}; // end of class SatStatsFwdUserQueuePacketsHelper

// FWD USER QUEUE IN BYTES ////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce queue size statistics in packets for return feeder link.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserQueueBytesHelper> s = Create<SatStatsFwdUserQueueBytesHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserQueueBytesHelper : public SatStatsSatelliteQueueHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserQueueBytesHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserQueueBytesHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkSinrHelper base class
    void DoInstallProbes();

}; // end of class SatStatsFwdUserQueueBytesHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_SAT_QUEUE_HELPER_H */
