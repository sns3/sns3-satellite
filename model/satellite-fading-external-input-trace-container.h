/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */


#ifndef SATELLITE_FADING_EXTERNAL_INPUT_TRACE_CONTAINER_H_
#define SATELLITE_FADING_EXTERNAL_INPUT_TRACE_CONTAINER_H_

#include <map>
#include <string>
#include "ns3/object.h"
#include "ns3/mobility-model.h"
#include "satellite-enums.h"
#include "geo-coordinate.h"
#include "satellite-fading-external-input-trace.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Container class for external input fading traces. The class
 * provides an interface for accessing the correct fading trace object.
 */
class SatFadingExternalInputTraceContainer : public Object
{
public:
  /**
   * Definitions for different modes of using trace input files
   */
  typedef enum
  {
    LIST_MODE,      //!< LIST_MODE
    POSITION_MODE,  //!< POSITION_MODE
    RANDOM_MODE     //!< RANDOM_MODE
  } InputMode_t;

  /**
   * Define type ChannelTracePair_t
   */
  typedef std::pair<Ptr<SatFadingExternalInputTrace>, Ptr<SatFadingExternalInputTrace> >  ChannelTracePair_t;

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Constructor
   */
  SatFadingExternalInputTraceContainer ();

  /**
   * \brief Destructor
   */
  ~SatFadingExternalInputTraceContainer ();

  /**
   * Get method for getting a proper fading trace
   *
   * \param nodeId GW or UT Node id (from SatIdMapper)
   * \param channelType Channel type
   * \param mobility Mobility of the node
   * \return Channel fading trace for a certain node and channel.
   */
  Ptr<SatFadingExternalInputTrace> GetFadingTrace (uint32_t nodeId, SatEnums::ChannelType_t channelType, Ptr<MobilityModel> mobility);

  /**
   * \brief A method to test that the fading traces are according to
   * assumptions.
   * \param numOfUts number of UTs to test
   * \param numOfGws number of GWs to test
   * \return boolean value indicating success or failure
   */
  bool TestFadingTraces (uint32_t numOfUts, uint32_t numOfGws);

private:
  typedef std::pair <std::string, GeoCoordinate > TraceFileContainerItem_t;
  typedef std::vector<TraceFileContainerItem_t> TraceFileContainer_t;

  typedef std::map<std::string, Ptr<SatFadingExternalInputTrace> > TraceInputContainer_t;

  /**
   * Container of the UT fading traces
   */
  std::map< uint32_t, ChannelTracePair_t> m_utFadingMap;

  /**
   * Container of the GW fading traces
   */
  std::map< uint32_t, ChannelTracePair_t> m_gwFadingMap;

  /**
   * \brief Input mode to read trace files form given index table (file) for UTs
   */
  InputMode_t m_utInputMode;

  /**
   * The name of file which defines the index table to be used for trace input of forward down link for UT.
   * Index table defines files used as trace input sources.
   */
  std::string m_utFwdDownIndexFileName;

  /**
   * The name of file which defines the index table to be used for trace input of return up link for UTs.
   * Index table defines files used as trace input sources.
   */
  std::string m_utRtnUpIndexFileName;

  /**
   * The name of file which defines the index table to be used for trace input of forward up link for GWs.
   * Index table defines files used as trace input sources.
   */
  std::string m_gwFwdUpIndexFileName;

  /**
   * The name of file which defines the index table to be used for trace input of forward up link for GWs.
   * Index table defines files used as trace input sources.
   */
  std::string m_gwRtnDownIndexFileName;

  /**
   * UT forward down link trace file names
   */
  TraceFileContainer_t  m_utFwdDownFileNames;

  /**
   * UT return up link trace file names
   */
  TraceFileContainer_t  m_utRtnUpFileNames;

  /**
   * GW forward up link trace file names
   */
  TraceFileContainer_t  m_gwFwdUpFileNames;

  /**
   * GW return down link trace file names
   */
  TraceFileContainer_t  m_gwRtnDownFileNames;

  /**
   * Loaded trace files
   */
  TraceInputContainer_t m_loadedTraces;

  /// flag telling if index trace files are already loaded
  bool m_indexFilesLoaded;

  /// data path to find trace input files
  std::string m_dataPath;

  /// Maximum distance allowed to the external fading trace source
  double m_maxDistanceToFading;

  /**
   * Initialize index files
   */
  void LoadIndexFiles ();

  /**
   * Create new UT fading trace
   * \param utId  Id of the UT (from SatIdMapper)
   * \param mobility Mobility for given UT node
   */
  void CreateUtFadingTrace (uint32_t utId, Ptr<MobilityModel> mobility);

  /**
   * Create new GW fading trace
   * \param gwId  Id of the GW (from SatIdMapper)
   * \param mobility Mobility for given GW node
   */
  void CreateGwFadingTrace (uint32_t gwId, Ptr<MobilityModel> mobility);

  /**
   * Read trace file information from given index file.
   *
   * \param indexFile Index file to parse for trace file names
   * \param container Container reference to store found trace file info
   */
  void ReadIndexFile (std::string indexFile, TraceFileContainer_t& container);

  /**
   * Create (or load) fading trace source for the requested UT/GW.
   *
   * \param fileType Type of the trace file
   * \param inputMode used when reading input traces from trace file
   * \param container Container reference to find out needed trace file info
   * \param id Id of the node GW or UT (from SatIdMapper)
   * \param mobility Mobility for given node
   * \return Created trace input (or found trace input if queried trace input already created)
   */
  Ptr<SatFadingExternalInputTrace> CreateFadingTrace (SatFadingExternalInputTrace::TraceFileType_e fileType, InputMode_t inputMode,
                                                      TraceFileContainer_t& container, uint32_t id, Ptr<MobilityModel> mobility);
  /**
   *  Find the nearest fading trace source file for the requested UT/GW based on given mobility.
   *  Only external fading sources which are closer than (equal to) maximum allowed distance are accepted.
   *  Maximum allowed distance is defined by attribute.
   *
   * \param container Container reference to find out needed trace file info
   * \param id Id of the node GW or UT (from SatIdMapper)
   * \param mobility Mobility for given node
   * \return The name of the nearest external fading source.
   */
  std::string FindSourceBasedOnPosition (TraceFileContainer_t& container, uint32_t id, Ptr<MobilityModel> mobility);
};

} // namespace ns3


#endif /* SATELLITE_FADING_EXTERNAL_INPUT_TRACE_CONTAINER_H_ */
