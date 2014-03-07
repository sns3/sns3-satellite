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

#ifndef SATELLITE_STATS_HELPER_CONTAINER_H
#define SATELLITE_STATS_HELPER_CONTAINER_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/satellite-stats-helper.h>
#include <list>


namespace ns3 {

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

  // FORWARD LINK APPLICATION-LEVEL PACKET DELAY STATISTICS ///////////////////

  /**
   * \brief
   * \param outputType
   */
  void AddGlobalFwdAppDelay (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerGwFwdAppDelay (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerBeamFwdAppDelay (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerUtFwdAppDelay (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerUtUserFwdAppDelay (SatStatsHelper::OutputType_t outputType);

  // FORWARD LINK THROUGHPUT STATISTICS ///////////////////////////////////////

  /**
   * \brief
   * \param outputType
   */
  void AddGlobalFwdThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerGwFwdThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerBeamFwdThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerUtFwdThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerUtUserFwdThroughput (SatStatsHelper::OutputType_t outputType);

  // RETURN LINK THROUGHPUT STATISTICS ////////////////////////////////////////

  /**
   * \brief
   * \param outputType
   */
  void AddGlobalRtnThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerGwRtnThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerBeamRtnThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerUtRtnThroughput (SatStatsHelper::OutputType_t outputType);

  /**
   * \brief
   * \param outputType
   */
  void AddPerUtUserRtnThroughput (SatStatsHelper::OutputType_t outputType);

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


#endif /* SATELLITE_STATS_HELPER_CONTAINER_H */
