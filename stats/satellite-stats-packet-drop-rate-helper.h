/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_STATS_PACKET_DROP_RATE_HELPER_H
#define SATELLITE_STATS_PACKET_DROP_RATE_HELPER_H

#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>

#include <list>
#include <map>

namespace ns3
{

class SatHelper;
class Node;
class Time;
class DataCollectionObject;
class DistributionCollector;

/**
 * \ingroup satstats
 * \brief Base class for antenna gain statistics helpers.
 */
class SatStatsPacketDropRateHelper : public SatStatsHelper
{
  public:
    // inherited from SatStatsHelper base class
    SatStatsPacketDropRateHelper(Ptr<const SatHelper> satHelper);

    /**
     * / Destructor.
     */
    virtual ~SatStatsPacketDropRateHelper();

    /**
     * inherited from ObjectBase base class
     */
    static TypeId GetTypeId();

    /**
     * \brief Set up several probes or other means of listeners
     *        and connect them to the collectors.
     */
    void InstallProbes();

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param nPackets number of packets in the received packet burst.
     * \param satSrc Satellite src
     * \param satDst Satellite dst
     * \param isError whether a PHY error has occurred.
     *
     * Used only in return link.
     */
    void PacketDropRateCallback(uint32_t nPackets,
                                Ptr<Node> satSrc,
                                Ptr<Node> satDst,
                                bool isError);

  protected:
    // inherited from SatStatsHelper base class
    void DoInstall();

    /// Maintains a list of collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

}; // end of class SatStatsPacketDropRateHelper

} // namespace ns3

#endif /* SATELLITE_STATS_PACKET_DROP_RATE_HELPER_H */
