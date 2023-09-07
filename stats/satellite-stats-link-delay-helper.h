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

#ifndef SATELLITE_STATS_LINK_DELAY_HELPER_H
#define SATELLITE_STATS_LINK_DELAY_HELPER_H

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
 * \brief Base class for delay statistics helpers.
 */
class SatStatsLinkDelayHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsLinkDelayHelper();

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
     * \param delay packet delay.
     * \param from the address of the sender of the packet.
     *
     * Used in return link statistics. DoInstallProbes() is expected to connect
     * the right trace sources to this method.
     */
    void RxLinkDelayCallback(const Time& delay, const Address& from);

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
     * \param delay
     * \param identifier
     */
    void PassSampleToCollector(const Time& delay, uint32_t identifier);

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

}; // end of class SatStatsLinkDelayHelper

// FORWARD FEEDER LINK DEV-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link DEV-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederDevLinkDelayHelper> s = Create<SatStatsFwdFeederDevLinkDelayHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederDevLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederDevLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederDevLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdFeederDevLinkDelayHelper

// FORWARD USER LINK DEV-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link DEV-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserDevLinkDelayHelper> s = Create<SatStatsFwdUserDevLinkDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserDevLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserDevLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserDevLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdUserDevLinkDelayHelper

// FORWARD FEEDER LINK MAC-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link MAC-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederMacLinkDelayHelper> s = Create<SatStatsFwdFeederMacLinkDelayHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederMacLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederMacLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederMacLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdFeederMacLinkDelayHelper

// FORWARD USER LINK MAC-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link MAC-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserMacLinkDelayHelper> s = Create<SatStatsFwdUserMacLinkDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserMacLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserMacLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserMacLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdUserMacLinkDelayHelper

// FORWARD FEEDER LINK PHY-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link PHY-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederPhyLinkDelayHelper> s = Create<SatStatsFwdFeederPhyLinkDelayHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederPhyLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederPhyLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederPhyLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdFeederPhyLinkDelayHelper

// FORWARD USER LINK PHY-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link PHY-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserPhyLinkDelayHelper> s = Create<SatStatsFwdUserPhyLinkDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserPhyLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserPhyLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserPhyLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsFwdUserPhyLinkDelayHelper

// RETURN FEEDER LINK DEV-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link DEV-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederDevLinkDelayHelper> s = Create<SatStatsRtnFeederDevLinkDelayHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederDevLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederDevLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederDevLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederDevLinkDelayHelper

// RETURN USER LINK DEV-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link DEV-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserDevLinkDelayHelper> s = Create<SatStatsRtnUserDevLinkDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserDevLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserDevLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserDevLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsRtnUserDevLinkDelayHelper

// RETURN FEEDER LINK MAC-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link MAC-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederMacLinkDelayHelper> s = Create<SatStatsRtnFeederMacLinkDelayHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederMacLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederMacLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederMacLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederMacLinkDelayHelper

// RETURN USER LINK MAC-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link MAC-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserMacLinkDelayHelper> s = Create<SatStatsRtnUserMacLinkDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserMacLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserMacLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserMacLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsRtnUserMacLinkDelayHelper

// RETURN FEEDER LINK PHY-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link PHY-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederPhyLinkDelayHelper> s = Create<SatStatsRtnFeederPhyLinkDelayHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederPhyLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederPhyLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederPhyLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederPhyLinkDelayHelper

// RETURN USER LINK PHY-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link PHY-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserPhyLinkDelayHelper> s = Create<SatStatsRtnUserPhyLinkDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserPhyLinkDelayHelper : public SatStatsLinkDelayHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserPhyLinkDelayHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserPhyLinkDelayHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkDelayHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

}; // end of class SatStatsRtnUserPhyLinkDelayHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_DELAY_HELPER_H */
