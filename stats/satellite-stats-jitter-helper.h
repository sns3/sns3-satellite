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

#ifndef SATELLITE_STATS_JITTER_HELPER_H
#define SATELLITE_STATS_JITTER_HELPER_H

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
 * \brief Base class for jitter statistics helpers.
 */
class SatStatsJitterHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsJitterHelper();

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
     * \param jitter packet jitter.
     * \param from the address of the sender of the packet.
     *
     * Used in return link statistics. DoInstallProbes() is expected to connect
     * the right trace sources to this method.
     */
    void RxJitterCallback(const Time& jitter, const Address& from);

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
     * \param jitter
     * \param identifier
     */
    void PassSampleToCollector(const Time& jitter, uint32_t identifier);

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

    /// Map of address and the identifier associated with it (for return link).
    std::map<const Address, uint32_t> m_identifierMap;

  private:
    bool m_averagingMode; ///< `AveragingMode` attribute.

}; // end of class SatStatsJitterHelper

// FORWARD LINK APPLICATION-LEVEL /////////////////////////////////////////////

class Probe;

class SatStatsFwdAppJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdAppJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * Destructor for SatStatsFwdAppJitterHelper.
     */
    virtual ~SatStatsFwdAppJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param helper Pointer to the jitter statistics collector helper
     * \param identifier Identifier used to group statistics.
     * \param packet the received packet, expected to have been tagged with
     *               TrafficTimeTag.
     * \param from the InetSocketAddress of the sender of the packet.
     */
    static void RxCallback(Ptr<SatStatsFwdAppJitterHelper> helper,
                           uint32_t identifier,
                           Ptr<const Packet> packet,
                           const Address& from);

    /**
     * \brief Returns the previous delay mesurment for given identifier,
     *        and update with new value measured
     * \param identifier Identifier used to group statistics.
     * \param newDelay new delay measurement
     * \return Previous delay measurement or zero if not value is stored
     */
    Time GetAndUpdatePreviousDelay(uint32_t identifier, Time newDelay);

  protected:
    // inherited from SatStatsFwdAppJitterHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

    /// Stores the last delay to a UT to compute jitter
    std::map<uint32_t, Time> m_previousDelayMap;

}; // end of class SatStatsFwdAppJitterHelper

// FORWARD LINK DEVICE-LEVEL /////////////////////////////////////////////////////

class SatStatsFwdDevJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdDevJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdDevJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsJitterHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdDevJitterHelper

// FORWARD LINK MAC-LEVEL /////////////////////////////////////////////////////

class SatStatsFwdMacJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdMacJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdMacJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsJitterHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdMacJitterHelper

// FORWARD LINK PHY-LEVEL /////////////////////////////////////////////////////

class SatStatsFwdPhyJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdPhyJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdPhyJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsJitterHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdPhyJitterHelper

// RETURN LINK APPLICATION-LEVEL //////////////////////////////////////////////

class SatStatsRtnAppJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnAppJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnAppJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param packet the received packet, expected to have been tagged with
     *               TrafficTimeTag.
     * \param from the InetSocketAddress of the sender of the packet.
     */
    void RxCallback(Ptr<const Packet> packet, const Address& from);

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param jitter packet jitter.
     * \param from the InetSocketAddress of the sender of the packet.
     */
    void Ipv4Callback(const Time& jitter, const Address& from);

  protected:
    // inherited from SatStatsRtnAppJitterHelper base class
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

    /// Stores the last delay to a UT to compute jitter
    std::map<const Address, Time> m_previousDelayMap;

    /// \todo Write SaveIpv6Address() method.

}; // end of class SatStatsRtnAppJitterHelper

// RETURN LINK DEVICE-LEVEL //////////////////////////////////////////////

class SatStatsRtnDevJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnDevJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnDevJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnDevJitterHelper

// RETURN LINK MAC-LEVEL //////////////////////////////////////////////

class SatStatsRtnMacJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnMacJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnMacJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnMacJitterHelper

// RETURN LINK PHY-LEVEL //////////////////////////////////////////////

class SatStatsRtnPhyJitterHelper : public SatStatsJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnPhyJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnPhyJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnPhyJitterHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_JITTER_HELPER_H */
