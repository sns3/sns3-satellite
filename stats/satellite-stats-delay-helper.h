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

#ifndef SATELLITE_STATS_DELAY_HELPER_H
#define SATELLITE_STATS_DELAY_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/ptr.h>
#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <list>
#include <map>


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class Time;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsDelayHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsDelayHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsDelayHelper ();

  /**
   * \brief Set up several probes or other means of listeners and connect them
   *        to the collectors.
   */
  void InstallProbes ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param delay packet delay.
   * \param from the address of the sender of the packet.
   *
   * Used in return link statistics. DoInstallProbes() is expected to connect
   * the right trace sources to this method.
   */
  virtual void RxDelayCallback (Time delay, const Address &from);

protected:
  // inherited from SatStatsHelper base class
  virtual void DoInstall ();

  /**
   * \brief
   */
  virtual void DoInstallProbes () = 0;

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
  virtual void SaveAddressAndIdentifier (Ptr<Node> utNode);

  /// Maintains a list of collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

  /// Map of address and the identifier associated with it (for return link).
  std::map<const Address, uint32_t> m_identifierMap;

}; // end of class SatStatsFwdAppDelayHelper


// FORWARD LINK APPLICATION-LEVEL /////////////////////////////////////////////

class Probe;

/**
 * \ingroup satstats
 * \brief Produce forward link application-level delay statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdAppDelayHelper> s = Create<SatStatsFwdAppDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdAppDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdAppDelayHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsFwdAppDelayHelper ();

protected:
  // inherited from SatStatsDelayHelper base class
  virtual void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdAppDelayHelper


// RETURN LINK APPLICATION-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return link application-level delay statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnAppDelayHelper> s = Create<SatStatsRtnAppDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnAppDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnAppDelayHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsRtnAppDelayHelper ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param delay packet delay.
   * \param from the InetSocketAddress of the sender of the packet.
   */
  void Ipv4Callback (Time delay, const Address &from);

protected:
  // inherited from SatStatsDelayHelper base class
  virtual void DoInstallProbes ();

private:
  /**
   * \brief Save the IPv4 address and the proper identifier from the given
   *        UT user node.
   * \param utUserNode a UT user node.
   *
   * Any addresses found in the given node will be saved in the
   * #m_identifierMap member variable.
   */
  void SaveIpv4AddressAndIdentifier (Ptr<Node> utUserNode);

  /// \todo Write SaveIpv6Address() method.

}; // end of class SatStatsRtnAppDelayHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_FWD_APP_DELAY_HELPER_H */
