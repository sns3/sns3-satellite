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

#ifndef SATELLITE_STATS_PACKET_COLLISION_HELPER_H
#define SATELLITE_STATS_PACKET_COLLISION_HELPER_H

#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/satellite-stats-helper.h>

#include <map>

namespace ns3
{

// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsPacketCollisionHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param nPackets number of packets in the received packet burst.
     * \param from the source address of the packet.
     * \param isCollided whether a collision has occurred.
     */
    void CollisionRxCallback(uint32_t nPackets, const Address& from, bool isCollided);

    /**
     * \return
     */
    std::string GetTraceSourceName() const;

    /**
     * \brief Get the valid carrier type
     * \return the valid carrier type
     */
    inline SatPhyRxCarrier::CarrierType GetValidCarrierType() const
    {
        return m_carrierType;
    }

  protected:
    /**
     * \param traceSourceName
     */
    void SetTraceSourceName(std::string traceSourceName);

    /**
     * \brief Set valid carrier type for this statistics helper type.
     * \param carrierType
     */
    inline void SetValidCarrierType(SatPhyRxCarrier::CarrierType carrierType)
    {
        m_carrierType = carrierType;
    }

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

  private:
    SatPhyRxCarrier::CarrierType m_carrierType;

    std::string m_traceSourceName;

}; // end of class SatStatsPacketCollisionHelper

// BASE CLASS FEEDER /////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsFeederPacketCollisionHelper : public SatStatsPacketCollisionHelper
{
  public:
    // inherited from SatStatsPacketCollisionHelper base class
    SatStatsFeederPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFeederPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

  private:
    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

}; // end of class SatStatsFeederPacketCollisionHelper

// BASE CLASS USER /////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsUserPacketCollisionHelper : public SatStatsPacketCollisionHelper
{
  public:
    // inherited from SatStatsPacketCollisionHelper base class
    SatStatsUserPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsUserPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

  private:
    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

}; // end of class SatStatsUserPacketCollisionHelper

// SLOTTED ALOHA FEEDER //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet collision statistics of Random Access Slotted ALOHA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFeederSlottedAlohaPacketCollisionHelper> s =
 * Create<SatStatsFeederSlottedAlohaPacketCollisionHelper> (satHelper); s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFeederSlottedAlohaPacketCollisionHelper : public SatStatsFeederPacketCollisionHelper
{
  public:
    // inherited from SatStatsFeederPacketCollisionHelper base class
    SatStatsFeederSlottedAlohaPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFeederSlottedAlohaPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// CRDSA FEEDER //////////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet collision statistics of Random Access CRDSA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFeederCrdsaPacketCollisionHelper> s =
 * Create<SatStatsFeederCrdsaPacketCollisionHelper> (satHelper); s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFeederCrdsaPacketCollisionHelper : public SatStatsFeederPacketCollisionHelper
{
  public:
    // inherited from SatStatsFeederPacketCollisionHelper base class
    SatStatsFeederCrdsaPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFeederCrdsaPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// E-SSA FEEDER //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet collision statistics of Random Access E-SSA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFeederEssaPacketCollisionHelper> s = Create<SatStatsFeederEssaPacketCollisionHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFeederEssaPacketCollisionHelper : public SatStatsFeederPacketCollisionHelper
{
  public:
    // inherited from SatStatsFeederPacketCollisionHelper base class
    SatStatsFeederEssaPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsFeederEssaPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// SLOTTED ALOHA USER //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet collision statistics of Random Access Slotted ALOHA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsUserSlottedAlohaPacketCollisionHelper> s =
 * Create<SatStatsUserSlottedAlohaPacketCollisionHelper> (satHelper); s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsUserSlottedAlohaPacketCollisionHelper : public SatStatsUserPacketCollisionHelper
{
  public:
    // inherited from SatStatsUserPacketCollisionHelper base class
    SatStatsUserSlottedAlohaPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsUserSlottedAlohaPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// CRDSA USER //////////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet collision statistics of Random Access CRDSA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsUserCrdsaPacketCollisionHelper> s = Create<SatStatsUserCrdsaPacketCollisionHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsUserCrdsaPacketCollisionHelper : public SatStatsUserPacketCollisionHelper
{
  public:
    // inherited from SatStatsUserPacketCollisionHelper base class
    SatStatsUserCrdsaPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsUserCrdsaPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

// E-SSA USER //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet collision statistics of Random Access E-SSA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsUserEssaPacketCollisionHelper> s = Create<SatStatsUserEssaPacketCollisionHelper>
 * (satHelper); s->SetName ("name"); s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsUserEssaPacketCollisionHelper : public SatStatsUserPacketCollisionHelper
{
  public:
    // inherited from SatStatsUserPacketCollisionHelper base class
    SatStatsUserEssaPacketCollisionHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsUserEssaPacketCollisionHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();
};

} // end of namespace ns3

#endif /* SATELLITE_STATS_PACKET_COLLISION_HELPER_H */
