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
    IDENTIFIER_UT_USER,
    IDENTIFIER_UT,
    IDENTIFIER_BEAM,
    IDENTIFIER_GW
  } IdentifierType_t;

  static std::string GetIdentiferTypeName (IdentifierType_t identifierType);

  /**
   * \enum OutputType_t
   * \brief
   */
  typedef enum
  {
    OUTPUT_NONE = 0,
    OUTPUT_SCALAR_FILE,
    OUTPUT_TRACE_FILE,
    OUTPUT_PDF_FILE,     // probability distribution function
    OUTPUT_CDF_FILE,     // cumulative distribution function
    OUTPUT_SCALAR_PLOT,
    OUTPUT_TRACE_PLOT,
    OUTPUT_PDF_PLOT,     // probability distribution function
    OUTPUT_CDF_PLOT,     // cumulative distribution function
  } OutputType_t;

  static std::string GetOutputTypeName (OutputType_t outputType);

  SatStatsHelper ();

  virtual ~SatStatsHelper ();

  void SetName (std::string);

  std::string GetName () const;

  void SetIdentifierType (IdentifierType_t identifierType);

  IdentifierType_t GetIdentifierType () const;

  void SetOutputType (OutputType_t outputType);

  OutputType_t GetOutputType () const;

  static uint32_t GetUtUserId (Ptr<Node> utUser);

protected:

  std::list<Ptr<Probe> > m_probes;

  // key: identifier ID
  std::map<uint32_t, Ptr<DataCollectionObject> > m_collectors;

  Ptr<DataCollectionObject> m_aggregator;

private:

  std::string m_name;

  IdentifierType_t m_identifierType;

  OutputType_t m_outputType;

}; // end of class SatStatsHelper


// SATELLITE STATS FWD THROUGHPUT HELPER //////////////////////////////////////

class SatHelper;

/**
 * \brief
 */
class SatStatsFwdThroughputHelper : public SatStatsHelper
{
public:

  SatStatsFwdThroughputHelper ();

  virtual ~SatStatsFwdThroughputHelper ();

  void Connect (Ptr<const SatHelper> satHelper);

private:



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

  SatStatsHelperContainer (Ptr<const SatHelper> satHelper);

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  void SetName (std::string);

  std::string GetName () const;

  void AddPerUtUserFwdThroughput (SatStatsHelper::OutputType_t outputType);

  //void AddPerUtFwdThroughput (SatStatsHelper::OutputType_t outputType);

  //void AddPerBeamFwdThroughput (SatStatsHelper::OutputType_t outputType);

  //void AddPerGwFwdThroughput (SatStatsHelper::OutputType_t outputType);

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
