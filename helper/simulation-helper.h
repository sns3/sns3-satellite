/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
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
 * Author: Lauri Sormunen <lauri.sormunen@magister.fi>
 */

#ifndef SIMULATION_HELPER_H
#define SIMULATION_HELPER_H

#include <set>
#include <ns3/random-variable-stream.h>
#include <ns3/command-line.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-stats-helper-container.h>
#include <ns3/satellite-enums.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY (x)


namespace ns3 {

/**
 * \ingroup satellite
 * \brief A helper to make it easier to create example simulation cases.
 *
 * Example usage:
 *
 * Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("My satellite simulation");
 *
 * simulationHelper->SetDefaultValues ();
 *
 * simulationHelper->SetBeams ("28 46 48 50 59");
 * simulationHelper->SetUtCountPerBeam (20);
 * simulationHelper->SetUserCountPerUt (1);
 * simulationHelper->SetSimulationTime (300);
 *
 * You may also customize the attribute settings with variety of different
 * public methods grouping several attributes under specific features.
 *
 */
class SimulationHelper : public Object
{
public:
  /**
   * Default constructor, which is not used.
   */
  SimulationHelper ();

  /**
   * \brief Contructor for simulation helper.
   * \param simulationName Name of the simulation.
   */
  SimulationHelper (std::string simulationName);

  /**
   * \brief Destructor.
   */
  virtual ~SimulationHelper ();

  /**
   * \brief Disposing.
   */
  void DoDispose (void);

  /**
   * \brief Derived from Object.
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Derived from Object.
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Set default values shared by all examples using
   * SimulationHelper. Adjust to your needs.
   */
  void SetDefaultValues ();

  /**
   * \brief Set enabled beams (1-72) as a string.
   * \param beamList List of beams.
   * \example simulationHelper->SetBeams ("1 5 20 71")
   *          enables beams 1, 5, 20 and 71.
   */
  void SetBeams (const std::string& beamList);

  /**
   * \brief Set enabled beams (1-72) as a set.
   * \param beamSet List of beams.
   * \example simulationHelper->SetBeams ({1,2,3})
   *          enables beams 1, 2 and 3..
   */
  void SetBeamSet (std::set<uint32_t> beamSet);

  /**
   * \brief Set UT count per beam.
   * \param count Number of UTs per beam.
   */
  void SetUtCountPerBeam (uint32_t count);

  /**
   * \brief Set UT count per beam to be taken from a random variable stream.
   * \param rs RandomVariableStream to be used, must implement GetInteger.
   */
  void SetUtCountPerBeam (Ptr<RandomVariableStream> rs);

  /**
   * \brief Set user count per UT.
   * \param count Number of users per UT.
   */
  void SetUserCountPerUt (uint32_t count);

  /**
   * \brief Set UT count per beam to be taken from a random variable stream.
   * \param rs RandomVariableStream to be used, must implement GetInteger.
   */
  void SetUserCountPerUt (Ptr<RandomVariableStream> rs);

  /**
   * \brief Set the number of GW users in the scenario.
   * Must be called before creation of satellite scenario.
   * \param gwUserCount
   */
  void SetGwUserCount (uint32_t gwUserCount);

  /**
   * \brief Set simulation time in seconds.
   * \param seconds
   */
  void SetSimulationTime (double seconds);

  /**
   * \brief Set simulation time
   * \param time
   */
  inline void SetSimulationTime (Time time)
  {
    m_simTime = time;
  }

  /**
   * \brief Set ideal channel/physical layer parameterization.
   */
  void SetIdealPhyParameterization ();

  /**
   * \brief Enable ACM for a simulation direction.
   * \param dir Direction
   */
  void EnableAcm (SatEnums::SatLinkDir_t dir);

  /**
   * \brief Disable ACM for a simulation direction.
   * \param dir Direction
   */
  void DisableAcm (SatEnums::SatLinkDir_t dir);

  /**
   * \brief Disable all capacity allocation categories: CRA/VBDC/RBDC
   */
  void DisableAllCapacityAssignmentCategories ();

  /**
   * \brief Enable only CRA for a given RC index
   * \param rcIndex Request class index
   * \param rateKbps CRA rate in kbps
   */
  void EnableOnlyConstantRate (uint32_t rcIndex, double rateKbps);

  /**
   * \brief Enable only RBDC for a given RC index
   * \param rcIndex Request class index
   */
  void EnableOnlyRbdc (uint32_t rcIndex);

  /**
   * \brief Enable only VBDC for a given RC index
   * \param rcIndex Request class index
   */
  void EnableOnlyVbdc (uint32_t rcIndex);

  /**
   * \brief Enable free capacity allocation
   */
  void EnableFca ();

  /**
   * \brief Disable free capacity allocation
   */
  void DisableFca ();

  /**
   * \brief Enable periodical control slots
   */
  void EnablePeriodicalControlSlots (Time periodicity);

  /**
   * \brief Enable ARQ
   */
  void EnableArq (SatEnums::SatLinkDir_t dir);

  /**
   * \brief Disable random access
   */
  void DisableRandomAccess ();

  /**
   * \brief Enable slotted ALOHA random access
   */
  void EnableSlottedAloha ();

  /**
   * \brief Enable CRDSA random access
   */
  void EnableCrdsa ();

  /**
   * \brief Configure a frame for a certain superframe id.
   * \param superFrameId Superframe id (currently always 0)
   * \param bw Frame bandwidth
   * \param carrierBw Bandwidth of the carriers within frame
   * \param rollOff Roll-off
   * \param carrierSpacing Carrier spacing between frames
   * \param isRandomAccess Is this a RA or DA frame
   */
  void ConfigureFrame (uint32_t superFrameId,
                       double bw,
                       double carrierBw,
                       double rollOff,
                       double carrierSpacing,
                       bool isRandomAccess = false);


  /**
   * \brief Configure the default setting for the forward
   * and return link frequencies.
   */
  void ConfigureFrequencyBands ();

  /**
   * \brief Enable external fading input.
   */
  void EnableExternalFadingInputTrace ();

  /**
   * \brief Enable all output traces
   */
  void EnableOutputTraces ();

  /**
   * \brief Configure all link budget related attributes
   */
  void ConfigureLinkBudget ();

  /**
   * \brief Set simulation error model.
   * \param em Error model.
   * \param errorRate Static error rate if constant error model used
   */
  void SetErrorModel (SatPhyRxCarrierConf::ErrorModel em,
                      double errorRate = 0.0);

  /**
   * \brief Set simulation interference model.
   * \param ifModel Interference model.
   * \param ifEliminationModel Interference elimination model.
   * \param constantIf Static interference if constant interference model used
   * \param residualSamplingError Sampling error if residual interference elimination model used
   */
  void SetInterferenceModel (SatPhyRxCarrierConf::InterferenceModel ifModel,
                             double constantIf = 0.0);

  /**
   * \brief Enables simulation progress logging.
   * Progress is logged to stdout in form of 'Progress: (current simulation time)/(simulation length)'.
   */
  void EnableProgressLogs ();

  /**
   * \brief Disables simulation progress logs.
   */
  void DisableProgressLogs ();

  /**
   * \brief Add default command line arguments for the simulation.
   * This method must be called between creation of the CommandLine helper and CommandLine::Parse () call.
   * \param cmd Reference to CommandLine helper instance
   */
  void AddDefaultUiArguments (CommandLine &cmd);

  /**
   * \brief Add default command line arguments for the simulation.
   * This method must be called between creation of the CommandLine helper and CommandLine::Parse () call.
   * \param cmd Reference to CommandLine helper instance
   * \param xmlInputFile Reference to string containing XML input file name
   */
  void AddDefaultUiArguments (CommandLine &cmd, std::string &xmlInputFile);

  /**
   * \brief Run the simulation
   */
  void RunSimulation ();

  /**
   * \brief Create the satellite scenario.
   * \return satHelper Satellite helper, which provides e.g. nodes for application installation.
   */
  Ptr<SatHelper> CreateSatScenario (SatHelper::PreDefinedScenario_t scenario = SatHelper::NONE);

  /**
   * \brief Create stats collectors and set default statistics settings
   * for both FWD and RTN links.
   */
  void CreateDefaultStats ();

  /**
   * \brief Create stats collectors if needed and set default statistics settings
   * for both FWD link. Adjust this method to your needs.
   */
  void CreateDefaultFwdLinkStats ();

  /**
   * \brief Create stats collectors if needed and set default statistics settings
   * for both RTN link. Adjust this method to your needs.
   */
  void CreateDefaultRtnLinkStats ();

  /**
   * \brief Set simulation output tag, which is the basename of the directory where
   * output files are stored.
   * \param tag Simulation tag.
   */
  void SetOutputTag (std::string tag);

  /**
   * \brief Force a output file path to this simulation instead of default
   * satellite/data/sims/.
   * \param path Output file path.
   */
  void SetOutputPath (std::string path);

  /**
   * \brief Configure this instance after reading input attributes from XML file
   * \param filePath full path to an Input XML file
   */
  void ConfigureAttributesFromFile (std::string filePath);

  /**
   * \brief Read input attributes from XML file
   * \param filePath full path to an Input XML file
   */
  void ReadInputAttributesFromFile (std::string filePath);

  /**
   * \brief Store all used attributes
   * \param fileName Output filename
   * \return string Output path
   */
  std::string StoreAttributesToFile (std::string fileName);

  /**
   * \brief Get simulation time
   * \return errorRate Simulation time
   */

  inline Time& GetSimTime ()
  {
    return m_simTime;
  }

  /**
   * \brief Set common UT position allocator for all beams.
   * The position allocator is used to draw UT position geocoordinates
   * in order of the beam IDs and number of UTs configured.
   * No validation is done.
   * \param posAllocator
   */
  void SetCommonUtPositionAllocator (Ptr<SatListPositionAllocator> posAllocator);

  /**
   * \brief Set a list position allocator for UTs of a specific beam.
   * The position allocator is used to draw UT position geocoordinates
   * when UTs are created for that specific beam-
   * \param beamId Beam ID
   * \param posAllocator List of UT positions, *must* match the number of UTs configured
   */
  void SetUtPositionAllocatorForBeam (uint32_t beamId, Ptr<SatListPositionAllocator> posAllocator);

  /**
   * \brief Enable reading UT list positions from input file
   * \param inputFile List postion file path (starting from data/)
   * \param checkBeam
   */
  void EnableUtListPositionsFromInputFile (std::string inputFile, bool checkBeams = true);

  /**
   * \brief If lower layer API access is required, use this to access SatHelper.
   * You MUST have called CreateSatScenario before calling this method.
   */
  inline Ptr<SatHelper> GetSatelliteHelper ()
  {
    return m_satHelper;
  }

  /**
   * \brief Get the statistics container of this helper. If does not exist, one is created.
   * \return Statistics helper container
   */
  Ptr<SatStatsHelperContainer> GetStatisticsContainer ();

  typedef enum
  {
    CBR,
    ONOFF,
    HTTP,
    NRTV,
  } TrafficModel_t;

  typedef enum
  {
    UDP, TCP
  } TransportLayerProtocol_t;

  typedef enum
  {
    RTN_LINK, FWD_LINK
  } TrafficDirection_t;

  /**
   * \brief Install simple traffic model from GW users to UT users or vice versa.
   * If traffic source/target is a single GW user, then its ID can be changed with
   * SetGwUserId ().
   * \param trafficModel Traffic models
   * \param protocol Transport layer protocol
   * \param direction Direction of traffic
   * \param startTime Application Start time
   * \param stopTime Application stop time
   * \param startDelay application start delay between each user
   * \param percentage ratio of users on which this traffic model will be installed
   */
  void InstallTrafficModel (TrafficModel_t trafficModel,
                            TransportLayerProtocol_t protocol,
                            TrafficDirection_t direction,
                            Time startTime,
                            Time stopTime,
                            Time startDelay,
                            double percentage);

  void InstallTrafficModel (TrafficModel_t trafficModel,
                            TransportLayerProtocol_t protocol,
                            TrafficDirection_t direction,
                            Time startTime,
                            Time stopTime,
                            Time startDelay)
  {
    InstallTrafficModel (
      trafficModel, protocol, direction,
      startTime, stopTime, startDelay, 1.0);
  }

  void InstallTrafficModel (TrafficModel_t trafficModel,
                            TransportLayerProtocol_t protocol,
                            TrafficDirection_t direction,
                            Time startTime,
                            Time stopTime)
  {
    InstallTrafficModel (
      trafficModel, protocol, direction,
      startTime,
      stopTime,
      Seconds (0));
  }

  void InstallTrafficModel (TrafficModel_t trafficModel,
                            TransportLayerProtocol_t protocol,
                            TrafficDirection_t direction,
                            Time startTime)
  {
    InstallTrafficModel (
      trafficModel, protocol, direction,
      startTime,
      m_simTime + Seconds (1),
      Seconds (0));
  }

  void InstallTrafficModel (TrafficModel_t trafficModel,
                            TransportLayerProtocol_t protocol,
                            TrafficDirection_t direction)
  {
    InstallTrafficModel (
      trafficModel, protocol, direction,
      Seconds (0.001),
      m_simTime + Seconds (1),
      Seconds (0));
  }

  typedef enum
  {
    CR_NOT_CONFIGURED,
    CR_PERIODIC_CONTROL,
    CR_SLOTTED_ALOHA,
    CR_CRDSA_LOOSE_RC_0,
  } CrTxConf_t;

  void SetCrTxConf (CrTxConf_t crTxConf);

  /**
   * \brief Set the ID of the GW user for traffic models.
   * \param gwUserId GW user's ID
   */
  inline void SetGwUserId (uint32_t gwUserId)
  {
    m_gwUserId = gwUserId;
  }

protected:
  /**
   * \brief Enable random access
   */
  void EnableRandomAccess ();

  /**
   * \brief Callback that prints simulation progress to stdout.
   */
  void ProgressCb ();

  /**
   * \brief Check if a beam is enabled.
   */
  bool IsBeamEnabled (uint32_t beamId) const;

  /**
   * \brief Get next UT count from internal random variable stream.
   */
  inline uint32_t GetNextUtCount () const
  {
    NS_ASSERT_MSG (m_utCount != NULL, "UT count per beam not set");
    return m_utCount->GetInteger ();
  }

  /**
   * \brief Get next UT user count from internal random variable stream.
   */
  inline uint32_t GetNextUtUserCount () const
  {
    NS_ASSERT_MSG (m_utCount != NULL, "User count per UT not set");
    return m_utUserCount->GetInteger ();
  }

  /**
   * \brief Check if node has a PacketSink installed at certain port.
   */
  bool HasSinkInstalled (Ptr<Node> node, uint16_t port);

  /**
   * \brief Check if output path has been set. If not, then create a default
   * output directory inside satellite/data/sims/campaign-name/tag-name.
   */
  void SetupOutputPath ();

private:
  Ptr<SatHelper> m_satHelper;
  Ptr<SatStatsHelperContainer> m_statContainer;
  Ptr<SatListPositionAllocator> m_commonUtPositions;
  std::map<uint32_t, Ptr<SatListPositionAllocator> > m_utPositionsByBeam;

  std::string                  m_simulationName;
  std::string                  m_simulationTag;
  std::string                  m_enabledBeamsStr;
  std::set<uint32_t>           m_enabledBeams;
  std::string                  m_outputPath;
  Ptr<RandomVariableStream>    m_utCount;
  Ptr<RandomVariableStream>    m_utUserCount;
  Time                         m_simTime;
  uint32_t                     m_numberOfConfiguredFrames;
  bool                         m_randomAccessConfigured;
  bool                         m_enableInputFileUtListPositions;
  bool                         m_inputFileUtPositionsCheckBeams;
  uint32_t                     m_gwUserId;

  bool                         m_progressLoggingEnabled;
  Time                         m_progressUpdateInterval;
  EventId                      m_progressReportEvent;
};


class SimulationHelperConf : public Object
{
public:
  /**
   * Default constructor.
   */
  SimulationHelperConf ();

  /**
   * \brief Destructor.
   */
  virtual ~SimulationHelperConf ();

  /**
   * \brief Derived from Object.
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Derived from Object.
   */
  TypeId GetInstanceTypeId (void) const;

  typedef enum
  {
    PROTOCOL_UDP, PROTOCOL_TCP, PROTOCOL_BOTH
  } TransportLayerProtocol_t;

  typedef enum
  {
    RTN_LINK, FWD_LINK, BOTH_LINK
  } TrafficDirection_t;

  typedef struct TrafficConfiguration_t
  {
    double m_percentage;
    TransportLayerProtocol_t m_protocol;
    TrafficDirection_t m_direction;
    Time m_startTime;
    Time m_stopTime;
    Time m_startDelay;

    TrafficConfiguration_t ()
      : m_percentage (0.0),
      m_protocol (SimulationHelperConf::PROTOCOL_UDP),
      m_direction (SimulationHelperConf::RTN_LINK),
      m_startTime (0),
      m_stopTime (0),
      m_startDelay (0)
    {
      // do nothing
    }
  } TrafficConfiguration_t;

  Time                                           m_simTime;
  std::string                                    m_enabledBeams;
  Ptr<RandomVariableStream>                      m_utCount;
  Ptr<RandomVariableStream>                      m_utUserCount;
  bool                                           m_activateStatistics;
  bool                                           m_activateProgressLogging;
  SimulationHelper::CrTxConf_t                   m_crTxConf;
  std::map<std::string, TrafficConfiguration_t>  m_trafficModel;

private:
  void SetTrafficPercentage (std::string trafficModel, double percentage)
  {
    std::map<std::string, TrafficConfiguration_t>::iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        TrafficConfiguration_t traffic;
        traffic.m_percentage = percentage;
        m_trafficModel.emplace (trafficModel, traffic);
      }
    else
      {
        it->second.m_percentage = percentage;
      }
  }

  double GetTrafficPercentage (std::string trafficModel) const
  {
    std::map<std::string, TrafficConfiguration_t>::const_iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        NS_FATAL_ERROR ("Traffic model " << trafficModel << " has not been configured");
      }

    return it->second.m_percentage;
  }

  void SetTrafficProtocol (std::string trafficModel, TransportLayerProtocol_t protocol)
  {
    std::map<std::string, TrafficConfiguration_t>::iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        TrafficConfiguration_t traffic;
        traffic.m_protocol = protocol;
        m_trafficModel.emplace (trafficModel, traffic);
      }
    else
      {
        it->second.m_protocol = protocol;
      }
  }

  TransportLayerProtocol_t GetTrafficProtocol (std::string trafficModel) const
  {
    std::map<std::string, TrafficConfiguration_t>::const_iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        NS_FATAL_ERROR ("Traffic model " << trafficModel << " has not been configured");
      }

    return it->second.m_protocol;
  }

  void SetTrafficDirection (std::string trafficModel, TrafficDirection_t direction)
  {
    std::map<std::string, TrafficConfiguration_t>::iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        TrafficConfiguration_t traffic;
        traffic.m_direction = direction;
        m_trafficModel.emplace (trafficModel, traffic);
      }
    else
      {
        it->second.m_direction = direction;
      }
  }

  TrafficDirection_t GetTrafficDirection (std::string trafficModel) const
  {
    std::map<std::string, TrafficConfiguration_t>::const_iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        NS_FATAL_ERROR ("Traffic model " << trafficModel << " has not been configured");
      }

    return it->second.m_direction;
  }

  void SetTrafficStartTime (std::string trafficModel, Time startTime)
  {
    std::map<std::string, TrafficConfiguration_t>::iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        TrafficConfiguration_t traffic;
        traffic.m_startTime = startTime;
        m_trafficModel.emplace (trafficModel, traffic);
      }
    else
      {
        it->second.m_startTime = startTime;
      }
  }

  Time GetTrafficStartTime (std::string trafficModel) const
  {
    std::map<std::string, TrafficConfiguration_t>::const_iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        NS_FATAL_ERROR ("Traffic model " << trafficModel << " has not been configured");
      }

    return it->second.m_startTime;
  }

  void SetTrafficStopTime (std::string trafficModel, Time stopTime)
  {
    std::map<std::string, TrafficConfiguration_t>::iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        TrafficConfiguration_t traffic;
        traffic.m_startTime = stopTime;
        m_trafficModel.emplace (trafficModel, traffic);
      }
    else
      {
        it->second.m_stopTime = stopTime;
      }
  }

  Time GetTrafficStopTime (std::string trafficModel) const
  {
    std::map<std::string, TrafficConfiguration_t>::const_iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        NS_FATAL_ERROR ("Traffic model " << trafficModel << " has not been configured");
      }

    return it->second.m_stopTime;
  }

  void SetTrafficStartDelay (std::string trafficModel, Time startDelay)
  {
    std::map<std::string, TrafficConfiguration_t>::iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        TrafficConfiguration_t traffic;
        traffic.m_startDelay = startDelay;
        m_trafficModel.emplace (trafficModel, traffic);
      }
    else
      {
        it->second.m_startDelay = startDelay;
      }
  }

  Time GetTrafficStartDelay (std::string trafficModel) const
  {
    std::map<std::string, TrafficConfiguration_t>::const_iterator it = m_trafficModel.find (trafficModel);
    if (it == m_trafficModel.end ())
      {
        NS_FATAL_ERROR ("Traffic model " << trafficModel << " has not been configured");
      }

    return it->second.m_startDelay;
  }

#define TRAFFIC_MODEL_ATTRIBUTE_ACCESSOR_DEFINE(index) \
  inline void SetTraffic ## index ## Percentage (double value) \
  { return SetTrafficPercentage (TOSTRING (index), value); \
  } \
  inline double GetTraffic ## index ## Percentage () const \
  { return GetTrafficPercentage (TOSTRING (index)); } \
  inline void SetTraffic ## index ## Protocol (TransportLayerProtocol_t value) \
  { return SetTrafficProtocol (TOSTRING (index), value); } \
  inline TransportLayerProtocol_t GetTraffic ## index ## Protocol () const \
  { return GetTrafficProtocol (TOSTRING (index)); } \
  inline void SetTraffic ## index ## Direction (TrafficDirection_t value) \
  { return SetTrafficDirection (TOSTRING (index), value); } \
  inline TrafficDirection_t GetTraffic ## index ## Direction () const \
  { return GetTrafficDirection (TOSTRING (index)); } \
  inline void SetTraffic ## index ## StartTime (Time value) \
  { return SetTrafficStartTime (TOSTRING (index), value); } \
  inline Time GetTraffic ## index ## StartTime () const \
  { return GetTrafficStartTime (TOSTRING (index)); } \
  inline void SetTraffic ## index ## StopTime (Time value) \
  { return SetTrafficStopTime (TOSTRING (index), value); } \
  inline Time GetTraffic ## index ## StopTime () const \
  { return GetTrafficStopTime (TOSTRING (index)); } \
  inline void SetTraffic ## index ## StartDelay (Time value) \
  { return SetTrafficStartDelay (TOSTRING (index), value); } \
  inline Time GetTraffic ## index ## StartDelay () const \
  { return GetTrafficStartDelay (TOSTRING (index)); }

  TRAFFIC_MODEL_ATTRIBUTE_ACCESSOR_DEFINE (Cbr);
  TRAFFIC_MODEL_ATTRIBUTE_ACCESSOR_DEFINE (Http);
  TRAFFIC_MODEL_ATTRIBUTE_ACCESSOR_DEFINE (OnOff);
  TRAFFIC_MODEL_ATTRIBUTE_ACCESSOR_DEFINE (Nrtv);
};

} // namespace ns3

#endif /* TEST_SCRIPT_INPUT_HELPER_H */
