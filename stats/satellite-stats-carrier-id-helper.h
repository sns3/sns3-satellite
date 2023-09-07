/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_STATS_CARRIER_ID_HELPER_H
#define SATELLITE_STATS_CARRIER_ID_HELPER_H

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
class SatStatsCarrierIdHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsCarrierIdHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsCarrierIdHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param carrierId carrier used by the received packet burst.
     * \param from the source address of the packet.
     */
    void CarrierIdRxCallback(uint32_t carrierId, const Address& from);

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
    // inherited from SatStatsHelper base class
    void DoInstall();

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

  private:
    /**
     * \brief Save the address and the proper identifier from the given UT node.
     * \param utNode a UT node.
     *
     * The address of the given node will be saved in the #m_identifierMap
     * member variable.
     */
    void SaveAddressAndIdentifier(Ptr<Node> utNode);

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

    /// Map of address and the identifier associated with it (for forward link).
    std::map<const Address, uint32_t> m_identifierMap;

    std::string m_traceSourceName;

    SatPhyRxCarrier::CarrierType m_carrierType;

}; // end of class SatStatsCarrierIdHelper

} // end of namespace ns3

#endif /* SATELLITE_STATS_CARRIER_ID_HELPER_H */
