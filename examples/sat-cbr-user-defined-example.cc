/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"


using namespace ns3;

/**
 * \file sat-cbr-user-defined-example.cc
 * \ingroup satellite
 *
 * \brief  Another example of CBR application usage in satellite network.
 *         The scripts is using user defined scenario, which means that user
 *         can change the scenario size quite to be whatever between 1 and
 *         full scenario (72 beams). Currently it is configured to using only
 *         one beam. CBR application is sending packets in RTN link, i.e. from UT
 *         side to GW side. Packet trace is enabled by default.
 *         End user may change the number of UTs and end users from
 *         the command line.
 *
 *         execute command -> ./waf --run "sat-cbr-user-defined-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-cbr-user-defined-example");

// callback called when packet is received by phy RX carrier
static void CrTraceCb (Time now, Mac48Address addr, Ptr<SatCrMessage> crMsg)
{

  NS_LOG_INFO ( "General info: " << Simulator::Now ().GetSeconds () << " "
                                 << addr << " "
                                 << crMsg->GetNumCapacityRequestElements () << " "
                                 << crMsg->GetSizeInBytes () << " "
                                 << crMsg->GetCnoEstimate () );

  SatCrMessage::RequestContainer_t c = crMsg->GetCapacityRequestContent ();
  for (SatCrMessage::RequestContainer_t::const_iterator it = c.begin ();
       it != c.end ();
       ++it)
    {
      NS_LOG_INFO ( "CR component: " << (uint32_t)(it->first.first) << " " << it->first.second << " " << it->second);
    }
}

int
main (int argc, char *argv[])
{
  // enable info logs
  // LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  // LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  // LogComponentEnable ("sat-cbr-user-defined-example", LOG_LEVEL_INFO);

  uint32_t beamId = 1;
  uint32_t endUsersPerUt (3);
  uint32_t utsPerBeam (3);
  uint32_t packetSize (128);
  Time interval (Seconds (1.0));
  Time simLength (Seconds (20.0));
  Time appStartTime = Seconds (0.1);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Enable packet trace
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));
  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("example-cbr-user-defined");

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("endUsersPerUt", "Number of end users per UT", endUsersPerUt);
  cmd.AddValue ("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  // dama-input-attributes.xml
  // tn8-profiling-input-attributes.xml
  // tn9-dama-input-attributes.xml
  // tn9-ra-input-attributes.xml
  // training-input-attributes.xml

  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetUtCountPerBeam (utsPerBeam);

  // Set beam ID
  std::stringstream beamsEnabled; beamsEnabled  << beamId;
  simulationHelper->SetBeams (beamsEnabled.str ());

  // Configure error model
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));
  //Config::SetDefault ("ns3::SatUtMac::CrUpdatePeriod", TimeValue(Seconds(10.0)));

  // Create reference system
  simulationHelper->CreateSatScenario ();

  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatLlc/SatRequestManager/CrTrace",
                                 MakeCallback (&CrTraceCb));

  // >>> Start of actual test using Full scenario >>>

  // setup CBR traffic
  Config::SetDefault ("ns3::CbrApplication::Interval", TimeValue (interval));
  Config::SetDefault ("ns3::CbrApplication::PacketSize", UintegerValue (packetSize));

  simulationHelper->InstallTrafficModel (
  					SimulationHelper::CBR, SimulationHelper::UDP, SimulationHelper::RTN_LINK,
  					appStartTime, Seconds (simLength), Seconds (0.05));

  NS_LOG_INFO ("--- sat-cbr-user-defined-example ---");
  NS_LOG_INFO ("  Packet size in bytes: " << packetSize);
  NS_LOG_INFO ("  Packet sending interval: " << interval.GetSeconds ());
  NS_LOG_INFO ("  Simulation length: " << simLength.GetSeconds ());
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  simulationHelper->RunSimulation ();

  return 0;
}
