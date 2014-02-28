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

#ifndef SATELLITE_STATS_HELPER_H
#define SATELLITE_STATS_HELPER_H

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>
#include <ns3/object.h>
#include <ns3/node-container.h>
#include <list>
#include <map>


namespace ns3 {


// SATELLITE STATS HELPER /////////////////////////////////////////////////////

class SatHelper;
class Node;
class Probe;
class DataCollectionObject;

/**
 * \brief Abstract class
 */
class SatStatsHelper : public SimpleRefCount<SatStatsHelper>
{
public:
  /**
   * \enum IdentifierType_t
   * \brief
   */
  typedef enum
  {
    IDENTIFIER_GLOBAL = 0,
    IDENTIFIER_GW,
    IDENTIFIER_BEAM,
    IDENTIFIER_UT,
    IDENTIFIER_UT_USER
  } IdentifierType_t;

  /**
   * \param identifierType
   * \return
   */
  static std::string GetIdentiferTypeName (IdentifierType_t identifierType);

  /**
   * \enum OutputType_t
   * \brief
   */
  typedef enum
  {
    OUTPUT_NONE = 0,
    OUTPUT_SCALAR_FILE,
    OUTPUT_SCATTER_FILE,
    OUTPUT_HISTOGRAM_FILE,
    OUTPUT_PDF_FILE,        // probability distribution function
    OUTPUT_CDF_FILE,        // cumulative distribution function
    OUTPUT_SCALAR_PLOT,
    OUTPUT_SCATTER_PLOT,
    OUTPUT_HISTOGRAM_PLOT,
    OUTPUT_PDF_PLOT,        // probability distribution function
    OUTPUT_CDF_PLOT,        // cumulative distribution function
  } OutputType_t;

  /**
   * \param outputType
   * \return
   */
  static std::string GetOutputTypeName (OutputType_t outputType);

  /**
   * \brief
   * \param satHelper
   */
  SatStatsHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsHelper ();

  /**
   * \brief Install probes, collectors, and aggregators.
   *
   * Behaviour should be implemented by child class in DoInstall().
   */
  void Install ();

  /**
   * \param name
   */
  void SetName (std::string name);

  /**
   * \return
   */
  std::string GetName () const;

  /**
   * \param identifierType
   * \warning Does not have any effect if invoked after Install().
   */
  void SetIdentifierType (IdentifierType_t identifierType);

  /**
   * \return
   */
  IdentifierType_t GetIdentifierType () const;

  /**
   * \param outputType
   * \warning Does not have any effect if invoked after Install().
   */
  void SetOutputType (OutputType_t outputType);

  /**
   * \return
   */
  OutputType_t GetOutputType () const;

  /**
   * \return
   */
  Ptr<const SatHelper> GetSatHelper () const;

  /**
   * \return
   */
  Ptr<DataCollectionObject> GetAggregator () const;

  /**
   * \brief
   * \param ut
   * \return
   */
  static uint32_t GetUtId (Ptr<Node> ut);

  /**
   * \brief
   * \param utUser
   * \return
   */
  static uint32_t GetUtUserId (Ptr<Node> utUser);

protected:
  /**
   * \brief
   */
  virtual void DoInstall () = 0;

  ///
  typedef std::map<uint32_t, Ptr<DataCollectionObject> > CollectorMap_t;

  /**
   * \brief Create the aggregator according to the output type.
   */
  virtual void CreateAggregator ();

  /**
   * \brief
   * \param collectorMap a map of collectors where the newly created collectors
   *                     will be inserted into.
   * \return number of collectors created.
   */
  virtual uint32_t CreateTerminalCollectors (CollectorMap_t &collectorMap) const;

  /**
   * \brief Create a collector for each identifier.
   * \param collectorTypeId the type of collector to be created.
   * \param collectorMap a map of collectors where the newly created collectors
   *                     will be inserted into.
   * \return number of collectors created.
   *
   * The currently active identifier type (as returned by GetIdentifierType()
   * and indicated by the `IdentifierType` attribute) determines the number of
   * collectors created by this method. The active identifier type can be
   * modified by invoking the SetIdentifierType() method or modifying the
   * `IdentifierType` attribute.
   */
  virtual uint32_t CreateCollectors (std::string collectorTypeId,
                                     CollectorMap_t &collectorMap) const;

  /**
   * \brief Create a new probe and connect it to a specified object and
   *        collector.
   * \param objectTypeId
   * \param objectTraceSourceName
   * \param probeName
   * \param probeTypeId
   * \param probeTraceSourceName
   * \param identifier
   * \param collectorMap
   * \param collectorTraceSink
   * \return pointer to the newly created probe, connected to the specified
   *         object and the right connector, or zero if failure happens.
   */
  template<typename R, typename C, typename P1, typename P2>
  Ptr<Probe> InstallProbe (Ptr<Object> object,
                           std::string objectTypeId,
                           std::string objectTraceSourceName,
                           std::string probeName,
                           std::string probeTypeId,
                           std::string probeTraceSourceName,
                           uint32_t identifier,
                           CollectorMap_t &collectorMap,
                           R (C::*collectorTraceSink) (P1, P2)) const;

  /**
   * \brief
   * \param sourceCollectorMap
   * \param sourceCollectorTypeId
   * \param traceSourceName
   * \param targetCollectorMap
   * \param targetCollectorTypeId
   * \param traceSink
   * \return
   */
  template<typename R, typename C, typename P1, typename P2>
  bool ConnectCollectorToCollector (CollectorMap_t &sourceCollectorMap,
                                    std::string sourceCollectorTypeId,
                                    std::string traceSourceName,
                                    CollectorMap_t &targetCollectorMap,
                                    std::string targetCollectorTypeId,
                                    R (C::*traceSink) (P1, P2)) const;

  /**
   * \brief
   * \param sourceCollectorMap
   * \param sourceCollectorTypeId
   * \param traceSourceName
   * \param targetCollector
   * \param targetCollectorTypeId
   * \param traceSink
   * \return
   */
  template<typename R, typename C, typename P1, typename P2>
  bool ConnectCollectorToCollector (CollectorMap_t &sourceCollectorMap,
                                    std::string sourceCollectorTypeId,
                                    std::string traceSourceName,
                                    Ptr<DataCollectionObject> targetCollector,
                                    std::string targetCollectorTypeId,
                                    R (C::*traceSink) (P1, P2)) const;

  /**
   * \brief
   * \param sourceCollector
   * \param sourceCollectorTypeId
   * \param traceSourceName
   * \param targetCollectorMap
   * \param targetCollectorTypeId
   * \param traceSink
   * \return
   */
  template<typename R, typename C, typename P1, typename P2>
  bool ConnectCollectorToCollector (Ptr<DataCollectionObject> sourceCollector,
                                    std::string sourceCollectorTypeId,
                                    std::string traceSourceName,
                                    CollectorMap_t &targetCollectorMap,
                                    std::string targetCollectorTypeId,
                                    R (C::*traceSink) (P1, P2)) const;

  /**
   * \brief
   * \param sourceCollector
   * \param sourceCollectorTypeId
   * \param traceSourceName
   * \param targetCollector
   * \param targetCollectorTypeId
   * \param traceSink
   * \return
   */
  template<typename R, typename C, typename P1, typename P2>
  bool ConnectCollectorToCollector (Ptr<DataCollectionObject> sourceCollector,
                                    std::string sourceCollectorTypeId,
                                    std::string traceSourceName,
                                    Ptr<DataCollectionObject> targetCollector,
                                    std::string targetCollectorTypeId,
                                    R (C::*traceSink) (P1, P2)) const;

  /*
  template<typename R, typename C, typename P1, typename V1>
  void ConnectCollectorToAggregator (CollectorMap_t &collectorMap,
                                     std::string collectorTraceSourceName,
                                     R (C::*aggregatorTraceSink) (P1, V1));
  template<typename R, typename C, typename P1, typename V1, typename V2>
  void ConnectCollectorToAggregator (CollectorMap_t &collectorMap,
                                     std::string collectorTraceSourceName,
                                     R (C::*aggregatorTraceSink) (P1, V1, V2));
  template<typename R, typename C, typename P1, typename V1, typename V2, typename V3>
  void ConnectCollectorToAggregator (CollectorMap_t &collectorMap,
                                     std::string collectorTraceSourceName,
                                     R (C::*aggregatorTraceSink) (P1, V1, V2, V3));
  template<typename R, typename C, typename P1, typename V1, typename V2, typename V3, typename V4>
  void ConnectCollectorToAggregator (CollectorMap_t &collectorMap,
                                     std::string collectorTraceSourceName,
                                     R (C::*aggregatorTraceSink) (P1, V1, V2, V3, V4));
  template<typename R, typename C, typename P1, typename V1, typename V2, typename V3, typename V4, typename V5>
  void ConnectCollectorToAggregator (CollectorMap_t &collectorMap,
                                     std::string collectorTraceSourceName,
                                     R (C::*aggregatorTraceSink) (P1, V1, V2, V3, V4, V5));
  template<typename R, typename C, typename P1, typename V1, typename V2, typename V3, typename V4, typename V5, typename V6>
  void ConnectCollectorToAggregator (CollectorMap_t &collectorMap,
                                     std::string collectorTraceSourceName,
                                     R (C::*aggregatorTraceSink) (P1, V1, V2, V3, V4, V5, V6));
  */

  /**
   * \param utUserNode
   * \return
   */
  virtual uint32_t GetUtUserIdentifier (Ptr<Node> utUserNode) const;

  /**
   * \param utNode
   * \return
   */
  virtual uint32_t GetUtIdentifier (Ptr<Node> utNode) const;

  /**
   * \param beamId
   * \return
   */
  virtual uint32_t GetBeamIdentifier (uint32_t beamId) const;

  /**
   * \param gwNode
   * \return
   */
  virtual uint32_t GetGwIdentifier (Ptr<Node> gwNode) const;

private:
  std::string                m_name;            ///<
  IdentifierType_t           m_identifierType;  ///<
  OutputType_t               m_outputType;      ///<
  bool                       m_isInstalled;     ///<
  Ptr<const SatHelper>       m_satHelper;       ///<
  Ptr<DataCollectionObject>  m_aggregator;      ///<

  // TODO: Add support for more than one aggregators.

}; // end of class SatStatsHelper


// SATELLITE STATS FWD THROUGHPUT HELPER //////////////////////////////////////

class SatHelper;
class Probe;
class DataCollectionObject;

/**
 * \brief
 */
class SatStatsFwdThroughputHelper : public SatStatsHelper
{
public:
  /**
   * \brief
   * \param satHelper
   */
  SatStatsFwdThroughputHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsFwdThroughputHelper ();

protected:
  // inherited from SatStatsHelper base class
  virtual void DoInstall ();

private:
  std::list<Ptr<Probe> > m_probes;

  // key: identifier ID
  CollectorMap_t m_intervalRateCollectors;
  CollectorMap_t m_outputCollectors;

}; // end of class SatStatsFwdThroughputHelper


// SATELLITE STATS HELPER CONTAINER ///////////////////////////////////////////

class SatHelper;

/**
 * \brief Container of SatStatsHelper instances.
 *
 * The container is initially empty. SatStatsHelper instances can be added into
 * the container using attributes or class methods.
 *
 * The names of these attributes and class methods follow the convention below:
 * - identifier (e.g., per UT user, per UT, per beam, per GW, etc.)
 * -
 *
 * The value of the attributes and the arguments of the class methods are the
 * desired output type (e.g., files, plots, etc.). For now, the only viable
 * output type is file.
 *
 * The output files will be named in certain pattern using the name set in
 * `Name` attribute or SetName method. The default name is "stat", which for
 * example will produce output files with the names
 * `stat-per-ut-throughput-scalar.txt`, `stat-per-ut-throughput-trace.txt`, etc.
 *
 * There are tons of those attributes and class methods, because we aim to
 * accommodate enabling specific statistics with one primitive operation (i.e.,
 * setting an attribute to true.
 */
class SatStatsHelperContainer : public Object
{
public:
  /**
   * \brief
   * \param satHelper
   */
  SatStatsHelperContainer (Ptr<const SatHelper> satHelper);

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \param name
   */
  void SetName (std::string name);

  /**
   * \return
   */
  std::string GetName () const;

  void AddPerUtUserFwdThroughput (SatStatsHelper::OutputType_t outputType);

  //void AddPerUtFwdThroughput (SatStatsHelper::OutputType_t outputType);

  //void AddPerBeamFwdThroughput (SatStatsHelper::OutputType_t outputType);

  //void AddPerGwFwdThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \param outputType
   * \return
   */
  static std::string GetOutputTypeSuffix (SatStatsHelper::OutputType_t outputType);

protected:
  // Inherited from Object base class
  virtual void DoDispose ();

private:

  Ptr<const SatHelper> m_satHelper;

  std::string m_name;

  std::list<Ptr<const SatStatsHelper> > m_stats;

}; // end of class StatStatsHelperContainer


} // end of namespace ns3


#endif /* SATELLITE_STATS_HELPER_H */
