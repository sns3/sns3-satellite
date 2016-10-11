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

#ifndef SATELLITE_STATS_QUEUE_HELPER_H
#define SATELLITE_STATS_QUEUE_HELPER_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/satellite-stats-helper.h>
#include <ns3/collector-map.h>
#include <list>
#include <utility>


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Mac48Address;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief Helper for queue statistics. Base class.
 */
class SatStatsQueueHelper : public SatStatsHelper
{
public:
  /**
   * \enum UnitType_t
   * \brief
   */
  typedef enum
  {
    UNIT_BYTES = 0,
    UNIT_NUMBER_OF_PACKETS,
  } UnitType_t;

  /**
   * \param unitType
   * \return
   */
  static std::string GetUnitTypeName (UnitType_t unitType);

  // inherited from SatStatsHelper base class
  SatStatsQueueHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsQueueHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \param pollInterval
   */
  void SetPollInterval (Time pollInterval);

  /**
   * \return
   */
  Time GetPollInterval () const;

  /**
   * \param unitType
   */
  void SetUnitType (UnitType_t unitType);

  /**
   * \return
   */
  UnitType_t GetUnitType () const;

  /**
   * \brief Identify the list of source of queue events.
   */
  void EnlistSource ();

  /**
   * \brief Retrieve the queue size of every relevant encapsulator and push the
   *        values to the right collectors.
   */
  void Poll ();

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

  /**
   * \brief
   */
  virtual void DoEnlistSource () = 0;

  /**
   * \brief Retrieve the queue size of every relevant encapsulator and push the
   *        values to the right collectors.
   */
  virtual void DoPoll () = 0;

  /**
   * \param identifier
   * \param value
   */
  void PushToCollector (uint32_t identifier, uint32_t value);

  /// Maintains a list of collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

private:
  Time         m_pollInterval;  ///< `PollInterval` attribute.
  UnitType_t   m_unitType;      ///<
  std::string  m_shortLabel;    ///<
  std::string  m_longLabel;     ///<

}; // end of class SatStatsQueueHelper


// FORWARD LINK ///////////////////////////////////////////////////////////////

class SatLlc;

/**
 * \ingroup satstats
 * \brief Helper for forward link queue statistics. Base class for forward link.
 */
class SatStatsFwdQueueHelper : public SatStatsQueueHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdQueueHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdQueueHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsQueueHelper base class
  void DoEnlistSource ();
  void DoPoll ();

private:
  ///
  typedef std::list<std::pair<Mac48Address, uint32_t> > ListOfUt_t;

  /// Maintains a list of GW LLC, its UT address, and its identifier.
  std::list<std::pair<Ptr<SatLlc>, ListOfUt_t> > m_llc;

}; // end of class SatStatsFwdQueueHelper


/**
 * \ingroup satstats
 * \brief Helper for forward link queue statistics using byte as unit.
 */
class SatStatsFwdQueueBytesHelper : public SatStatsFwdQueueHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdQueueBytesHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdQueueBytesHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

}; // end of class SatStatsFwdQueueBytesHelper


/**
 * \ingroup satstats
 * \brief Helper for forward link queue statistics using packet as unit.
 */
class SatStatsFwdQueuePacketsHelper : public SatStatsFwdQueueHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdQueuePacketsHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdQueuePacketsHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

}; // end of class SatStatsFwdQueuePacketsHelper


// RETURN LINK ////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Helper for return link queue statistics. Base class for return link.
 */
class SatStatsRtnQueueHelper : public SatStatsQueueHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnQueueHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnQueueHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsQueueHelper base class
  void DoEnlistSource ();
  void DoPoll ();

private:

  /// Maintains a list of UT LLC and its identifier.
  std::list<std::pair<Ptr<SatLlc>, uint32_t> > m_llc;

}; // end of class SatStatsRtnQueueHelper


/**
 * \ingroup satstats
 * \brief Helper for return link queue statistics using byte as unit.
 */
class SatStatsRtnQueueBytesHelper : public SatStatsRtnQueueHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnQueueBytesHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnQueueBytesHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

}; // end of class SatStatsRtnQueueBytesHelper


/**
 * \ingroup satstats
 * \brief Helper for return link queue statistics using packet as unit.
 */
class SatStatsRtnQueuePacketsHelper : public SatStatsRtnQueueHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnQueuePacketsHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnQueuePacketsHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

}; // end of class SatStatsRtnQueuePacketsHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_QUEUE_HELPER_H */
