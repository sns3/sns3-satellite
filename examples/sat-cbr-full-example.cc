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
#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-cbr-full-example.cc
 * \ingroup satellite
 *
 * \brief  An example to run CBR application in full satellite scenario and FWD link.
 *
 *         execute command -> ./waf --run "sat-cbr-full-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-cbr-full-example");

int
main (int argc, char *argv[])
{
  // LogComponentEnable ("CbrKpiHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("CbrApplication", LOG_LEVEL_ALL);
  // LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);
  // LogComponentEnable ("sat-arq-rtn-example", LOG_LEVEL_INFO);

  uint32_t endUsersPerUt (3);
  uint32_t utsPerBeam (3);
  uint32_t packetSize (128);
  std::string interval ("1s");
  double simLength (10.0);

  Time appStartTime = Seconds(0.001);

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("endUsersPerUt", "Number of end users per UT", endUsersPerUt);
  cmd.AddValue("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue("packetSize", "Size of constant packet (bytes)", packetSize);
  cmd.AddValue("interval", "Interval to sent packets in seconds, (e.g. (1s)", interval);
  cmd.AddValue("simLength", "Simulation length in seconds", simLength);
  cmd.Parse (argc, argv);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::SimulationCampaignName", StringValue ("example-cbr-full"));
  Config::SetDefault ("ns3::SatEnvVariables::SimulationTag", StringValue (""));
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  Config::SetDefault ("ns3::SatHelper::UtCount", UintegerValue(utsPerBeam));
  Config::SetDefault ("ns3::SatHelper::UtUsers", UintegerValue(endUsersPerUt));

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_CONSTANT);
  double errorRate (0.2);
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::ConstantErrorRatio", DoubleValue(errorRate));
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Create full scenario

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);
  helper->CreatePredefinedScenario (SatHelper::FULL);

  helper->EnablePacketTrace ();

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
  cbrHelper.SetAttribute("Interval", StringValue (interval));
  cbrHelper.SetAttribute("PacketSize", UintegerValue (packetSize) );

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  // initialized time values for simulation
  uint32_t maxReceivers = utUsers.GetN ();

  ApplicationContainer gwApps;
  ApplicationContainer utApps;

  Time cbrStartDelay = appStartTime;

  // Cbr and Sink applications creation
  for ( uint32_t i = 0; i < maxReceivers; i++)
    {
      cbrHelper.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (i)), port))));
      sinkHelper.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (i)), port))));

      gwApps.Add(cbrHelper.Install (gwUsers.Get (4)));
      utApps.Add(sinkHelper.Install (utUsers.Get (i)));

      cbrStartDelay += Seconds (0.001);

      gwApps.Get(i)->SetStartTime (cbrStartDelay);
      gwApps.Get(i)->SetStopTime (Seconds (simLength));
    }

  // Add the created applications to CbrKpiHelper
  CbrKpiHelper kpiHelper (KpiHelper::KPI_FWD);
  kpiHelper.AddSink (utApps);
  kpiHelper.AddSender (gwApps);

  utApps.Start (appStartTime);
  utApps.Stop (Seconds (simLength));

  NS_LOG_INFO("--- sat-cbr-full-example ---");
  NS_LOG_INFO("  Packet size in bytes: " << packetSize);
  NS_LOG_INFO("  Packet sending interval: " << interval);
  NS_LOG_INFO("  Simulation length: " << simLength);
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  Simulator::Stop (Seconds (simLength));
  Simulator::Run ();

  kpiHelper.Print ();

  Simulator::Destroy ();

  return 0;

} // end of `int main (int argc, char *argv[])`
