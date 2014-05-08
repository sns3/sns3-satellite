/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

/**
* \ingroup satellite
*
* \brief Example for environmental variables class.
*
*/

NS_LOG_COMPONENT_DEFINE ("sat-environmental-variables-example");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("sat-environmental-variables-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatEnvVariables", LOG_LEVEL_INFO);

  /// Create default environmental variables
  Ptr<SatEnvVariables> envVariables = CreateObject<SatEnvVariables> ();

  /// Run simulation
  Simulator::Schedule (MilliSeconds(0), &SatEnvVariables::GetCurrentWorkingDirectory, envVariables);
  Simulator::Schedule (MilliSeconds(1), &SatEnvVariables::GetPathToExecutable, envVariables);
  Simulator::Schedule (MilliSeconds(2), &SatEnvVariables::GetDataPath, envVariables);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
