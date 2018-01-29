#include "ns3/core-module.h"
#include "ns3/traffic-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/config-store-module.h"


using namespace ns3;


/**
 * \file sat-generic-launcher.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results with
 * a high degree of customization through XML file.
 *
 * execute command -> ./waf --run "sat-generic-launcher --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-generic-launcher");


int
main (int argc, char *argv[])
{
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("generic-launcher");
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/generic-input-attributes.xml";

  CommandLine cmd;
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  simulationHelper->CreateSatScenario ();
  simulationHelper->InstallTrafficModel (
    SimulationHelper::HTTP,
    SimulationHelper::TCP,
    SimulationHelper::FWD_LINK,
    MilliSeconds (3));

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamRtnDevDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnDevDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamRtnPhyDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnPhyDelay (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamFwdAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamFwdAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFwdDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFwdMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFwdPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddPerBeamFwdAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamFwdAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamFwdDevDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamFwdDevDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamFwdPhyDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamFwdPhyDelay (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_PLOT);

  s->AddPerBeamCrdsaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamCrdsaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketCollision (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamSlottedAlohaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);

  std::string outputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/generic-output-attributes.xml";
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (outputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  simulationHelper->RunSimulation ();
}
