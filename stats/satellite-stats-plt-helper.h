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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef SATELLITE_STATS_PLT_HELPER_H
#define SATELLITE_STATS_PLT_HELPER_H

#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>

#include <list>
#include <map>

namespace ns3
{

// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class Time;
class DataCollectionObject;
class DistributionCollector;

/**
 * \ingroup satstats
 * \brief Base class for PLT statistics helpers.
 */
class SatStatsPltHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsPltHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsPltHelper();

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
     * \brief Set up several probes or other means of listeners and connect them
     *        to the collectors.
     */
    void InstallProbes();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param plt object PLT.
     * \param from the address of the sender of the object.
     *
     * Used in return link statistics. DoInstallProbes() is expected to connect
     * the right trace sources to this method.
     */
    void RxPltCallback(const Time& plt, const Address& from);

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
     * \param plt
     * \param identifier
     */
    void PassSampleToCollector(const Time& plt, uint32_t identifier);

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

  private:
    bool m_averagingMode; ///< `AveragingMode` attribute.

}; // end of class SatStatsPltHelper

// FORWARD LINK APPLICATION-LEVEL /////////////////////////////////////////////

class Probe;

class SatStatsFwdAppPltHelper : public SatStatsPltHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdAppPltHelper(Ptr<const SatHelper> satHelper);

    /**
     * Destructor for SatStatsFwdAppPltHelper.
     */
    virtual ~SatStatsFwdAppPltHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsFwdAppPltHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdAppPltHelper

// RETURN LINK APPLICATION-LEVEL //////////////////////////////////////////////

class SatStatsRtnAppPltHelper : public SatStatsPltHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnAppPltHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnAppPltHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param plt object PLT.
     * \param from the InetSocketAddress of the sender of the object.
     */
    void Ipv4Callback(const Time& plt, const Address& from);

  protected:
    // inherited from SatStatsRtnAppPltHelper base class
    void DoInstallProbes();

  private:
    /**
     * \brief Save the IPv4 address and the proper identifier from the given
     *        UT user node.
     * \param utUserNode a UT user node.
     *
     * Any addresses found in the given node will be saved in the
     * #m_identifierMap member variable.
     */
    void SaveIpv4AddressAndIdentifier(Ptr<Node> utUserNode);

    /// \todo Write SaveIpv6Address() method.

}; // end of class SatStatsRtnAppPltHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_PLT_HELPER_H */
