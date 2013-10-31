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
* \brief
*
*/

NS_LOG_COMPONENT_DEFINE ("rayleigh-example");

int
main (int argc, char *argv[])
{
  /// enable info logs
  LogComponentEnable ("rayleigh-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatRayleighModel", LOG_LEVEL_INFO);

  /// create fader
  Ptr<SatRayleighModel> rayleighFader = CreateObject<SatRayleighModel> (10,1);

  /// run simulation
  for (uint32_t i = 0; i < 1000; i++)
    {
      Simulator::Schedule(Time(500000 + i*500000), &SatRayleighModel::GetChannelGainDb, rayleighFader);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
