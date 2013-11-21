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

NS_LOG_COMPONENT_DEFINE ("environmental-variables-example");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("environmental-variables-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatEnvVariables", LOG_LEVEL_INFO);

  /// create default environmental variables
  Ptr<SatEnvVariables> envVariables = CreateObject<SatEnvVariables> ();

  /// run simulation
  Simulator::Schedule (MilliSeconds(0), &SatEnvVariables::GetCurrentWorkingDirectory, envVariables);
  Simulator::Schedule (MilliSeconds(1), &SatEnvVariables::GetPathToExecutable, envVariables);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
