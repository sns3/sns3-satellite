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

#ifndef SATELLITE_STATS_JITTER_HELPER_H
#define SATELLITE_STATS_JITTER_HELPER_H

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
 * \brief Base class for jitter statistics helpers.
 */
class SatStatsJitterHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsJitterHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsJitterHelper ();


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
   * \param jitter packet jitter.
   * \param from the address of the sender of the packet.
   *
   * Used in return link statistics. DoInstallProbes() is expected to connect
   * the right trace sources to this method.
   */
  void RxJitterCallback (const Time &jitter, const Address &from);

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
   * \param jitter
   * \param identifier
   */
  void PassSampleToCollector (const Time &jitter, uint32_t identifier);

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

}; // end of class SatStatsJitterHelper


// FORWARD LINK DEVICE-LEVEL /////////////////////////////////////////////////////

class SatStatsFwdDevJitterHelper : public SatStatsJitterHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdDevJitterHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdDevJitterHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsJitterHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdDevJitterHelper


// FORWARD LINK MAC-LEVEL /////////////////////////////////////////////////////

class SatStatsFwdMacJitterHelper : public SatStatsJitterHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdMacJitterHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdMacJitterHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsJitterHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdMacJitterHelper


// FORWARD LINK PHY-LEVEL /////////////////////////////////////////////////////

class SatStatsFwdPhyJitterHelper : public SatStatsJitterHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdPhyJitterHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdPhyJitterHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsJitterHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

}; // end of class SatStatsFwdPhyJitterHelper


// RETURN LINK DEVICE-LEVEL //////////////////////////////////////////////

class SatStatsRtnDevJitterHelper : public SatStatsJitterHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnDevJitterHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnDevJitterHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsJitterHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnDevJitterHelper


// RETURN LINK MAC-LEVEL //////////////////////////////////////////////

class SatStatsRtnMacJitterHelper : public SatStatsJitterHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnMacJitterHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnMacJitterHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsJitterHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnMacJitterHelper


// RETURN LINK PHY-LEVEL //////////////////////////////////////////////

class SatStatsRtnPhyJitterHelper : public SatStatsJitterHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnPhyJitterHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnPhyJitterHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsJitterHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnPhyJitterHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_JITTER_HELPER_H */
