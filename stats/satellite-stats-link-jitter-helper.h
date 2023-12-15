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

#ifndef SATELLITE_STATS_LINK_JITTER_HELPER_H
#define SATELLITE_STATS_LINK_JITTER_HELPER_H

#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>
#include <ns3/satellite-enums.h>
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
class SatStatsLinkJitterHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsLinkJitterHelper();

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
    void RxLinkJitterCallback(const Time& jitter, const Address& from);

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
     * \brief Disconnect the probe from the right collector.
     * \param probe
     * \param identifier
     */
    bool DisconnectProbeFromCollector(Ptr<Probe> probe, uint32_t identifier);

    /**
     * \brief Find a collector with the right identifier and pass a sample data
     *        to it.
     * \param jitter
     * \param identifier
     */
    void PassSampleToCollector(const Time& jitter, uint32_t identifier);

    /**
     * \brief Set the channel used by this probe.
     * \param channelLink The channel used.
     */
    void SetChannelLink(SatEnums::ChannelType_t channelLink);

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

  private:
    bool m_averagingMode; ///< `AveragingMode` attribute.

    SatEnums::ChannelType_t m_channelLink; /// < Set channel where packet is received.

}; // end of class SatStatsLinkJitterHelper

// FORWARD FEEDER LINK DEV-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link DEV-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederDevLinkJitterHelper> s = Create<SatStatsFwdFeederDevLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederDevLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederDevLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederDevLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsFwdFeederDevLinkJitterHelper

// FORWARD USER LINK DEV-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link DEV-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserDevLinkJitterHelper> s = Create<SatStatsFwdUserDevLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserDevLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserDevLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserDevLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * Change identifier used on probes, when handovers occur.
     */
    virtual void UpdateIdentifierOnProbes();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::map<Ptr<Probe>, std::pair<Ptr<Node>, uint32_t>> m_probes;

}; // end of class SatStatsFwdUserDevLinkJitterHelper

// FORWARD FEEDER LINK MAC-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link MAC-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederMacLinkJitterHelper> s = Create<SatStatsFwdFeederMacLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederMacLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederMacLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederMacLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsFwdFeederMacLinkJitterHelper

// FORWARD USER LINK MAC-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link MAC-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserMacLinkJitterHelper> s = Create<SatStatsFwdUserMacLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserMacLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserMacLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserMacLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * Change identifier used on probes, when handovers occur.
     */
    virtual void UpdateIdentifierOnProbes();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::map<Ptr<Probe>, std::pair<Ptr<Node>, uint32_t>> m_probes;

}; // end of class SatStatsFwdUserMacLinkJitterHelper

// FORWARD FEEDER LINK PHY-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link PHY-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederPhyLinkJitterHelper> s = Create<SatStatsFwdFeederPhyLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederPhyLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdFeederPhyLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederPhyLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsFwdFeederPhyLinkJitterHelper

// FORWARD USER LINK PHY-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link PHY-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserPhyLinkJitterHelper> s = Create<SatStatsFwdUserPhyLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserPhyLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsFwdUserPhyLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserPhyLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * Change identifier used on probes, when handovers occur.
     */
    virtual void UpdateIdentifierOnProbes();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

  private:
    /// Maintains a list of probes created by this helper.
    std::map<Ptr<Probe>, std::pair<Ptr<Node>, uint32_t>> m_probes;

}; // end of class SatStatsFwdUserPhyLinkJitterHelper

// RETURN FEEDER LINK DEV-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link DEV-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederDevLinkJitterHelper> s = Create<SatStatsRtnFeederDevLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederDevLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederDevLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederDevLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederDevLinkJitterHelper

// RETURN USER LINK DEV-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link DEV-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserDevLinkJitterHelper> s = Create<SatStatsRtnUserDevLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserDevLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserDevLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserDevLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnUserDevLinkJitterHelper

// RETURN FEEDER LINK MAC-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link MAC-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederMacLinkJitterHelper> s = Create<SatStatsRtnFeederMacLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederMacLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederMacLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederMacLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederMacLinkJitterHelper

// RETURN USER LINK MAC-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link MAC-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserMacLinkJitterHelper> s = Create<SatStatsRtnUserMacLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserMacLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserMacLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserMacLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnUserMacLinkJitterHelper

// RETURN FEEDER LINK PHY-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link PHY-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederPhyLinkJitterHelper> s = Create<SatStatsRtnFeederPhyLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederPhyLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnFeederPhyLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederPhyLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnFeederPhyLinkJitterHelper

// RETURN USER LINK PHY-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link PHY-level jitter statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserPhyLinkJitterHelper> s = Create<SatStatsRtnUserPhyLinkJitterHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserPhyLinkJitterHelper : public SatStatsLinkJitterHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsRtnUserPhyLinkJitterHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserPhyLinkJitterHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsLinkJitterHelper base class
    void DoInstallProbes();

}; // end of class SatStatsRtnUserPhyLinkJitterHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_JITTER_HELPER_H */
