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

#ifndef SATELLITE_STATS_THROUGHPUT_HELPER_H
#define SATELLITE_STATS_THROUGHPUT_HELPER_H

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
class DistributionCollector;

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsThroughputHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsThroughputHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \param averagingMode average all samples before passing them to aggregator.
   */
  void SetAveragingMode (bool averagingMode);

  /**
   * \return the currently active averaging mode.
   */
  bool GetAveragingMode () const;

  /**
   * \brief Set up several probes or other means of listeners and connect them
   *        to the first-level collectors.
   */
  void InstallProbes ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param packet received packet data.
   * \param from the address of the sender of the packet.
   *
   * Used in return link statistics. DoInstallProbes() is expected to connect
   * the right trace sources to this method.
   */
  void RxCallback (Ptr<const Packet> packet, const Address &from);

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
   * Used in return link statistics. DoInstallProbes() is expected to pass the
   * the UT node of interest into this method.
   */
  void SaveAddressAndIdentifier (Ptr<Node> utNode);

  /// Maintains a list of first-level collectors created by this helper.
  CollectorMap m_conversionCollectors;

  /// Maintains a list of second-level collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The final collector utilized in averaged output (histogram, PDF, and CDF).
  Ptr<DistributionCollector> m_averagingCollector;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

  /// Map of address and the identifier associated with it (for return link).
  std::map<const Address, uint32_t> m_identifierMap;

private:
  bool m_averagingMode;  ///< `AveragingMode` attribute.

}; // end of class SatStatsThroughputHelper


// FORWARD LINK APPLICATION-LEVEL /////////////////////////////////////////////

class Probe;

/**
 * \ingroup satstats
 * \brief Produce forward link application-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdAppThroughputHelper> s = Create<SatStatsFwdAppThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdAppThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdAppThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdAppThroughputHelper ();


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

}; // end of class SatStatsFwdAppThroughputHelper


// FORWARD LINK DEVICE-LEVEL //////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward link device-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdDevThroughputHelper> s = Create<SatStatsFwdDevThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdDevThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdDevThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdDevThroughputHelper ();


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

}; // end of class SatStatsFwdDevThroughputHelper


// FORWARD FEEDER LINK MAC-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link MAC-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederMacThroughputHelper> s = Create<SatStatsFwdFeederMacThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics include control messages.
 */
class SatStatsFwdFeederMacThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdFeederMacThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdFeederMacThroughputHelper ();


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

}; // end of class SatStatsFwdFeederMacThroughputHelper


// FORWARD USER LINK MAC-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link MAC-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserMacThroughputHelper> s = Create<SatStatsFwdUserMacThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics include control messages.
 */
class SatStatsFwdUserMacThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdUserMacThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdUserMacThroughputHelper ();


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

}; // end of class SatStatsFwdUserMacThroughputHelper


// FORWARD FEEDER LINK PHY-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link PHY-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederPhyThroughputHelper> s = Create<SatStatsFwdFeederPhyThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics include control messages.
 */
class SatStatsFwdFeederPhyThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdFeederPhyThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdFeederPhyThroughputHelper ();


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

}; // end of class SatStatsFwdFeederPhyThroughputHelper


// FORWARD USER LINK PHY-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link PHY-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserPhyThroughputHelper> s = Create<SatStatsFwdUserPhyThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics include control messages.
 */
class SatStatsFwdUserPhyThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdUserPhyThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdUserPhyThroughputHelper ();


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

}; // end of class SatStatsFwdUserPhyThroughputHelper


// RETURN LINK APPLICATION-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return link application-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnAppThroughputHelper> s = Create<SatStatsRtnAppThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnAppThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnAppThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnAppThroughputHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param packet received packet data.
   * \param from the InetSocketAddress of the sender of the packet.
   */
  void Ipv4Callback (Ptr<const Packet> packet, const Address &from);

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

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

}; // end of class SatStatsRtnAppThroughputHelper


// RETURN LINK DEVICE-LEVEL ///////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return link device-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnDevThroughputHelper> s = Create<SatStatsRtnDevThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnDevThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnDevThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnDevThroughputHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnDevThroughputHelper


// RETURN FEEDER LINK MAC-LEVEL //////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link MAC-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederMacThroughputHelper> s = Create<SatStatsRtnFeederMacThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics does not include control messages.
 */
class SatStatsRtnFeederMacThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnFeederMacThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnFeederMacThroughputHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnFeederMacThroughputHelper


// RETURN USER LINK MAC-LEVEL //////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link MAC-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserMacThroughputHelper> s = Create<SatStatsRtnUserMacThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics does not include control messages.
 */
class SatStatsRtnUserMacThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnUserMacThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnUserMacThroughputHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnUserMacThroughputHelper


// RETURN FEEDER LINK PHY-LEVEL //////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link PHY-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederPhyThroughputHelper> s = Create<SatStatsRtnFeederPhyThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics does not include control messages.
 */
class SatStatsRtnFeederPhyThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnFeederPhyThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnFeederPhyThroughputHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnFeederPhyThroughputHelper


// RETURN USER LINK PHY-LEVEL //////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link PHY-level throughput statistics from a
 *        satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserPhyThroughputHelper> s = Create<SatStatsRtnUserPhyThroughputHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 *
 * \todo This statistics does not include control messages.
 */
class SatStatsRtnUserPhyThroughputHelper : public SatStatsThroughputHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnUserPhyThroughputHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnUserPhyThroughputHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsThroughputHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnUserPhyThroughputHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_THROUGHPUT_HELPER_H */
