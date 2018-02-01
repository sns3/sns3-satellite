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

  simulationHelper->RunSimulation ();
}
