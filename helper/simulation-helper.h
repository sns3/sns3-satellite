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
#include <ns3/satellite-helper.h>
#include <ns3/satellite-stats-helper-container.h>
#include <ns3/satellite-enums.h>

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
  void SetBeams (std::string beamList);

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
   * \brief Set simulation time in seconds.
   * \param seconds
   */
  void SetSimulationTime (double seconds);

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
   * \param constantIf Static interference if constant interference model used
   */
  void SetInterferenceModel(SatPhyRxCarrierConf::InterferenceModel ifModel,
                            double constantIf = 0.0);

  /**
   * \brief Enables simulation progress logging.
   * Progress is logged to stdout in form of 'Progress: (current simulation time)/(simulation length)'.
   */
  void EnableProgressLogging ();

  /**
   * \brief Run the simulation
   */
  void RunSimulation ();

  /**
   * \brief Create the satellite scenario.
   * \return satHelper Satellite helper, which provides e.g. nodes for application installation.
   */
  Ptr<SatHelper> CreateSatScenario ();

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
   * \brief Read input attributes from XML file
   * \param fileName Input XML file name
   */
  void ReadInputAttributesFromFile (std::string fileName);

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

  inline Time& GetSimTime () { return m_simTime; }

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
  inline uint32_t GetNextUtCount () const { return m_utCount->GetInteger (); }

  /**
   * \brief Get next UT user count from internal random variable stream.
   */
  inline uint32_t GetNextUtUserCount () const { return m_utUserCount->GetInteger (); }

private:

  Ptr<SatHelper> m_satHelper;
  Ptr<SatStatsHelperContainer> m_statContainer;

  std::string                  m_simulationName;
  std::string                  m_enabledBeamsStr;
  std::set<uint32_t>           m_enabledBeams;
  Ptr<RandomVariableStream>    m_utCount;
  Ptr<RandomVariableStream>    m_utUserCount;
  Time                         m_simTime;
  std::string                  m_uiOutputPath;
  std::string                  m_uiOutputFileName;
  uint32_t                     m_numberOfConfiguredFrames;
  bool                         m_randomAccessConfigured;
};

} // namespace ns3

#endif /* TEST_SCRIPT_INPUT_HELPER_H */
