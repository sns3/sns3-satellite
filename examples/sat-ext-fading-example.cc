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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-ext-fading-example.cc
 * \ingroup satellite
 *
 * \brief  External fading example to demonstrate how to set user defined positions
 *         and to set external fading on/off.
 *
 *         Some parameter can be set as command line arguments.
 *         To see help for user arguments:
 *         execute command -> ./waf --run "sat-ext-fading-example --PrintHelp"
 *
 *         Example sends packets from GW connected user to every UT users and
 *         from every UT users to GW connected user.
 *
 *         Information of the created UTs and link budget info for every received packet
 *         are printed as log info.
 */

NS_LOG_COMPONENT_DEFINE ("sat-ext-fading-example");

// callback called when packet is received by phy RX carrier
static void PacketTraceCb ( std::string context, Ptr<SatSignalParameters> params, Mac48Address ownAdd , Mac48Address destAdd,
                            double ifPower, double cSinr)
{
  // print only unicast message to prevent printing control messages like TBTP messages
  if ( !destAdd.IsBroadcast() )
    {
      NS_LOG_INFO ( Simulator::Now() << " "
                    << params->m_channelType << " "
                    << ownAdd << " "
                    << destAdd << " "
                    << params->m_beamId << " "
                    << SatUtils::LinearToDb (params->m_sinr) << " "
                    << SatUtils::LinearToDb (cSinr) );
    }
}

int
main (int argc, char *argv[])
{
  uint32_t utCount = 1;
  uint32_t usersPerUt = 1;
  bool extFadingOn = true;

  // read command line parameters can be given by user
  CommandLine cmd;
  cmd.AddValue ("utCount", "Number of the UTs.", utCount);
  cmd.AddValue ("usersPerUt", "Users per UT.", usersPerUt);
  cmd.AddValue ("extFadingOn", "External fading on/off (true/false).", extFadingOn);
  cmd.Parse (argc, argv);

  // enable info logs
  LogComponentEnable ("sat-ext-fading-example", LOG_LEVEL_INFO);

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  // Set user specific UT position file (UserDefinedUtPos.txt) to be utilized by SatConf.
  // Given file must locate in /satellite/data folder
  //
  // This enables user defined positions used instead of default positions defined file UtPos.txt,
  Config::SetDefault ("ns3::SatConf::UtPositionInputFileName", StringValue ("utpositions/BeamId-1_256_UT_Positions.txt"));

  // Set external fading input trace container mode as list mode
  // Now external fading input file used for UT1 is input file defined in row one in set index file,
  // for UT2 second input file defined in row two in set index file etc.
  // Position info in index file is ignored by list mode
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::InputMode", StringValue ("ListMode"));

  // Set index files defining external tracing input files for UTs
  // Given index files must locate in /satellite/data/ext-fadingtraces/input folder
  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName",
                       StringValue ("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"));

  Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName",
                       StringValue ("BeamId-1_256_UT_fading_rtnup_trace_index.txt"));

  // for GWs we don't set up index files, so default ones are used (GW_fading_fwdup_traces.txt and GW_fading_rtndwn_traces.txt)

  // enable/disable external fading input on SatChannel as user requests
  Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (extFadingOn));

  // create helper
  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  helper->EnableCreationTraces ("ext-fading", false);

  /** create user defined scenario */

  // define how many UTs and users per UT is created
  SatBeamUserInfo beamInfo = SatBeamUserInfo (utCount, usersPerUt);

  // pass empty beam ID info in creation as first parameter to allow beam selected freely
  // if IDs are given, then helpers verifies that the best beam based on position is among of the given beams.
  // In case that beam is not in given list simulation crashes.
  SatHelper::BeamIdInfo_t beamIdInfo;

  // Now earlier defined/set positions for SatConf are set for UTs from input file
  // (defined by attribute 'ns3::SatConf::UtPositionInputFileName)
  // When creating user defined scenario with method version
  // SatHelper::CreateUserDefinedScenario (BeamIdInfo_t& beamInfo, SatBeamUserInfo& utInfo),
  // position are set from input file in creation order.
  // So first created UT get position from row one, second from row two etc.

  helper->CreateUserDefinedScenario (beamIdInfo, beamInfo);
  // Note! Positions can be set also freely (by passing SatConf), when using method version
  // SatHelper::CreateUserDefinedScenario (BeamIdInfo_t& beamInfo, SatBeamUserInfo& utInfo, Ptr<SatListPositionAllocator> utPositions)
  // instead of version we are using here

  // set callback traces where we want results out
  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                               MakeCallback (&PacketTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                               MakeCallback (&PacketTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                                 MakeCallback (&PacketTraceCb));

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  uint16_t port = 9;

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));

  // create sink application on GW user
  ApplicationContainer sinkContainer = sinkHelper.Install (gwUsers.Get (0));
  sinkContainer.Start (Seconds (0.1));
  sinkContainer.Stop (Seconds (1.0));

  ApplicationContainer cbrContainer;
  cbrContainer.Start (Seconds (0.1));
  cbrContainer.Stop (Seconds (0.25));

  CbrHelper gwCbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port));
  gwCbrHelper.SetAttribute ("Interval", StringValue ("0.1s"));
  gwCbrHelper.SetAttribute ("PacketSize", UintegerValue (512) );

  CbrHelper utCbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));
  utCbrHelper.SetAttribute ("Interval", StringValue ("0.1s"));
  utCbrHelper.SetAttribute ("PacketSize", UintegerValue (512) );

  // create sink applications on UT users, CBR applications on GW user and UT users
  for (uint32_t i = 0; i < utUsers.GetN (); i++)
    {
      sinkHelper.SetAttribute ("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get(i)), port))));
      sinkContainer.Add (sinkHelper.Install (utUsers.Get (i)));

      gwCbrHelper.SetAttribute ("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get(i)), port))));
      cbrContainer.Add (gwCbrHelper.Install (gwUsers.Get (0)));

      cbrContainer.Add (utCbrHelper.Install (utUsers.Get (i)));
    }

  NS_LOG_INFO ("--- External Fading Example ---");
  NS_LOG_INFO ("UT info (Beam ID, UT ID, Latitude, Longitude, Altitude + addresses");

  // print UT info
  NS_LOG_INFO (helper->GetBeamHelper ()->GetUtInfo ());
  NS_LOG_INFO ("Link results (Time, Channel type, Own address, Dest. address, Beam ID, SINR, Composite SINR) :");
  // results are printed out in callback (PacketTraceCb)

  Simulator::Stop (Seconds (1.1));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
