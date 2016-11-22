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

#ifndef SATELLITE_STATS_FRAME_LOAD_HELPER_H
#define SATELLITE_STATS_FRAME_LOAD_HELPER_H

#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>
#include <ns3/collector-map.h>
#include <list>
#include <map>
#include <sstream>


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class DataCollectionObject;
class ScalarCollector;

/**
 * \ingroup satstats
 * \brief Helper for frame load statistics. Base class.
 */
class SatStatsFrameLoadHelper : public SatStatsHelper
{
public:
  /**
   * \enum UnitType_t
   * \brief
   */
  typedef enum
  {
    /// Number of allocated symbols over total number of symbols in the frame.
    UNIT_SYMBOLS = 0,
    /// Number of scheduled users (i.e., UT).
    UNIT_USERS,
  } UnitType_t;

  /**
   * \param unitType
   * \return
   */
  static std::string GetUnitTypeName (UnitType_t unitType);

  // inherited from SatStatsHelper base class
  SatStatsFrameLoadHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFrameLoadHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \param unitType
   */
  void SetUnitType (UnitType_t unitType);

  /**
   * \return
   */
  UnitType_t GetUnitType () const;

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();
  std::string GetIdentifierHeading (std::string dataLabel) const;

private:
  /**
   * \param object
   * \param identifier
   * \param traceSink
   * \return
   */
  template<typename P, typename V>
  bool SetupProbe (Ptr<Object> object,
                   uint32_t identifier,
                   void (SatStatsFrameLoadHelper::*traceSink)(std::string, uint32_t, V));

  /**
   * \param context
   * \param frameId
   * \param loadRatio
   */
  void FrameSymbolLoadCallback (std::string context, uint32_t frameId, double loadRatio);

  /**
   * \param context
   * \param frameId
   * \param utCount
   */
  void FrameUserLoadCallback (std::string context, uint32_t frameId, uint32_t utCount);

  /**
   * \param frameId
   * \param identifier
   * \return
   */
  Ptr<ScalarCollector> GetCollector (uint32_t frameId, std::string identifier);

  UnitType_t   m_unitType;               ///<
  std::string  m_shortLabel;             ///<
  std::string  m_longLabel;              ///<
  std::string  m_objectTraceSourceName;  ///<
  std::string  m_probeTraceSourceName;   ///<

  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

  /**
   * \brief Two-dimensional map of collectors, indexed by the frame ID and then
   *        by the identifier.
   */
  std::map<uint32_t, CollectorMap> m_collectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

}; // end of class SatStatsFrameLoadHelper


template<typename P, typename V>
bool
SatStatsFrameLoadHelper::SetupProbe (Ptr<Object> object,
                                     uint32_t identifier,
                                     void (SatStatsFrameLoadHelper::*traceSink)(std::string, uint32_t, V))
{
  std::ostringstream oss;
  oss << identifier;

  Ptr<P> probe = CreateObject<P> ();
  if (!probe->GetInstanceTypeId ().IsChildOf (TypeId::LookupByName ("ns3::Probe")))
    {
      NS_FATAL_ERROR ("The specified type is not a probe.");
    }

  // Connect the object to the probe and then to the callback.
  if (probe->ConnectByObject (m_objectTraceSourceName, object)
      && probe->TraceConnect (m_probeTraceSourceName,
                              oss.str (),
                              MakeCallback (traceSink, this)))
    {
      m_probes.push_back (probe->template GetObject<Probe> ());
      return true;
    }
  else
    {
      return false;
    }
}



// IN SYMBOL UNIT /////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Helper for frame symbol load statistics.
 */
class SatStatsFrameSymbolLoadHelper : public SatStatsFrameLoadHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFrameSymbolLoadHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFrameSymbolLoadHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

}; // end of class SatStatsFrameSymbolLoadHelper


// IN USER UNIT ///////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Helper for frame user load statistics.
 */
class SatStatsFrameUserLoadHelper : public SatStatsFrameLoadHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFrameUserLoadHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFrameUserLoadHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

}; // end of class SatStatsFrameUserLoadHelper



} // end of namespace ns3


#endif /* SATELLITE_STATS_FRAME_LOAD_HELPER_H */
