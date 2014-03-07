/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef SATELLITE_STATS_RTN_APP_DELAY_HELPER_H
#define SATELLITE_STATS_RTN_APP_DELAY_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/ptr.h>
#include <ns3/address.h>
#include <map>

namespace ns3 {

class SatHelper;
class DataCollectionObject;
class Time;

/**
 * \brief
 */
class SatStatsRtnAppDelayHelper : public SatStatsHelper
{
public:
  /**
   * \brief
   * \param satHelper
   */
  SatStatsRtnAppDelayHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsRtnAppDelayHelper ();

  /**
   * \brief
   * \param delay
   * \param from
   */
  void ApplicationDelayCallback (Time delay, const Address &from);

protected:
  // inherited from SatStatsHelper base class
  virtual void DoInstall ();

private:
  /**
   * \brief Save the MAC address and the proper identifier from the given UT
   *        user node.
   * \param utUserNode
   *
   * Saved in #m_identifierMap member variable.
   *
   * This method is not used at the moment.
   */
  void SaveMacAddressAndIdentifier (Ptr<Node> utUserNode);

  /**
   * \brief Save the IPv4 address and the proper identifier from the given UT
   *        user node.
   * \param utUserNode
   *
   * Saved in #m_identifierMap member variable.
   */
  void SaveIpv4AddressAndIdentifier (Ptr<Node> utUserNode);

  // TODO: Write SaveIpv6Address() method.

  // key: identifier ID
  SatStatsHelper::CollectorMap_t m_terminalCollectors;

  ///
  Ptr<DataCollectionObject> m_aggregator;

  // key: UT user address
  std::map<const Address, uint32_t> m_identifierMap;

}; // end of class SatStatsRtnAppDelayHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_RTN_APP_DELAY_HELPER_H */
