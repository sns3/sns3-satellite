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

#ifndef SATELLITE_STATS_LINK_MODCOD_HELPER_H
#define SATELLITE_STATS_LINK_MODCOD_HELPER_H

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
 * \brief Base class for MODCOD statistics helpers.
 */
class SatStatsLinkModcodHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsLinkModcodHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsLinkModcodHelper();

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
     * \param modcod MODCOD ID
     * \param from the address of the sender of the packet.
     *
     * Used in return link statistics. DoInstallProbes() is expected to connect
     * the right trace sources to this method.
     */
    void RxLinkModcodCallback(uint32_t modcod, const Address& from);

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
     * \param modcod
     * \param identifier
     */
    void PassSampleToCollector(uint32_t modcod, uint32_t identifier);

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

  private:
    bool m_averagingMode; ///< `AveragingMode` attribute.

}; // end of class SatStatsLinkModcodHelper

// FORWARD FEEDER LINK /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link modcod statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederLinkModcodHelper> s = Create<SatStatsFwdFeederLinkModcodHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederLinkModcodHelper : public SatStatsLinkModcodHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederLinkModcodHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederLinkModcodHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkModcodHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdFeederLinkModcodHelper

// FORWARD USER LINK /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link modcod statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserLinkModcodHelper> s = Create<SatStatsFwdUserLinkModcodHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserLinkModcodHelper : public SatStatsLinkModcodHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserLinkModcodHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserLinkModcodHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkModcodHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdUserLinkModcodHelper

// RETURN FEEDER LINK //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link modcod statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederLinkModcodHelper> s = Create<SatStatsRtnFeederLinkModcodHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederLinkModcodHelper : public SatStatsLinkModcodHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederLinkModcodHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederLinkModcodHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkModcodHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederLinkModcodHelper

// RETURN USER LINK //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link modcod statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserLinkModcodHelper> s = Create<SatStatsRtnUserLinkModcodHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserLinkModcodHelper : public SatStatsLinkModcodHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserLinkModcodHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserLinkModcodHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkModcodHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsRtnUserLinkModcodHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_LINK_MODCOD_HELPER_H */
