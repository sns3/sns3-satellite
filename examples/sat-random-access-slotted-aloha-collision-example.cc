/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

/**
* \ingroup satellite
*
* \brief  Example of Random Access Slotted ALOHA usage in satellite network. This
*         example is modified to produce collisions with high probability.
*         The scripts is using CBR application in user defined scenario,
*         which means that user can change the scenario size quite to be
*         whatever between 1 and full scenario (72 beams). Currently it
*         is configured to using only one beam. CBR application is sending
*         packets in RTN link, i.e. from UT side to GW side. Packet trace
*         and KpiHelper are enabled by default. End user may change the
*         number of UTs and end users from the command line.
*
*         execute command -> ./waf --run "sat-random-access-slotted-aloha-collision-example --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-random-access-slotted-aloha-collision-example");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (30);
  uint32_t packetSize (20);
  Time interval (Seconds(0.01));
  Time simLength (Seconds(1.00));
  Time appStartTime = Seconds(0.01);

  // enable info logs
  LogComponentEnable ("sat-random-access-slotted-aloha-collision-example", LOG_LEVEL_INFO);
  //LogComponentEnable ("SatRandomAccess", LOG_LEVEL_INFO);
  //LogComponentEnable ("SatUtMac", LOG_LEVEL_INFO);
  LogComponentEnable ("SatPhyRxCarrier", LOG_LEVEL_INFO);
  LogComponentEnable ("SatInterference", LOG_LEVEL_INFO);

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("endUsersPerUt", "Number of end users per UT", endUsersPerUt);
  cmd.AddValue("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.Parse (argc, argv);

  // Configure error model
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_AVI);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));
  //Config::SetDefault ("ns3::SatUtMac::CrUpdatePeriod", TimeValue(Seconds(10.0)));

  // Enable Random Access with CRDSA
  Config::SetDefault ("ns3::SatBeamHelper::RandomAccessModel",EnumValue (SatEnums::RA_SLOTTED_ALOHA));

  // Create reference system, two options:
  // - "Scenario72"
  // - "Scenario98"
  std::string scenarioName = "Scenario72";
  //std::string scenarioName = "Scenario98";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  // create user defined scenario
  SatBeamUserInfo beamInfo = SatBeamUserInfo (utsPerBeam,endUsersPerUt);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[beamId] = beamInfo;
  helper->SetBeamUserInfo (beamMap);
  helper->EnablePacketTrace ();

  helper->CreateScenario (SatHelper::USER_DEFINED);

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));
  cbrHelper.SetAttribute("Interval", TimeValue (interval));
  cbrHelper.SetAttribute("PacketSize", UintegerValue (packetSize) );

  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port)));

  // initialized time values for simulation
  uint32_t maxTransmitters = utUsers.GetN ();

  ApplicationContainer gwApps;
  ApplicationContainer utApps;

  Time cbrStartDelay = appStartTime;

  // Cbr and Sink applications creation
  for ( uint32_t i = 0; i < maxTransmitters; i++)
    {
      cbrHelper.SetAttribute("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));
      sinkHelper.SetAttribute("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port))));

      utApps.Add(cbrHelper.Install (utUsers.Get (i)));
      gwApps.Add(sinkHelper.Install (gwUsers.Get (0)));

      cbrStartDelay += Seconds (0.05);

      utApps.Get(i)->SetStartTime (cbrStartDelay);
      utApps.Get(i)->SetStopTime (simLength);
    }

  // Add the created applications to CbrKpiHelper
  CbrKpiHelper kpiHelper (KpiHelper::KPI_RTN);
  kpiHelper.AddSink (gwApps);
  kpiHelper.AddSender (utApps);

  utApps.Start (appStartTime);
  utApps.Stop (simLength);

  NS_LOG_INFO("--- Cbr-user-defined-example ---");
  NS_LOG_INFO("  Packet size in bytes: " << packetSize);
  NS_LOG_INFO("  Packet sending interval: " << interval.GetSeconds ());
  NS_LOG_INFO("  Simulation length: " << simLength.GetSeconds ());
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  Simulator::Stop (simLength);
  Simulator::Run ();

  kpiHelper.Print ();

  Simulator::Destroy ();

  return 0;
}
