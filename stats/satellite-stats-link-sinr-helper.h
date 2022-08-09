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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 *
 */

#ifndef SATELLITE_STATS_LINK_SINR_HELPER_H
#define SATELLITE_STATS_LINK_SINR_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/ptr.h>
#include <ns3/callback.h>
#include <ns3/collector-map.h>


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief Base class for link SINR statistics helpers.
 */
class SatStatsLinkSinrHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsLinkSinrHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsLinkSinrHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \param averagingMode average all samples before passing them to aggregator.
   */
  void SetAveragingMode (bool averagingMode);

  /**
   * \brief Set up several probes or other means of listeners and connect them
   *        to the collectors.
   */
  void InstallProbes ();

  /**
   * \brief Receive inputs from trace sources and forward them to the collector.
   * \param sinrDb SINR value in dB.
   */
  void SinrCallback (double sinrDb, const Address &);

  /**
   * \return
   */
  Callback<void, double, const Address &> GetTraceSinkCallback () const;

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
   * \param sinrDb
   * \param identifier
   */
  void PassSampleToCollector (double sinrDb, uint32_t identifier);

  /// Maintains a list of collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The collector created by this helper.
  Ptr<DataCollectionObject> m_collector;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

  /// The final collector utilized in averaged output (histogram, PDF, and CDF).
  Ptr<DistributionCollector> m_averagingCollector;

  /// Map of address and the identifier associated with it (for return link).
  std::map<const Address, uint32_t> m_identifierMap;

private:
  ///
  Callback<void, double, const Address &> m_traceSinkCallback;

  bool m_averagingMode;  ///< `AveragingMode` attribute.

}; // end of class SatStatsLinkSinrHelper


// FORWARD FEEDER LINK ////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link SINR statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederLinkSinrHelper> s = Create<SatStatsFwdFeederLinkSinrHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederLinkSinrHelper : public SatStatsLinkSinrHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdFeederLinkSinrHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdFeederLinkSinrHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkSinrHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsFwdFeederLinkSinrHelper


// FORWARD USER LINK //////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link SINR statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserLinkSinrHelper> s = Create<SatStatsFwdUserLinkSinrHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserLinkSinrHelper : public SatStatsLinkSinrHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdUserLinkSinrHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdUserLinkSinrHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkSinrHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsFwdUserLinkSinrHelper


// RETURN FEEDER LINK /////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link SINR statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederLinkSinrHelper> s = Create<SatStatsRtnFeederLinkSinrHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederLinkSinrHelper : public SatStatsLinkSinrHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnFeederLinkSinrHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnFeederLinkSinrHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkSinrHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnFeederLinkSinrHelper


// RETURN USER LINK ///////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link SINR statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserLinkSinrHelper> s = Create<SatStatsRtnUserLinkSinrHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserLinkSinrHelper : public SatStatsLinkSinrHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnUserLinkSinrHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnUserLinkSinrHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkSinrHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnUserLinkSinrHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_LINK_SINR_HELPER_H */
