/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store.h"


using namespace ns3;

/**
* \ingroup satellite
*
* \brief Simulation script to execute system tests for the forward link.
*
* To get help of the command line arguments for the example,
* execute command -> ./waf --run "sat-dama-onoff-sim-tn9 --PrintHelp"
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
      << ", Duration: " << bbFrame->GetDuration()
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
  // Spot-beam served by GW1
  uint32_t beamId = 26;
  uint32_t gwEndUsers = 10;

  uint32_t testCase = 0;
  uint32_t trafficModel = 0;
  double simLength (400.0); // in seconds
  Time senderAppStartTime = Seconds (0.1);
  bool traceFrameInfo = true;
  bool traceMergeInfo = true;

  UintegerValue packetSize (128); // in bytes
  TimeValue interval (MicroSeconds (10));
  DataRateValue dataRate (DataRate (16000));

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
  cmd.AddValue ("senderAppStartTime", "Sender application (first) start time", senderAppStartTime);
  cmd.Parse (argc, argv);

  /**
   * Select test case to execute
   */

  switch (testCase)
    {
      case 0: // scheduler, ACM disabled
        Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (false));
        break;

      case 1: // scheduler, ACM enabled
        Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (true));
        break;

      case 2: // ACM enabled, one UT with one user, Markov + external fading
        Config::SetDefault ("ns3::SatBbFrameConf::AcmEnabled", BooleanValue (true));
        Config::SetDefault ("ns3::SatBeamHelper::FadingModel", StringValue ("FadingMarkov"));
        Config::SetDefault ("ns3::SatChannel::EnableExternalFadingInputTrace", BooleanValue (true));
        Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtFwdDownIndexFileName", StringValue ("Beam1_UT_fading_fwddwn_traces.txt"));
        Config::SetDefault ("ns3::SatFadingExternalInputTraceContainer::UtRtnUpIndexFileName", StringValue ("Beam1_UT_fading_rtnup_traces.txt"));
        gwEndUsers = 1;
        break;

      default:
        break;
    }

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";
  Config::SetDefault ("ns3::SatHelper::GwUsers", UintegerValue (gwEndUsers));

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  // create user defined scenario
  SatBeamUserInfo beamInfo = SatBeamUserInfo (gwEndUsers, 1);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[beamId] = beamInfo;

  helper->CreateUserDefinedScenario (beamMap);

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

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  /**
   * Set-up CBR or OnOff traffic with sink receivers
   */

  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address ());
  CbrHelper cbr ("ns3::UdpSocketFactory", Address ());
  SatOnOffHelper onOff ("ns3::UdpSocketFactory", Address ());

  // Create a packet sink to receive packets and CBR to sent packets

  for ( uint32_t i = 0; i < gwEndUsers; i++)
    {
      sink.SetAttribute ("Local", AddressValue (InetSocketAddress (helper->GetUserAddress (utUsers.Get(i)), port)));
      sink.Install (utUsers.Get (i));

      ApplicationContainer senderAppAdded;

      switch (trafficModel)
        {
          case 0: // CBR
            cbr.SetAttribute ("Remote", AddressValue (InetSocketAddress (helper->GetUserAddress (utUsers.Get(i)), port)));
            senderAppAdded = cbr.Install (gwUsers.Get(i));
            DynamicCast<CbrApplication> (senderAppAdded.Get (0))->GetAttribute ("PacketSize", packetSize );
            DynamicCast<CbrApplication> (senderAppAdded.Get (0))->GetAttribute ("Interval", interval );
            break;

          case 1: // On-Off
            onOff.SetAttribute ("Remote", AddressValue (InetSocketAddress (helper->GetUserAddress (utUsers.Get(i)), port)));
            senderAppAdded = onOff.Install (gwUsers.Get(i));
            DynamicCast<OnOffApplication> (senderAppAdded.Get (0))->GetAttribute ("PacketSize", packetSize );
            DynamicCast<OnOffApplication> (senderAppAdded.Get (0))->GetAttribute ("DataRate", dataRate );
            break;

          default:
            NS_FATAL_ERROR ("Not Supported Traffic Model!");
            break;
        }

      senderAppAdded.Get (0)->SetStartTime (senderAppStartTime);
      senderAppStartTime += MicroSeconds (20);
    }

  // enable info logs on Apps
    //LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
    //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

  // Enable some logs.
  LogComponentEnable ("sat-fwd-sys-test", LOG_INFO);

  NS_LOG_INFO("--- sat-sys-test ---");
  NS_LOG_INFO("  Packet size: " << packetSize.Get ());
  NS_LOG_INFO("  Interval (CBR): " << interval.Get ().GetSeconds ());
  NS_LOG_INFO("  Data rate (OnOff): " << dataRate.Get ());
  NS_LOG_INFO("  Simulation length: " << simLength);
  NS_LOG_INFO("  Number of GW end users: " << gwEndUsers);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("sat-sys-test.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();

  /**
   * Run simulation
   */
  Simulator::Stop (Seconds (simLength));
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}

