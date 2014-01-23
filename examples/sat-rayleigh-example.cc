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
* \brief Example for Rayleigh fader class.
*
*/

NS_LOG_COMPONENT_DEFINE ("sat-rayleigh-example");

int
main (int argc, char *argv[])
{
  /// enable info logs
  LogComponentEnable ("sat-rayleigh-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatRayleighModel", LOG_LEVEL_INFO);

  /// create fader
  Ptr<SatRayleighConf> rayleighConf = CreateObject<SatRayleighConf> ();
  Ptr<SatRayleighModel> rayleighFader = CreateObject<SatRayleighModel> (rayleighConf,0,0);

  /// run simulation
  for (uint32_t i = 0; i < 1000; i++)
    {
      Simulator::Schedule (Time (500000 + i*500000), &SatRayleighModel::GetChannelGainDb, rayleighFader);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
