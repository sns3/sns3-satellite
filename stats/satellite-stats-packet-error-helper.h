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

#ifndef SATELLITE_STATS_PACKET_ERROR_HELPER_H
#define SATELLITE_STATS_PACKET_ERROR_HELPER_H

#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/satellite-stats-helper.h>

#include <map>

namespace ns3
{

// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class DataCollectionObject;
class Probe;

/**
 * \ingroup satstats
 * \brief Base class for packet error statistics helpers.
 */
class SatStatsPacketErrorHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param nPackets number of packets in the received packet burst.
     * \param fromOrTo the source or destination address of the packet.
     * \param isError whether a PHY error has occurred.
     *
     * Used only in return link.
     */
    void ErrorRxCallback(uint32_t nPackets, const Address& fromOrTo, bool isError);

    /**
     * \param traceSourceName name of trace source of PHY RX carrier to listen to.
     */
    void SetTraceSourceName(std::string traceSourceName);

    /**
     * \return name of trace source of PHY RX carrier to listen to.
     */
    std::string GetTraceSourceName() const;

    /**
     * \param channelType link where statistics are gathered from.
     */
    void SetChannelType(SatEnums::ChannelType_t channelType);

    /**
     * \return link where statistics are gathered from.
     */
    SatEnums::ChannelType_t GetChannelType() const;

    /**
     * \brief Get the valid carrier type
     * \return the valid carrier type
     */
    inline SatPhyRxCarrier::CarrierType GetValidCarrierType() const
    {
        return m_carrierType;
    }

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

    /**
     * \brief Set valid carrier type for this statistics helper type.
     * \param carrierType
     */
    inline void SetValidCarrierType(SatPhyRxCarrier::CarrierType carrierType)
    {
        m_carrierType = carrierType;
    }

  private:
    /**
     * \brief Set up several listeners on a GW node and connect them to the
     *        collectors.
     * \param gwNode
     */
    void InstallProbeOnGw(Ptr<Node> gwNode);

    /**
     * \brief Set up several listeners on a SAT feeder node and connect them to the
     *        collectors.
     * \param satNode
     */
    void InstallProbeOnSatFeeder(Ptr<Node> satNode);

    /**
     * \brief Set up several listeners on a SAT user node and connect them to the
     *        collectors.
     * \param satNode
     */
    void InstallProbeOnSatUser(Ptr<Node> satNode);

    /**
     * \brief Set up several probes on a UT node and connect them to the
     *        collectors.
     * \param utNode
     */
    void InstallProbeOnUt(Ptr<Node> utNode);

    /// Maintains a list of probes created by this helper (for forward link).
    std::list<Ptr<Probe>> m_probes;

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

    /// Name of trace source of PHY RX carrier to listen to.
    std::string m_traceSourceName;

    /// Link where statistics are gathered from.
    SatEnums::ChannelType_t m_channelType;

    /// Valid carrier type
    SatPhyRxCarrier::CarrierType m_carrierType;

}; // end of class SatStatsPacketErrorHelper

// FORWARD FEEDER LINK DEDICATED ACCESS //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Dedicated Access in forward feeder link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederDaPacketErrorHelper> s = Create<SatStatsFwdFeederDaPacketErrorHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederDaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsFwdFeederDaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdFeederDaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// FORWARD USER LINK DEDICATED ACCESS //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Dedicated Access in forward user link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserDaPacketErrorHelper> s = Create<SatStatsFwdUserDaPacketErrorHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserDaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsFwdUserDaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFwdUserDaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// RETURN FEEDER LINK DEDICATED ACCESS ///////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Dedicated Access in return feeder link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederDaPacketErrorHelper> s = Create<SatStatsRtnFeederDaPacketErrorHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederDaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsRtnFeederDaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnFeederDaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// RETURN USER LINK DEDICATED ACCESS ///////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Dedicated Access in return user link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserDaPacketErrorHelper> s = Create<SatStatsRtnUserDaPacketErrorHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserDaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsRtnUserDaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsRtnUserDaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// FEEDER SLOTTED ALOHA //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access Slotted ALOHA on feeder link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFeederSlottedAlohaPacketErrorHelper> s =
 * Create<SatStatsFeederSlottedAlohaPacketErrorHelper> (satHelper); s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFeederSlottedAlohaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsFeederSlottedAlohaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFeederSlottedAlohaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// USER SLOTTED ALOHA //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access Slotted ALOHA on user link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsUserSlottedAlohaPacketErrorHelper> s =
 * Create<SatStatsUserSlottedAlohaPacketErrorHelper> (satHelper); s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsUserSlottedAlohaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsUserSlottedAlohaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsUserSlottedAlohaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// FEEDER CRDSA //////////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access CRDSA on feeder link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFeederCrdsaPacketErrorHelper> s = Create<SatStatsFeederCrdsaPacketErrorHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFeederCrdsaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsFeederCrdsaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFeederCrdsaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// USER CRDSA //////////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access CRDSA on user link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsUserCrdsaPacketErrorHelper> s = Create<SatStatsUserCrdsaPacketErrorHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsUserCrdsaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsUserCrdsaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsUserCrdsaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// FEEDER E-SSA //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access E-SSA on feeder link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFeederEssaPacketErrorHelper> s = Create<SatStatsFeederEssaPacketErrorHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFeederEssaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsFeederEssaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFeederEssaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// USER E-SSA //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access E-SSA on user link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsUserEssaPacketErrorHelper> s = Create<SatStatsUserEssaPacketErrorHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsUserEssaPacketErrorHelper : public SatStatsPacketErrorHelper
{
  public:
    // inherited from SatStatsPacketErrorHelper base class
    SatStatsUserEssaPacketErrorHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsUserEssaPacketErrorHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

} // end of namespace ns3

#endif /* SATELLITE_STATS_PACKET_ERROR_HELPER_H */
