/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/satellite-module.h"

using namespace ns3;

/**
 * \file sat-rayleigh-example.cc
 * \ingroup satellite
 *
 * \brief Example for Rayleigh fader class. Demonstrates the functionality
 * of the Rayleigh fader used by the Markov-fading. The example creates
 * a single Rayleigh fader and outputs the fading values in time.
 */

NS_LOG_COMPONENT_DEFINE ("sat-rayleigh-example");

int
main (int argc, char *argv[])
{
  /// Enable info logs
  LogComponentEnable ("sat-rayleigh-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatRayleighModel", LOG_LEVEL_INFO);

  /// Create fader
  Ptr<SatRayleighConf> rayleighConf = CreateObject<SatRayleighConf> ();
  Ptr<SatRayleighModel> rayleighFader = CreateObject<SatRayleighModel> (rayleighConf,0,0);

  /// Run simulation
  for (uint32_t i = 0; i < 1000; i++)
    {
      Simulator::Schedule (Time (500000 + i*500000), &SatRayleighModel::GetChannelGainDb, rayleighFader);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
