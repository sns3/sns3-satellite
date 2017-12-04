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
class DistributionCollector;

/**
 * \ingroup satstats
 * \brief Base class for delay statistics helpers.
 */
class SatStatsDelayHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsDelayHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsDelayHelper ();


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
  void RxDelayCallback (const Time &delay, const Address &from);

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

  /**
   * \brief Connect the probe to the right collector.
   * \param probe
   * \param identifier
   */
  bool ConnectProbeToCollector (Ptr<Probe> probe, uint32_t identifier);

  /**
   * \brief Find a collector with the right identifier and pass a sample data
   *        to it.
   * \param delay
   * \param identifier
   */
  void PassSampleToCollector (const Time &delay, uint32_t identifier);

  /// Maintains a list of collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The final collector utilized in averaged output (histogram, PDF, and CDF).
  Ptr<DistributionCollector> m_averagingCollector;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

  /// Map of address and the identifier associated with it (for return link).
  std::map<const Address, uint32_t> m_identifierMap;

private:
  bool m_averagingMode;  ///< `AveragingMode` attribute.

}; // end of class SatStatsDelayHelper


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


  /**
   * Destructor for SatStatsFwdAppDelayHelper.
   */
  virtual ~SatStatsFwdAppDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param helper Pointer to the delay statistics collector helper
   * \param identifier Identifier used to group statistics.
   * \param packet the received packet, expected to have been tagged with
   *               TrafficTimeTag.
   * \param from the InetSocketAddress of the sender of the packet.
   */
  static void RxCallback (Ptr<SatStatsFwdAppDelayHelper> helper,
                          uint32_t identifier,
                          Ptr<const Packet> packet,
                          const Address &from);

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdAppDelayHelper


// FORWARD LINK DEVICE-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward link device-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdDevDelayHelper> s = Create<SatStatsFwdDevDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdDevDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdDevDelayHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdDevDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdDevDelayHelper


// FORWARD LINK MAC-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward link MAC-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdMacDelayHelper> s = Create<SatStatsFwdMacDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdMacDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdMacDelayHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdMacDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdMacDelayHelper


// FORWARD LINK PHY-LEVEL /////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward link PHY-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdPhyDelayHelper> s = Create<SatStatsFwdPhyDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdPhyDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdPhyDelayHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdPhyDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdPhyDelayHelper


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


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnAppDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param packet the received packet, expected to have been tagged with
   *               TrafficTimeTag.
   * \param from the InetSocketAddress of the sender of the packet.
   */
  void RxCallback (Ptr<const Packet> packet, const Address &from);

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param delay packet delay.
   * \param from the InetSocketAddress of the sender of the packet.
   */
  void Ipv4Callback (const Time &delay, const Address &from);

protected:
  // inherited from SatStatsDelayHelper base class
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

}; // end of class SatStatsRtnAppDelayHelper


// RETURN LINK DEVICE-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return link device-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnDevDelayHelper> s = Create<SatStatsRtnDevDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnDevDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnDevDelayHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnDevDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnDevDelayHelper


// RETURN LINK MAC-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return link MAC-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnMacDelayHelper> s = Create<SatStatsRtnMacDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnMacDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnMacDelayHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnMacDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnMacDelayHelper


// RETURN LINK PHY-LEVEL //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return link PHY-level delay statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnPhyDelayHelper> s = Create<SatStatsRtnPhyDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnPhyDelayHelper : public SatStatsDelayHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnPhyDelayHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnPhyDelayHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnPhyDelayHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_DELAY_HELPER_H */
