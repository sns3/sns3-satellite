/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "../model/cbr-application.h"
#include "../helper/cbr-helper.h"
#include "../helper/cbr-kpi-helper.h"
#include "../helper/satellite-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("sat-cbr-larger-example");

int
main (int argc, char *argv[])
{
//  LogComponentEnable ("CbrKpiHelper", LOG_LEVEL_ALL);
//  LogComponentEnable ("CbrApplication", LOG_LEVEL_ALL);
//  LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);

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

  Config::SetDefault ("ns3::SatHelper::UtCount", UintegerValue(utsPerBeam));
  Config::SetDefault ("ns3::SatHelper::UtUsers", UintegerValue(endUsersPerUt));

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_CONSTANT);
  double errorRate (0.2);
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::ConstantErrorRatio", DoubleValue(errorRate));
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  // Create full scenario

   // Create reference system, two options:
   // - "Scenario72"
   // - "Scenario98"
   std::string scenarioName = "Scenario72";
   Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);
   helper->CreateScenario (SatHelper::FULL);

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
   CbrKpiHelper kpiHelper;
   kpiHelper.AddClient (utApps);
   kpiHelper.AddServer (gwApps);

   utApps.Start (appStartTime);
   utApps.Stop (Seconds (simLength));

   Simulator::Stop (Seconds (simLength));
   Simulator::Run ();

   kpiHelper.Print ();

   Simulator::Destroy ();

   return 0;

} // end of `int main (int argc, char *argv[])`
