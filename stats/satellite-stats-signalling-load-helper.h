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

#ifndef SATELLITE_STATS_SIGNALLING_LOAD_HELPER_H
#define SATELLITE_STATS_SIGNALLING_LOAD_HELPER_H

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
class Packet;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief Abstract class inherited by SatStatsFwdSignallingLoadHelper and SatStatsRtnSignallingLoadHelper.
 */
class SatStatsSignallingLoadHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsSignallingLoadHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsSignallingLoadHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \brief Set up several probes or other means of listeners and connect them
   *        to the first-level collectors.
   */
  void InstallProbes ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param packet control message packet transmitted.
   * \param to the destination address of the packet.
   *
   * Used in return link statistics. DoInstallProbes() is expected to connect
   * the right trace sources to this method.
   */
  void SignallingTxCallback (Ptr<const Packet> packet, const Address &to);

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

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
   * Used in forward link statistics. DoInstallProbes() is expected to pass the
   * the UT node of interest into this method.
   */
  void SaveAddressAndIdentifier (Ptr<Node> utNode);

  /// Maintains a list of first-level collectors created by this helper.
  CollectorMap m_conversionCollectors;

  /// Maintains a list of second-level collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

  /// Map of address and the identifier associated with it (for forward link).
  std::map<const Address, uint32_t> m_identifierMap;

}; // end of class SatStatsSignallingLoadHelper


// FORWARD LINK ///////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward link signalling load statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdSignallingLoadHelper> s = Create<SatStatsFwdSignallingLoadHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdSignallingLoadHelper : public SatStatsSignallingLoadHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdSignallingLoadHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdSignallingLoadHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsFwdSignallingLoadHelper


// RETURN LINK ////////////////////////////////////////////////////////////////

class Probe;

/**
 * \ingroup satstats
 * \brief Produce return link signalling load statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnSignallingLoadHelper> s = Create<SatStatsRtnSignallingLoadHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnSignallingLoadHelper : public SatStatsSignallingLoadHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnSignallingLoadHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnSignallingLoadHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsRtnSignallingLoadHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_SIGNALLING_LOAD_HELPER_H */
