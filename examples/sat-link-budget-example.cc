/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/cbr-helper.h"

using namespace ns3;

/**
* \ingroup satellite
*
* \brief  Link budget example application for link budget calculation purposes in satellite network.
*         Some parameter can be set as command line arguments.
*         To see help for user arguments:
*         execute command -> ./waf --run "sat-link-budget-example --PrintHelp"
*
*         Application sends one packet from GW connected user to UT connected users and
*         from UT connected user to GW connected user. User defined scenario is created with given beam ID.
*
*         Used parameters and link budget calculation result are printed as long info.
*/

NS_LOG_COMPONENT_DEFINE ("sat-link-budget-example");

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
                    << params->m_carrierFreq_hz << " "
                    << SatUtils::WToDbW (ifPower) << " "
                    << SatUtils::WToDbW ( params->m_rxPower_W ) << " "
                    << SatUtils::LinearToDb (params->m_sinr) << " "
                    << SatUtils::LinearToDb (cSinr) );
    }
}

int
main (int argc, char *argv[])
{
  // parameters for link budget calculation
  uint32_t beamId = 1;
  double latitude = 50.00;
  double longitude = -1.00;
  double altitude = 0.00;

  // read command line parameters can be given by user
  CommandLine cmd;
  cmd.AddValue ("beam", "Beam to use for testing. (1 - 72)", beamId);
  cmd.AddValue ("latitude", "Latitude of UT position (-90 ... 90.0)", latitude);
  cmd.AddValue ("longitude", "Longitude of UT position (-180 ... 180)", longitude);
  cmd.AddValue ("altitude", "Altitude of UT position (meters)", altitude);
  cmd.Parse (argc, argv);

  // enable info logs
  LogComponentEnable ("sat-link-budget-example", LOG_LEVEL_INFO);

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  std::string scenarioName = "Scenario72";

  // create helpers
  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  // configure helpers
  Ptr<SatBeamHelper> beamHelper = helper->GetBeamHelper();
  Ptr<SatUtHelper> utHelper = beamHelper->GetUtHelper();
  Ptr<SatGwHelper> gwHelper = beamHelper->GetGwHelper();
  Ptr<SatGeoHelper> geoHelper = beamHelper->GetGeoHelper();

  // These are default values used by different PHY objects.
  // Uncomment a line and change attribute value to override default

//  gwHelper->SetPhyAttribute ("RxTemperatureDbk", DoubleValue (24.62));
//  gwHelper->SetPhyAttribute ("RxMaxAntennaGainDb", DoubleValue (61.50));
//  gwHelper->SetPhyAttribute ("TxMaxAntennaGainDb", DoubleValue (65.20));
//  gwHelper->SetPhyAttribute ("TxMaxPowerDbw", DoubleValue (8.97));
//  gwHelper->SetPhyAttribute ("TxOutputLossDb", DoubleValue (2.00) );
//  gwHelper->SetPhyAttribute ("TxPointingLossDb", DoubleValue (1.10));
//  gwHelper->SetPhyAttribute ("TxOboLossDb", DoubleValue (6.00));
//  gwHelper->SetPhyAttribute ("TxAntennaLossDb", DoubleValue (0.00));
//  gwHelper->SetPhyAttribute ("RxAntennaLossDb", DoubleValue (0.00));
//  gwHelper->SetPhyAttribute ("DefaultFadingValue", DoubleValue (1.00));
//  gwHelper->SetPhyAttribute ("ImIfCOverIDb", DoubleValue (22.0));
//  gwHelper->SetPhyAttribute ("AciIfWrtNoisePercent", DoubleValue (10.0));

//  utHelper->SetPhyAttribute ("RxTemperatureDbk", DoubleValue (24.62));
//  utHelper->SetPhyAttribute ("RxMaxAntennaGainDb", DoubleValue (44.60));
//  utHelper->SetPhyAttribute ("TxMaxAntennaGainDb", DoubleValue (45.20));
//  utHelper->SetPhyAttribute ("TxMaxPowerDbw", DoubleValue (4.00));
//  utHelper->SetPhyAttribute ("TxOutputLossDb", DoubleValue (0.50) );
//  utHelper->SetPhyAttribute ("TxPointingLossDb", DoubleValue (1.00));
//  utHelper->SetPhyAttribute ("TxOboLossDb", DoubleValue (0.50));
//  utHelper->SetPhyAttribute ("TxAntennaLossDb", DoubleValue (1.00));
//  utHelper->SetPhyAttribute ("RxAntennaLossDb", DoubleValue (0.00));
//  utHelper->SetPhyAttribute ("DefaultFadingValue", DoubleValue (1.00));
//  utHelper->SetPhyAttribute ("OtherSysIfCOverIDb", DoubleValue (24.7));
//
//  geoHelper->SetUserPhyAttribute ("RxTemperatureDbk", DoubleValue (28.4));
//  geoHelper->SetUserPhyAttribute ("RxMaxAntennaGainDb", DoubleValue (54.00));
//  geoHelper->SetUserPhyAttribute ("TxMaxAntennaGainDb", DoubleValue (54.00));
//  geoHelper->SetUserPhyAttribute ("TxMaxPowerDbw", DoubleValue (15.00));
//  geoHelper->SetUserPhyAttribute ("TxOutputLossDb", DoubleValue (2.85) );
//  geoHelper->SetUserPhyAttribute ("TxPointingLossDb", DoubleValue (0.00));
//  geoHelper->SetUserPhyAttribute ("TxOboLossDb", DoubleValue (0.00));
//  geoHelper->SetUserPhyAttribute ("TxAntennaLossDb", DoubleValue (1.00));
//  geoHelper->SetUserPhyAttribute ("RxAntennaLossDb", DoubleValue (1.00));
//  geoHelper->SetUserPhyAttribute ("DefaultFadingValue", DoubleValue (1.00));
//  geoHelper->SetUserPhyAttribute ("OtherSysIfCOverIDb", DoubleValue (27.5));
//  geoHelper->SetUserPhyAttribute ("AciIfCOverIDb", DoubleValue (17.0));
//
//  geoHelper->SetFeederPhyAttribute ("RxTemperatureDbk", DoubleValue (28.40));
//  geoHelper->SetFeederPhyAttribute ("RxMaxAntennaGainDb", DoubleValue (54.00));
//  geoHelper->SetFeederPhyAttribute ("TxMaxAntennaGainDb", DoubleValue (54.00));
//  geoHelper->SetFeederPhyAttribute ("TxMaxPowerDbw", DoubleValue (-4.38));
//  geoHelper->SetFeederPhyAttribute ("TxOutputLossDb", DoubleValue (1.75) );
//  geoHelper->SetFeederPhyAttribute ("TxPointingLossDb", DoubleValue (1.10));
//  geoHelper->SetFeederPhyAttribute ("TxOboLossDb", DoubleValue (4.00));
//  geoHelper->SetFeederPhyAttribute ("TxAntennaLossDb", DoubleValue (1.00));
//  geoHelper->SetFeederPhyAttribute ("RxAntennaLossDb", DoubleValue (1.00));
//  geoHelper->SetFeederPhyAttribute ("DefaultFadingValue", DoubleValue (1.00));
//  geoHelper->SetFeederPhyAttribute ("ExtNoisePowerDensityDbwhz", DoubleValue (-207.0));
//  geoHelper->SetFeederPhyAttribute ("ImIfCOverIDb", DoubleValue (27.0));

  // create user defined scenario
  SatBeamUserInfo beamInfo = SatBeamUserInfo (1,1);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[beamId] = beamInfo;
  helper->SetBeamUserInfo (beamMap);

  helper->CreateScenario (SatHelper::USER_DEFINED);

  // set callback traces where we want results out

  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/PhyRx/RxCarrierList/*/PacketTrace",
                               MakeCallback (&PacketTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/*/PhyRx/RxCarrierList/*/PacketTrace",
                               MakeCallback (&PacketTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/*/PhyRx/RxCarrierList/*/PacketTrace",
                                 MakeCallback (&PacketTraceCb));
  // Set UT position
  NodeContainer ut = helper->UtNodes ();
  Ptr<SatMobilityModel> utMob = ut.Get (0)->GetObject<SatMobilityModel> ();
  utMob->SetGeoPosition (GeoCoordinate (latitude, longitude, altitude));

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  uint16_t port = 9;

  // create application on GW user
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress(gwUsers.Get(0)), port));
  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress(helper->GetUserAddress(utUsers.Get(0)), port));
  cbrHelper.SetAttribute ("Interval", StringValue ("0.1s"));
  cbrHelper.SetAttribute ("PacketSize", UintegerValue (512) );

  ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
  gwSink.Start (Seconds (0.1));
  gwSink.Stop (Seconds (1.0));

  ApplicationContainer gwCbr = cbrHelper.Install (gwUsers.Get (0));
  gwCbr.Start (Seconds (0.1));
  gwCbr.Stop (Seconds (0.25));

  // create application on UT user
  sinkHelper.SetAttribute ("Local", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get(0)), port))));
  cbrHelper.SetAttribute ("Remote", AddressValue(Address (InetSocketAddress (helper->GetUserAddress (gwUsers.Get(0)), port))));

  ApplicationContainer utSink = sinkHelper.Install (utUsers.Get (0));
  utSink.Start (Seconds (0.1));
  utSink.Stop (Seconds (1.0));

  ApplicationContainer utCbr = cbrHelper.Install (utUsers.Get (0));
  utCbr.Start (Seconds (0.1));
  utCbr.Stop (Seconds (0.25));

  NodeContainer gw = helper->GwNodes ();
  Ptr<SatMobilityModel> gwMob = gw.Get (0)->GetObject<SatMobilityModel> ();

  Ptr<Node> geo = helper->GeoSatNode ();
  Ptr<SatMobilityModel> geoMob = geo->GetObject<SatMobilityModel> ();

  // print used parameters using log info
  NS_LOG_INFO ("--- Satellite link budget ---");
  NS_LOG_INFO (" Beam ID: " << beamId);
  NS_LOG_INFO (" Geo position: " << geoMob->GetGeoPosition () << " " << geoMob->GetPosition () );
  NS_LOG_INFO (" GW position: " << gwMob->GetGeoPosition () << " " << gwMob->GetPosition () );
  NS_LOG_INFO (" UT position: " << utMob->GetGeoPosition () << " " << utMob->GetPosition ());
  NS_LOG_INFO ("  ");
  NS_LOG_INFO ("Link results (Time, Channel type, Own address, Dest. address, Beam ID, Carrier Center freq, IF Power, RX Power, SINR, Composite SINR) :");
  // results are printed out in callback (PacketTraceCb)

  Simulator::Stop (Seconds (1.1));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
