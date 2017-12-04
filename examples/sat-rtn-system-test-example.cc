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
 * \file sat-rtn-system-test-example.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results related to
 * satellite system tests.
 *
 * To get help of the command line arguments for the example,
 * execute command -> ./waf --run "sat-rtn-system-test-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-rtn-system-test-example");


// Callback called when RBDC CR has been sent by request manager
static void RbcdRcvdCb (uint32_t value)
{
  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s RBDC request generated with " << value << " kbps");
}

// Callback called when AVBDC CR has been sent by request manager
static void AvbcdRcvdCb (uint32_t value)
{
  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s AVBDC request generated with " << value << " Bytes");
}

// Callback called when VBDC CR has been sent by request manager
static void VbcdRcvdCb (uint32_t value)
{
  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s VBDC request generated with " << value << " Bytes");
}

// Callback called when VBDC CR has been sent by request manager
static void TbtpResources (uint32_t value)
{
  if (value > 0)
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s " << value << " Bytes allocated within TBTP");
    }
}


int
main (int argc, char *argv[])
{
  uint32_t beamId = 26;
  uint32_t endUsersPerUt = 10;
  uint32_t utsPerBeam = 10;
  Time utAppStartTime = Seconds (0.1);

  //UintegerValue packetSize (1280); // in bytes
  //TimeValue interval (MilliSeconds (500));
  //DataRateValue dataRate (16000);

  double simLength (30.0); // in seconds
  std::string preDefinedFrameConfig = "Configuration_0";
  uint32_t testCase = 0;
  uint32_t trafficModel = 0;

  LogComponentEnable ("sat-rtn-system-test-example", LOG_LEVEL_INFO);

  /// Set simulation output details
  std::string simulationName = "example-rtn-system-test";
	auto simulationHelper = CreateObject<SimulationHelper> (simulationName);

  // set default values for traffic model apps here
  // attributes can be overridden by command line arguments or input xml when needed
  //Config::SetDefault ("ns3::CbrApplication::PacketSize", packetSize);
  //Config::SetDefault ("ns3::CbrApplication::Interval", interval);
  //Config::SetDefault ("ns3::OnOffApplication::PacketSize", packetSize);
  //Config::SetDefault ("ns3::OnOffApplication::DataRate", dataRate);
  Config::SetDefault ("ns3::OnOffApplication::OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
  Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));

  // To read input attributes from input xml-file
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/sys-rtn-test.xml";

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("testCase", "Test case to execute", testCase);
  cmd.AddValue ("frameConf", "Pre-defined super frame configuration", preDefinedFrameConfig);
  cmd.AddValue ("trafficModel", "Traffic model to use (0 = CBR, 1 = OnOff)", trafficModel);
  cmd.AddValue ("simLength", "Simulation length", simLength);
  cmd.AddValue ("beamId", "Beam Id", beamId);
  cmd.AddValue ("utAppStartTime", "Application (first) start time.", utAppStartTime);
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (inputFileNameWithPath));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputFrameConfig;
  inputFrameConfig.ConfigureDefaults ();

  // select pre-defined super frame configuration wanted to use.
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", StringValue (preDefinedFrameConfig));

  switch (testCase)
    {
    case 0: // scheduler, CRA, ACM is selected by command line arguments ( --"ns3::SatWaveformConf::AcmEnabled=true" or --"ns3::SatWaveformConf::AcmEnabled=false" )
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=2]"));
      Config::SetDefault ("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue (false));
      break;

    case 1: // scheduler, FCA (CRA + VBDC), ACM is selected by command line arguments ( --"ns3::SatWaveformConf::AcmEnabled=true" or --"ns3::SatWaveformConf::AcmEnabled=false" )
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=2]"));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));
      Config::SetDefault ("ns3::SatSuperframeAllocator::FcaEnabled", BooleanValue (true));
      break;

    case 2: // ACM, one UT with one user, MARKOV fading on, external fading on
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
      Config::SetDefault ("ns3::SatWaveformConf::AcmEnabled", BooleanValue (true));
      Config::SetDefault ("ns3::SatBeamHelper::FadingModel", StringValue ("FadingMarkov"));

      // Note, that the positions of the fading files do not necessarily match with the
      // beam location, since this example is not using list position allocator!
      Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
      Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName", StringValue ("BeamId-1_256_UT_fading_fwddwn_trace_index.txt"));
      Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName", StringValue ("BeamId-1_256_UT_fading_rtnup_trace_index.txt"));
      endUsersPerUt = 1;
      utsPerBeam = 1;
      break;

    case 3: // RM, one UT with one user, CRA only

      /**
       * DAMA is disabled entirely, so there should not be any generated capacity requests
       * which ever traffic model is utilized. CRA is configured with 100 kbps, thus it is
       * the maximum throughput the scheduler can give to the UT.
       */
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (true));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=100]"));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

      endUsersPerUt = 1;
      utsPerBeam = 1;

      break;

    case 4: // RM, one UT with one user, RBDC only

      /**
       * RBDC is allowed for BE traffic queue.
       */
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (true));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (false));

      // Change ways of sending the capacity requests!

      // CRA
      //Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
      //Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));

      // Slotted Aloha
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
      //Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", StringValue ("RaCollisionConstantErrorProbability"));
      //Config::SetDefault ("ns3::SatBeamHelper::RaConstantErrorRate", DoubleValue (0.3));

      // CRDSA
      //Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_CRDSA));
      //Config::SetDefault ("ns3::SatUtHelper::UseCrdsaOnlyForControlPackets", BooleanValue (true));

      // Periodical control slots
      //Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));

      Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));

      endUsersPerUt = 1;
      utsPerBeam = 1;

      break;

    case 5: // RM, one UT with one user, VBDC only

      /**
       * VBDC is allowed for BE traffic queue.
       */
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_ConstantAssignmentProvided", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_RbdcAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService2_VolumeAllowed", BooleanValue (false));
      Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService3_VolumeAllowed", BooleanValue (true));

      // Change ways of sending the capacity requests!

      // CRA
      //Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue (true));
      //Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_OFF));

      // Slotted Aloha
      Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_SLOTTED_ALOHA));
      //Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", StringValue ("RaCollisionConstantErrorProbability"));
      //Config::SetDefault ("ns3::SatBeamHelper::RaConstantErrorRate", DoubleValue (0.3));

      // CRDSA
      //Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_CRDSA));
      //Config::SetDefault ("ns3::SatUtHelper::UseCrdsaOnlyForControlPackets", BooleanValue (true));

      // Periodical control slots
      //Config::SetDefault ("ns3::SatBeamScheduler::ControlSlotsEnabled", BooleanValue (true));

      Config::SetDefault ("ns3::OnOffApplication::OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=10.0|Bound=0.0]"));

      endUsersPerUt = 1;
      utsPerBeam = 1;
      break;

    default:
      break;
    }

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  simulationHelper->SetBeamSet ({beamId});
  simulationHelper->SetSimulationTime (simLength);

  // Create satellite scenario
  simulationHelper->CreateSatScenario ();

  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatLlc/SatRequestManager/RbdcTrace",
                                 MakeCallback (&RbcdRcvdCb));

  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatLlc/SatRequestManager/VbdcTrace",
                                 MakeCallback (&VbcdRcvdCb));

  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatLlc/SatRequestManager/AvbdcTrace",
                                 MakeCallback (&AvbcdRcvdCb));

  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatMac/DaResourcesTrace",
                                 MakeCallback (&TbtpResources));

  //Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/SatLlc/SatEncapsulator/SatQueue/Enqueue",
  //                               MakeCallback (&PacketEnqueu));

  /**
   * Set-up CBR or OnOff traffic
   */
  SimulationHelper::TrafficModel_t model;

	switch (trafficModel)
		{
		case 0:   // CBR
			model = SimulationHelper::CBR;
			break;

		case 1:   // On-Off
			model = SimulationHelper::ONOFF;
			break;

		default:
			NS_FATAL_ERROR ("Not Supported Traffic Model!");
			break;
		};

  simulationHelper->InstallTrafficModel (
  		model,
			SimulationHelper::UDP,
			SimulationHelper::RTN_LINK,
			utAppStartTime, Seconds (simLength+1), Seconds (0.1));

  /**
   * Set-up statistics
   */
  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnMacThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddAverageUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddAverageUtUserRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnAppDelay (SatStatsHelper::OUTPUT_CDF_PLOT);

  s->AddPerBeamRtnDaPacketError (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamFrameSymbolLoad (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamWaveformUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // enable application info logs
  //LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);


  NS_LOG_INFO ("--- sat-rtn-sys-test ---");
  NS_LOG_INFO ("  Test case: " << testCase);
  NS_LOG_INFO ("  Traffic model: " << trafficModel);
  //NS_LOG_INFO("  Packet size: " << packetSize.Get ());
  //NS_LOG_INFO("  Data rate (on-off): " << dataRate.Get ());
  //NS_LOG_INFO("  Interval (CBR): " << interval.Get ().GetSeconds ());
  NS_LOG_INFO ("  Simulation length: " << simLength);
  NS_LOG_INFO ("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO ("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO ("  ");

  /**
   * Store attributes into XML output
   */
  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("sat-rtn-sys-test.xml"));
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

