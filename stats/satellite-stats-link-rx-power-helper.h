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

#ifndef SATELLITE_STATS_LINK_RX_POWER_HELPER_H
#define SATELLITE_STATS_LINK_RX_POWER_HELPER_H

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
 * \brief Abstract class inherited by SatStatsFwdFeederLinkRxPowerHelpe, SatStatsFwdUserLinkRxPowerHelper,
 * SatStatsRtnFeederLinkRxPowerHelper and SatStatsRtnFeederLinkRxPowerHelper.
 */
class SatStatsLinkRxPowerHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsLinkRxPowerHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsLinkRxPowerHelper ();


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
   * \param rxPowerDb Rx power value in dB.
   */
  void RxPowerCallback (double rxPowerDb);

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

}; // end of class SatStatsLinkRxPowerHelper


// FORWARD FEEDER LINK ////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward feeder link Rx power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdFeederLinkRxPowerHelper> s = Create<SatStatsFwdFeederLinkRxPowerHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdFeederLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdFeederLinkRxPowerHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdFeederLinkRxPowerHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkRxPowerHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsFwdFeederLinkRxPowerHelper


// FORWARD USER LINK //////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce forward user link Rx power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdUserLinkRxPowerHelper> s = Create<SatStatsFwdUserLinkRxPowerHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdUserLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdUserLinkRxPowerHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdUserLinkRxPowerHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkRxPowerHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsFwdUserLinkRxPowerHelper


// RETURN FEEDER LINK /////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return feeder link Rx power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnFeederLinkRxPowerHelper> s = Create<SatStatsRtnFeederLinkRxPowerHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnFeederLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnFeederLinkRxPowerHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnFeederLinkRxPowerHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkRxPowerHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnFeederLinkRxPowerHelper


// RETURN USER LINK ///////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return user link Rx power statistics from a satellite
 *        module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnUserLinkRxPowerHelper> s = Create<SatStatsRtnUserLinkRxPowerHelper> (satHelper);
 * s->SetName ("name");
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnUserLinkRxPowerHelper : public SatStatsLinkRxPowerHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnUserLinkRxPowerHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnUserLinkRxPowerHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsLinkRxPowerHelper base class
  void DoInstallProbes ();

}; // end of class SatStatsRtnUserLinkRxPowerHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_LINK_RX_POWER_HELPER_H */
