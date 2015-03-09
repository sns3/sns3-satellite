/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 *
 */

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

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::SimulationCampaignName", StringValue ("example-rayleigh"));
  Config::SetDefault ("ns3::SatEnvVariables::SimulationTag", StringValue (""));
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Create fader
  Ptr<SatRayleighConf> rayleighConf = CreateObject<SatRayleighConf> ();
  Ptr<SatRayleighModel> rayleighFader = CreateObject<SatRayleighModel> (rayleighConf,0,0);

  /// Run simulation
  for (uint32_t i = 0; i < 1000; i++)
    {
      Simulator::Schedule (Time (500000 + i * 500000), &SatRayleighModel::GetChannelGainDb, rayleighFader);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
