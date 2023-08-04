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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
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
 * \file sat-vhts-example.cc
 * \ingroup satellite
 *
 * \brief This file allows to create a VHTS scenario
 */

NS_LOG_COMPONENT_DEFINE ("sat-vhts-example");

// TODO confirm that
void EnableRA (std::string raModel, bool dynamicLoadControl)
{
  // Enable Random Access with CRDSA or MARSALA
  if (raModel == "CRDSA")
  {
    Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_CRDSA));
  }
  else if (raModel == "MARSALA")
  {
    Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel", EnumValue (SatEnums::RA_MODEL_MARSALA));
  }
  else
  {
    NS_FATAL_ERROR ("Incorrect RA model");
  }

  // Set Random Access interference model
  Config::SetDefault ("ns3::SatBeamHelper::RaInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));

  // Set Random Access collision model
  Config::SetDefault ("ns3::SatBeamHelper::RaCollisionModel", EnumValue (SatPhyRxCarrierConf::RA_COLLISION_CHECK_AGAINST_SINR));

  // Set dynamic load control parameters
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableRandomAccessDynamicLoadControl", BooleanValue (dynamicLoadControl));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::RandomAccessAverageNormalizedOfferedLoadMeasurementWindowSize", UintegerValue (10));
}

static void
SatCourseChange (std::string context, Ptr<const SatMobilityModel> position)
{
  auto tracedPosition = DynamicCast<const SatTracedMobilityModel> (position);
  NS_ASSERT_MSG (tracedPosition != NULL, "Course changed for a non-mobile UT");
}

int
main (int argc, char *argv[])
{
  // Variables
  std::string beams = "8";
  uint32_t nbGw = 1;
  uint32_t nbUtsPerBeam = 10;
  uint32_t nbEndUsersPerUt = 1;

  Time appStartTime = Seconds (0.001);
  Time simLength = Seconds (60.0);

  std::string raModel = "CRDSA";
  bool dynamicLoadControl = true;
  bool utMobility = false;
  std::string mobilityPath = "contrib/satellite/data/utpositions/mobiles/scenario0/trajectory";
  std::string burstLengthStr = "ShortBurst";
  SatEnums::SatWaveFormBurstLength_t burstLength = SatEnums::SHORT_BURST;

  uint32_t superFrameConfForSeq0Int = 0;
  SatSuperframeConf::SuperFrameConfiguration_t superFrameConfForSeq0 = SatSuperframeConf::SUPER_FRAME_CONFIG_0;
  uint32_t frameConfigTypeInt = 0;
  SatSuperframeConf::ConfigType_t frameConfigType = SatSuperframeConf::CONFIG_TYPE_0;

  std::string modcodsUsed = "QPSK_1_TO_2 QPSK_3_TO_5 QPSK_2_TO_3 QPSK_3_TO_4 QPSK_4_TO_5 QPSK_5_TO_6 QPSK_8_TO_9 QPSK_9_TO_10 "
          "8PSK_3_TO_5 8PSK_2_TO_3 8PSK_3_TO_4 8PSK_5_TO_6 8PSK_8_TO_9 8PSK_9_TO_10 "
          "16APSK_2_TO_3 16APSK_3_TO_4 16APSK_4_TO_5 16APSK_5_TO_6 16APSK_8_TO_9 16APSK_9_TO_10 "
          "32APSK_3_TO_4 32APSK_4_TO_5 32APSK_5_TO_6 32APSK_8_TO_9";

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-vhts-example");

  // Read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("Beams", "Ids of beams used (each separated by _)", beams);
  cmd.AddValue ("NbGw", "Number of GWs", nbGw);
  cmd.AddValue ("NbUtsPerBeam", "Number of UTs per spot-beam", nbUtsPerBeam);
  cmd.AddValue ("NbEndUsersPerUt", "Number of end users per UT", nbEndUsersPerUt);
  cmd.AddValue ("AppStartTime", "Applications start time (in seconds, or add unit)", appStartTime);
  cmd.AddValue ("SimLength", "Simulation length (in seconds, or add unit)", simLength);
  cmd.AddValue ("RaModel", "Random Access model chosen (CRDSA or MARSALA)", raModel);
  cmd.AddValue ("DynamicLoadControl", "Set true to use dynamic load control", dynamicLoadControl);
  cmd.AddValue ("UtMobility", "Set true to use UT mobility", utMobility);
  cmd.AddValue ("mobilityPath", "Path to the mobility file", mobilityPath); // TODO works if only one UT ?
  cmd.AddValue ("BurstLength", "Burst length (can be ShortBurst, LongBurst or ShortAndLongBurst)", burstLengthStr);
  cmd.AddValue ("SuperFrameConfForSeq0", "Super frame configuration used for super frame sequence 0", superFrameConfForSeq0Int);
  cmd.AddValue ("FrameConfigType", "The frame configuration type used for super frame", frameConfigTypeInt);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  std::replace (beams.begin (), beams.end (), '_', ' ');

  // Initialize enum values
  if (burstLengthStr == "ShortBurst")
    {
      burstLength = SatEnums::SHORT_BURST;
    }
  else if (burstLengthStr == "LongBurst")
    {
      burstLength = SatEnums::LONG_BURST;
    }
  else if (burstLengthStr == "ShortAndLongBurst")
    {
      burstLength = SatEnums::SHORT_AND_LONG_BURST;
    }
  else
    {
      NS_FATAL_ERROR ("Incorrect burst size");
    }

  switch(superFrameConfForSeq0Int)
    {
      case 0:
        superFrameConfForSeq0 = SatSuperframeConf::SUPER_FRAME_CONFIG_0;
        break;
      case 1:
        superFrameConfForSeq0 = SatSuperframeConf::SUPER_FRAME_CONFIG_1;
        break;
      case 2:
        superFrameConfForSeq0 = SatSuperframeConf::SUPER_FRAME_CONFIG_2;
        break;
      case 3:
        superFrameConfForSeq0 = SatSuperframeConf::SUPER_FRAME_CONFIG_3;
        break;
      default:
        NS_FATAL_ERROR ("Incorrect super frame conf");
    }

  switch(frameConfigTypeInt)
    {
      case 0:
        frameConfigType = SatSuperframeConf::CONFIG_TYPE_0;
        break;
      case 1:
        frameConfigType = SatSuperframeConf::CONFIG_TYPE_1;
        break;
      case 2:
        frameConfigType = SatSuperframeConf::CONFIG_TYPE_2;
        break;
      case 3:
        frameConfigType = SatSuperframeConf::CONFIG_TYPE_3;
        break;
      default:
        NS_FATAL_ERROR ("Incorrect frame type");
    }

  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  /*
   * FWD link
   */
  // Set defaults
  Config::SetDefault ("ns3::SatConf::FwdUserLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatConf::FwdFeederLinkBandwidth", DoubleValue (8e+09));
  Config::SetDefault ("ns3::SatConf::FwdCarrierAllocatedBandwidth", DoubleValue (500e+06));
  Config::SetDefault ("ns3::SatConf::FwdCarrierRollOff", DoubleValue (0.05));

  // ModCods selection
  Config::SetDefault ("ns3::SatBeamHelper::DvbVersion", StringValue ("DVB_S2"));
  Config::SetDefault ("ns3::SatBbFrameConf::ModCodsUsed", StringValue (modcodsUsed));
  Config::SetDefault ("ns3::SatBbFrameConf::DefaultModCod", StringValue ("QPSK_1_TO_2"));

  /*
   * RTN link
   */
  // Set defaults
  Config::SetDefault ("ns3::SatConf::RtnUserLinkBandwidth", DoubleValue (500e+06));
  Config::SetDefault ("ns3::SatConf::RtnFeederLinkBandwidth", DoubleValue (2e+09));
  Config::SetDefault ("ns3::SatWaveformConf::BurstLength", EnumValue (burstLength));

  EnableRA (raModel, dynamicLoadControl);

  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (superFrameConfForSeq0));
  switch(superFrameConfForSeq0)
    {
      case SatSuperframeConf::SUPER_FRAME_CONFIG_0:
        Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", EnumValue (frameConfigType));
        break;
      case SatSuperframeConf::SUPER_FRAME_CONFIG_1:
        Config::SetDefault ("ns3::SatSuperframeConf1::FrameConfigType", EnumValue (frameConfigType));
        break;
      case SatSuperframeConf::SUPER_FRAME_CONFIG_2:
        Config::SetDefault ("ns3::SatSuperframeConf2::FrameConfigType", EnumValue (frameConfigType));
        break;
      case SatSuperframeConf::SUPER_FRAME_CONFIG_3:
        Config::SetDefault ("ns3::SatSuperframeConf3::FrameConfigType", EnumValue (frameConfigType));
        break;
      default:
        NS_FATAL_ERROR ("Impossible to reach here");
    }

  /*
   * Traffics
   */
  simulationHelper->SetSimulationTime (simLength);

  simulationHelper->SetGwUserCount (nbGw);
  simulationHelper->SetUtCountPerBeam (nbUtsPerBeam);
  simulationHelper->SetUserCountPerUt (nbEndUsersPerUt);
  simulationHelper->SetBeams (beams);

  simulationHelper->CreateSatScenario ();

  Ptr<SatHelper> satHelper = simulationHelper->GetSatelliteHelper ();
  Ptr<SatTrafficHelper> trafficHelper = simulationHelper->GetTrafficHelper ();
  trafficHelper->AddVoipTraffic (SatTrafficHelper::FWD_LINK,
                                 SatTrafficHelper::G_711_1,
                                 satHelper->GetGwUsers (),
                                 satHelper->GetUtUsers (),
                                 appStartTime,
                                 simLength,
                                 Seconds (0.001));
  trafficHelper->AddHttpTraffic (SatTrafficHelper::FWD_LINK,
                                 satHelper->GetGwUsers (),
                                 satHelper->GetUtUsers (),
                                 appStartTime,
                                 simLength,
                                 Seconds (0.001));

  // Link results
  // Uncomment to use custom C/N0 traces or constants for some links
  /*
  Ptr<SatCnoHelper> satCnoHelper = simulationHelper->GetCnoHelper ();
  satCnoHelper->UseTracesForDefault (false);
  for (uint32_t i = 0; i < satHelper->GetBeamHelper ()->GetUtNodes ().GetN (); i++)
    {
      satCnoHelper->SetUtNodeCnoFile (satHelper->GetBeamHelper ()->GetUtNodes ().Get (i), SatEnums::FORWARD_USER_CH, "path_to_cno_file"); // For input trace file
      // or
      satCnoHelper->SetGwNodeCno (satHelper->GetBeamHelper ()->GetUtNodes ().Get (i), SatEnums::FORWARD_USER_CH, 1e10); // For constant value
    }
  */

  // Mobility
  if (utMobility)
    {
      Ptr<SatMobilityModel> satMobility = satHelper->GetBeamHelper ()->GetGeoSatNodes ().Get (0)->GetObject<SatMobilityModel> ();
      Ptr<Node> node = satHelper->LoadMobileUtFromFile (0, mobilityPath);
      node->GetObject<SatMobilityModel> ()->TraceConnect ("SatCourseChange", "BeamTracer", MakeCallback (SatCourseChange));
    }

  /*
   * Outputs
   * Note: some outputs are automatically generated by traffic helper
   */
  simulationHelper->EnableProgressLogs ();

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();

  Ptr<SatStatsHelperContainer> s = simulationHelper->GetStatisticsContainer ();

  // Capacity request
  s->AddGlobalCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerGwCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Granted resources
  s->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalResourcesGranted (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtResourcesGranted (SatStatsHelper::OUTPUT_SCATTER_FILE);

  // Link SINR
  s->AddGlobalFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnFeederLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnUserLinkSinr (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // SINR
  s->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalFwdCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtFwdCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddGlobalRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerBeamRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_FILE);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerUtRtnCompositeSinr (SatStatsHelper::OUTPUT_CDF_PLOT);

  // Link RX Power
  s->AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCATTER_FILE);

  s->AddGlobalFwdFeederLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalFwdUserLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnFeederLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddGlobalRtnUserLinkRxPower (SatStatsHelper::OUTPUT_SCALAR_FILE);

  // Frame type usage
  s->AddGlobalFrameTypeUsage (SatStatsHelper::OUTPUT_SCALAR_FILE);

  if (raModel == "MARSALA")
    {
      s->AddPerBeamMarsalaCorrelation (SatStatsHelper::OUTPUT_SCALAR_FILE);
      s->AddPerBeamMarsalaCorrelation (SatStatsHelper::OUTPUT_SCATTER_FILE);
    }

  simulationHelper->RunSimulation ();
  return 0;

} // end of `int main (int argc, char *argv[])`
