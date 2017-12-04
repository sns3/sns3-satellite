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
#include "ns3/traffic-module.h"
#include "ns3/config-store-module.h"


using namespace ns3;

/**
 * \file sat-fwd-system-test-example.cc
 * \ingroup satellite
 *
 * \brief Simulation script to execute system tests for the forward link.
 *
 * To get help of the command line arguments for the example,
 * execute command -> ./waf --run "sat-fwd-sys-test --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-fwd-sys-test");

static void PrintBbFrameInfo (Ptr<SatBbFrame> bbFrame)
{
  std::cout
  << "[BBFrameTx] "
  << "Time: " << Now ().GetSeconds ()
  << ", Frame Type: " << SatEnums::GetFrameTypeName (bbFrame->GetFrameType ())
  << ", ModCod: " << SatEnums::GetModcodTypeName (bbFrame->GetModcod ())
  << ", Occupancy: " << bbFrame->GetOccupancy ()
  << ", Duration: " << bbFrame->GetDuration ()
  << ", Space used: " << bbFrame->GetSpaceUsedInBytes ()
  << ", Space Left: " << bbFrame->GetSpaceLeftInBytes ();

  std::cout << " [Receivers: ";

  for ( SatBbFrame::SatBbFramePayload_t::const_iterator it = bbFrame->GetPayload ().begin (); it != bbFrame->GetPayload ().end (); it++ )
    {
      SatMacTag tag;

      if ( (*it)->PeekPacketTag (tag) )
        {
          if (it != bbFrame->GetPayload ().begin ())
            {
              std::cout << ", ";
            }

          std::cout << tag.GetDestAddress ();
        }
      else
        {
          NS_FATAL_ERROR ("No tag");
        }
    }

  std::cout << "]" << std::endl;
}

static void PrintBbFrameMergeInfo (Ptr<SatBbFrame> mergeTo, Ptr<SatBbFrame> mergeFrom)
{
  std::cout << "[Merge Info Begins]" << std::endl;
  std::cout << "Merge To   -> ";
  PrintBbFrameInfo (mergeTo);
  std::cout << "Merge From <- ";
  PrintBbFrameInfo (mergeFrom);
  std::cout << "[Merge Info Ends]" << std::endl;
}

int
main (int argc, char *argv[])
{
  // Enable some logs.
  LogComponentEnable ("sat-fwd-sys-test", LOG_INFO);

  // Spot-beam served by GW1
  uint32_t beamId = 26;
  uint32_t gwEndUsers = 10;

  uint32_t testCase = 0;
  std::string trafficModel = "cbr";
  double simLength (400.0); // in seconds
  Time senderAppStartTime = Seconds (0.1);
  bool traceFrameInfo = true;
  bool traceMergeInfo = true;

  UintegerValue packetSize (128); // in bytes
  TimeValue interval (MicroSeconds (10));
  DataRateValue dataRate (DataRate (16000));

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-fwd-system-test");
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));


  // set default values for traffic model apps here
  // attributes can be overridden by command line arguments when needed
  Config::SetDefault ("ns3::CbrApplication::PacketSize", packetSize);
  Config::SetDefault ("ns3::CbrApplication::Interval", interval);
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", packetSize);
  Config::SetDefault ("ns3::OnOffApplication::DataRate", dataRate);
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));

  Config::SetDefault ("ns3::SatBbFrameConf::BbFrameHighOccupancyThreshold", DoubleValue (0.9));
  Config::SetDefault ("ns3::SatBbFrameConf::BbFrameLowOccupancyThreshold", DoubleValue (0.8));
  Config::SetDefault ("ns3::SatBbFrameConf::BBFrameUsageMode", StringValue ("ShortAndNormalFrames"));
  Config::SetDefault ("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue (1.25e+07));

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("testCase", "Test case to execute. 0 = scheduler, ACM off, 1 = scheduler, ACM on, 2 = ACM one UT", testCase);
  cmd.AddValue ("gwEndUsers", "Number of the GW end users", gwEndUsers);
  cmd.AddValue ("simLength", "Length of simulation", simLength);
  cmd.AddValue ("traceFrameInfo", "Trace (print) BB frame info", traceFrameInfo);
  cmd.AddValue ("traceMergeInfo", "Trace (print) BB frame merge info", traceMergeInfo);
  cmd.AddValue ("beamId", "Beam Id", beamId);
  cmd.AddValue ("trafficModel", "Traffic model: either 'cbr' or 'onoff'", trafficModel);
  cmd.AddValue ("senderAppStartTime", "Sender application (first) start time", senderAppStartTime);
  cmd.Parse (argc, argv);

  if (trafficModel != "cbr" && trafficModel != "onoff")
		{
			NS_FATAL_ERROR ("Invalid traffic model, use either 'cbr' or 'onoff'");
		}
  SimulationHelper::TrafficModel_t model = trafficModel == "cbr" ? SimulationHelper::CBR : SimulationHelper::ONOFF;

  simulationHelper->SetUtCountPerBeam (gwEndUsers);
  simulationHelper->SetUserCountPerUt (1);
  simulationHelper->SetSimulationTime (simLength);
  simulationHelper->SetGwUserCount (gwEndUsers);
  simulationHelper->SetBeamSet ({beamId});

  /**
   * Select test case to execute
   */

  switch (testCase)
    {
    case 0:   // scheduler, ACM disabled
      Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (false));
      break;

    case 1:   // scheduler, ACM enabled
      Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (true));
      break;

    case 2:   // ACM enabled, one UT with one user, Markov + external fading
      Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (true));
      Config::SetDefault ("ns3::SatBeamHelper::FadingModel", StringValue ("FadingMarkov"));

      // Note, that the positions of the fading files do not necessarily match with the
      // beam location, since this example is not using list position allocator!
      Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
      Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName", StringValue ("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"));
      Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName", StringValue ("BeamId-1_256_UT_fading_rtnup_trace_index.txt"));

      gwEndUsers = 1;
      break;

    default:
      break;
    }

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  simulationHelper->CreateSatScenario ();

  // connect BB frame TX traces on, if enabled
  if (traceFrameInfo)
    {
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatMac/BBFrameTxTrace", MakeCallback (&PrintBbFrameInfo));
    }

  // connect BB frame merge traces on, if enabled
  if (traceMergeInfo)
    {
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatMac/Scheduler/BBFrameContainer/BBFrameMergeTrace", MakeCallback (&PrintBbFrameMergeInfo));
    }

  /**
   * Set-up CBR or OnOff traffic with sink receivers
   */
	simulationHelper->InstallTrafficModel (
						model, SimulationHelper::UDP, SimulationHelper::FWD_LINK,
						senderAppStartTime, Seconds (simLength), MicroSeconds (20));


  NS_LOG_INFO ("--- sat-fwd-sys-test ---");
  NS_LOG_INFO ("  Packet size: " << packetSize.Get ());
  NS_LOG_INFO ("  Interval (CBR): " << interval.Get ().GetSeconds ());
  NS_LOG_INFO ("  Data rate (OnOff): " << dataRate.Get ());
  NS_LOG_INFO ("  Simulation length: " << simLength);
  NS_LOG_INFO ("  Number of GW end users: " << gwEndUsers);
  NS_LOG_INFO ("  ");

  /**
   * Store attributes into XML output
   */
  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("sat-fwd-sys-test.xml"));
  // Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  // Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  // ConfigStore outputConfig;
  // outputConfig.ConfigureDefaults ();

  /**
   * Run simulation
   */
  simulationHelper->RunSimulation ();

  return 0;
}


