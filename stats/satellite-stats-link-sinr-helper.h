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

#ifndef SATELLITE_STATS_LINK_SINR_HELPER_H
#define SATELLITE_STATS_LINK_SINR_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/ptr.h>
#include <ns3/callback.h>


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
   * \brief Set up several probes or other means of listeners and connect them
   *        to the collectors.
   */
  void InstallProbes ();

  /**
   * \brief Receive inputs from trace sources and forward them to the collector.
   * \param sinrDb SINR value in dB.
   */
  void SinrCallback (double sinrDb);

  /**
   * \return
   */
  Callback<void, double> GetTraceSinkCallback () const;

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

  /**
   * \brief
   */
  virtual void DoInstallProbes () = 0;

  /// The collector created by this helper.
  Ptr<DataCollectionObject> m_collector;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

private:
  ///
  Callback<void, double> m_traceSinkCallback;

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
