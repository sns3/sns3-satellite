/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 */

/**
 * \file satellite-per-packet-if-test.cc
 * \ingroup satellite
 * \brief System test cases for Satellite Per-Packet Interference Model.
 */

// Include a header file from your module to test.
#include <iomanip>
#include "ns3/singleton.h"
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/applications-module.h"
#include "ns3/satellite-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

static void LinkBudgetTraceCb ( std::string context, Ptr<SatSignalParameters> params,
                                Mac48Address ownAdd, Mac48Address destAdd,
                                double ifPower, double cSinr)
{
  // print only unicast message to prevent printing control messages like TBTP messages
  if ( !destAdd.IsBroadcast () )
    {
      std::cout.precision (2);
      std::cout.setf (std::cout.fixed | std::cout.showpoint);

      std::cout << "Time= " << Simulator::Now ()
                << ", ChType= " << std::setw (17) << SatEnums::GetChannelTypeName (params->m_channelType)
                << ", OwnAddr= " << ownAdd
                << ", DestAddr= " << destAdd
                << ", Beam= " << std::setw (2) << params->m_beamId
                << ", Freq= " << params->m_carrierFreq_hz
                << ", IFPwr= " << std::setw (8) << SatUtils::WToDbW<double> ( ifPower )
                << ", RXPwr= " << std::setw (8) << SatUtils::WToDbW<double> ( params->m_rxPower_W )
                << ", SINR= " << std::setw (7) << SatUtils::LinearToDb<double> (params->m_sinr)
                << ", CSINR= " << std::setw (7) << SatUtils::LinearToDb<double> (cSinr) << std::endl;
    }
}

class SatPerPacketBaseTestCase : public TestCase
{
public:
  SatPerPacketBaseTestCase ();
  SatPerPacketBaseTestCase (std::string name, std::string extName, SatEnums::FadingModel_t fading, bool dummyFrames,
                            StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2);
  virtual ~SatPerPacketBaseTestCase ();

protected:
  virtual void DoRun (void) = 0;
  void InitOutput ( bool figureOutput );
  void PrintTraceInfo ( );

  StringValue m_Interval1;
  StringValue m_Interval2;

  UintegerValue m_PackageSize1;
  UintegerValue m_PackageSize2;

  std::string m_extName;

  SatEnums::FadingModel_t m_fading;
  bool m_dummyFrames;

};

SatPerPacketBaseTestCase::SatPerPacketBaseTestCase ()
  : TestCase (""),
    m_Interval1 (),
    m_Interval2 (),
    m_PackageSize1 (),
    m_PackageSize2 (),
    m_extName (),
    m_fading (),
    m_dummyFrames (false)
{

}

SatPerPacketBaseTestCase::SatPerPacketBaseTestCase (std::string name, std::string extName, SatEnums::FadingModel_t fading, bool dummyFrames,
                                                    StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2)
  : TestCase (name),
    m_Interval1 (ival1),
    m_Interval2 (ival2),
    m_PackageSize1 (pSize1),
    m_PackageSize2 (pSize2),
    m_extName (extName),
    m_fading (fading),
    m_dummyFrames (dummyFrames)
{

}

SatPerPacketBaseTestCase::~SatPerPacketBaseTestCase ()
{

}

void
SatPerPacketBaseTestCase::InitOutput ( bool figureOutput )
{
  Ptr<SatIdMapper> ptrMapper = Singleton<SatIdMapper>::Get ();
  ptrMapper->Reset ();

  Ptr<SatInterferenceOutputTraceContainer> ptrCont = Singleton<SatInterferenceOutputTraceContainer>::Get ();
  ptrCont->Reset ();

  std::size_t pos = m_extName.find (' ');

  while ( pos != std::string::npos )
    {
      m_extName.replace ( pos, 1, "_");
      pos = m_extName.find (' ');
    }

  pos = m_extName.find (',');

  while ( pos != std::string::npos )
    {
      m_extName.replace ( pos, 1, "\0");
      pos = m_extName.find (',');
    }

  pos = m_extName.find ('.');

  while ( pos != std::string::npos )
    {
      m_extName.replace ( pos, 1, "\0");
      pos = m_extName.find ('.');
    }

  ptrCont->EnableFigureOutput (figureOutput);
}

void
SatPerPacketBaseTestCase::PrintTraceInfo ( )
{
  Ptr<SatIdMapper> ptrMapper = Singleton<SatIdMapper>::Get ();

  std::cout << m_extName << std::endl;
  ptrMapper->PrintTraceMap ();
}

/**
 * \ingroup satellite
 * \brief Per-packet interference, Forward Link System test case.
 *        User defined scenario created without fading and dummy frame sending.
 *
 *  Pre-conditions:
 *    Network is configured to use only one carrier in forward link.
 *    Per-packet interference is configured on for forward link.
 *
 *  This case tests per-packet interference in forward link with full or user defined scenario.
 *    1.  User-defined test scenario set with helper.
 *    2.  User Node-1 sends packets continuously, transmitting constant bitrate (CBR) to user Node-3.
 *    3.  User Node-2 sends short packets with e.g. 100ms delay between packets to user Node-4.
 *
 *  Expected result:
 *    UT serving Node-3 should see interference only in some of the received packet (when Node-2 is sending).
 *    UT serving Node-4 should see inference in every packet it receives.
 *
 *  Notes:
 *    Node-1 and Node-2 are attached to different IP router (different GW nodes) and Node-3 and Node-4 are attached to different UT nodes.
 *    Node-1 and Node-2 must be selected from different beams, so that packet sending happens in same channel at least in feeder or user link.
 */

// Default name to show for SatPerPacketFwdLinkUserTestCase test. When the second constructor is used, the text to append this name can be given.
static const char * defFwdUserName = "Test satellite per packet interference in Forward Link with user defined scenario. ";

class SatPerPacketFwdLinkUserTestCase : public SatPerPacketBaseTestCase
{
public:
  SatPerPacketFwdLinkUserTestCase ();
  SatPerPacketFwdLinkUserTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames);
  SatPerPacketFwdLinkUserTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames,
                                   StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2);

  virtual ~SatPerPacketFwdLinkUserTestCase ();

private:
  virtual void DoRun (void);

};

SatPerPacketFwdLinkUserTestCase::SatPerPacketFwdLinkUserTestCase ()
  : SatPerPacketBaseTestCase (defFwdUserName, "IfTestFwdUsr", SatEnums::FADING_OFF, false,
                              StringValue ("0.00015s"), UintegerValue (512), StringValue ("0.1s"), UintegerValue (512))
{

}

SatPerPacketFwdLinkUserTestCase::SatPerPacketFwdLinkUserTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames)
  : SatPerPacketBaseTestCase (defFwdUserName + name, "IfTestFwdUsr_" + name, fading, dummyFrames,
                              StringValue ("0.00015s"), UintegerValue (512), StringValue ("0.1s"), UintegerValue (512) )
{

}

SatPerPacketFwdLinkUserTestCase::SatPerPacketFwdLinkUserTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames,
                                                                  StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2)
  : SatPerPacketBaseTestCase (defFwdUserName + name, "IfTestFwdUsr_" + name, fading, dummyFrames, ival1, pSize1, ival2, pSize2 )
{

}

SatPerPacketFwdLinkUserTestCase::~SatPerPacketFwdLinkUserTestCase ()
{
}

void
SatPerPacketFwdLinkUserTestCase::DoRun (void)
{
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();

  InitOutput (true);

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-per-packet-if", m_extName, true);

  // Configure a static error probability
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_NONE);
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));
  Config::SetDefault ("ns3::SatGwHelper::RtnLinkErrorModel", EnumValue (em));

  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (m_fading));
  Config::SetDefault ("ns3::SatFwdLinkScheduler::DummyFrameSendingEnabled", BooleanValue (m_dummyFrames));

  Config::SetDefault ("ns3::SatHelper::UtCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatHelper::UtUsers", UintegerValue (1));
  Config::SetDefault ("ns3::SatGeoHelper::DaFwdLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatUtHelper::DaFwdLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace", BooleanValue (true));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();

  // create user defined scenario with beams 1 and 5
  SatBeamUserInfo beamInfo = SatBeamUserInfo (1,1);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[1] = beamInfo;
  beamMap[5] = beamInfo;

  helper->CreateUserDefinedScenario (beamMap);

  // set callback traces where we want results out

  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();
  uint16_t port = 9;

  // create Sink helper
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port));

  // install Sink application on UT user 1 to receive packet
  ApplicationContainer utSinks = sinkHelper.Install (utUsers.Get (0));

  // install Sink application on UT user 5 to receive packet
  sinkHelper.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (1)), port))));
  utSinks.Add (sinkHelper.Install (utUsers.Get (1)));
  utSinks.Start (Seconds (0.1));
  utSinks.Stop (Seconds (0.5));

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port));

  // create application on GW to sent beam 1 (UT user 1)
  cbrHelper.SetAttribute ("Interval", m_Interval1 );
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize1 );


  ApplicationContainer gw1Cbr = cbrHelper.Install (gwUsers.Get (0));
  gw1Cbr.Start (Seconds (0.2));
  gw1Cbr.Stop (Seconds (0.5));

  // create application on GW to sent beam 5 (UT user 5)
  cbrHelper.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (1)), port))));
  cbrHelper.SetAttribute ("Interval", m_Interval2 );
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize2 );

  ApplicationContainer gw2Cbr = cbrHelper.Install (gwUsers.Get (1));
  gw2Cbr.Start (Seconds (0.2));
  gw2Cbr.Stop (Seconds (1.0));

  Simulator::Stop (Seconds (1.0));
  Simulator::Run ();
  PrintTraceInfo ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Per-packet interference, Forward Link System test case.
 *        Full scenario created without fading and dummy frame sending.
 *
 *  Pre-conditions:
 *    Network is configured to use only one carrier in forward link.
 *    Per-packet interference is configured on for forward link.
 *
 *  This case tests per-packet interference in forward link with full or user defined scenario.
 *    1.  Full test scenario set with helper.
 *    2.  User Node-1 sends packets continuously, transmitting constant bitrate (CBR) to user Node-3.
 *    3.  User Node-2 sends short packets with e.g. 100ms delay between packets to user Node-4.
 *
 *  Expected result:
 *    UT serving Node-3 should see interference only in some of the received packet (when Node-2 is sending).
 *    UT serving Node-4 should see inference in every packet it receives.
 *
 *  Notes:
 *    Node-1 and Node-2 are attached to different IP router (different GW nodes) and Node-3 and Node-4 are attached to different UT nodes.
 *    Node-1 and Node-2 must be selected from different beams, so that packet sending happens in same channel at least in feeder or user link.
 */

// Default name to show for SatPerPacketFwdLinkFullTestCase test. When the second constructor is used, the text to append this name can be given.
static const char * defFwdFullName = "Test satellite per packet interference in Forward Link with fullscenario. ";


class SatPerPacketFwdLinkFullTestCase : public SatPerPacketBaseTestCase
{
public:
  SatPerPacketFwdLinkFullTestCase ();
  SatPerPacketFwdLinkFullTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames);
  SatPerPacketFwdLinkFullTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames,
                                   StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2);
  virtual ~SatPerPacketFwdLinkFullTestCase ();

private:
  virtual void DoRun (void);
};

SatPerPacketFwdLinkFullTestCase::SatPerPacketFwdLinkFullTestCase ()
  : SatPerPacketBaseTestCase (defFwdFullName, "IfTestFwdFull", SatEnums::FADING_OFF, false,
                              StringValue ("0.00015s"), UintegerValue (512), StringValue ("0.1s"),  UintegerValue (512) )

{

}

SatPerPacketFwdLinkFullTestCase::SatPerPacketFwdLinkFullTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames)
  : SatPerPacketBaseTestCase (defFwdFullName + name, "IfTestFwdFull_" + name, fading, dummyFrames,
                              StringValue ("0.00015s"), UintegerValue (512), StringValue ("0.1s"),  UintegerValue (512) )
{

}

SatPerPacketFwdLinkFullTestCase::SatPerPacketFwdLinkFullTestCase (std::string name, SatEnums::FadingModel_t fading, bool dummyFrames,
                                                                  StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2)
  : SatPerPacketBaseTestCase (defFwdFullName + name, "IfTestFwdFull_" + name, fading, dummyFrames,
                              ival1, pSize1, ival2, pSize2 )
{

}

SatPerPacketFwdLinkFullTestCase::~SatPerPacketFwdLinkFullTestCase ()
{

}

void
SatPerPacketFwdLinkFullTestCase::DoRun (void)
{
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();

  InitOutput ( false );

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-per-packet-if", m_extName, true);


  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (m_fading));
  Config::SetDefault ("ns3::SatFwdLinkScheduler::DummyFrameSendingEnabled", BooleanValue (m_dummyFrames));

  Config::SetDefault ("ns3::SatHelper::UtCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatHelper::UtUsers", UintegerValue (1));
  Config::SetDefault ("ns3::SatGeoHelper::DaFwdLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatUtHelper::DaFwdLinkInterferenceModel", EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace",BooleanValue (true));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();

  helper->CreatePredefinedScenario (SatHelper::FULL);

  // set callback traces where we want results out

  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();
  uint16_t port = 9;

  // create Sink helper
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port));

  // install Sink application on UT user 1 to receive packet
  ApplicationContainer utSinks = sinkHelper.Install (utUsers.Get (0));

  // install Sink application on UT user 5 to receive packet
  sinkHelper.SetAttribute ("Local", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (4)), port))));
  utSinks.Add (sinkHelper.Install (utUsers.Get (4)));
  utSinks.Start (Seconds (0.1));
  utSinks.Stop (Seconds (1.0));

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (utUsers.Get (0)), port));

  // create application on GW to sent beam 1 (UT user 1)
  cbrHelper.SetAttribute ("Interval", m_Interval1);
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize1 );


  ApplicationContainer gw1Cbr = cbrHelper.Install (gwUsers.Get (0));
  gw1Cbr.Start (Seconds (0.2));
  gw1Cbr.Stop (Seconds (0.4));

  // create application on GW to sent beam 5 (UT user 5)
  cbrHelper.SetAttribute ("Remote", AddressValue (Address (InetSocketAddress (helper->GetUserAddress (utUsers.Get (4)), port))));
  cbrHelper.SetAttribute ("Interval", m_Interval2);
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize2 );

  ApplicationContainer gw2Cbr = cbrHelper.Install (gwUsers.Get (4));
  gw2Cbr.Start (Seconds (0.2));
  gw2Cbr.Stop (Seconds (0.4));

  Simulator::Stop (Seconds (1.0));
  Simulator::Run ();
  PrintTraceInfo ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Per-packet interference, Return Link System test case.
 *
 *  Pre-conditions:
 *    Network is configured to use only one carrier in return link.
 *    Per-packet interference is configured on for return link.
 *
 *  This case tests per-packet interference in return link with full or user defined scenario.
 *    1.  Full or user-defined test scenario set with helper.
 *    2.  User Node-3 sends packets continuously, transmitting constant bitrate (CBR) to user Node-1.
 *    3.  User Node-4 sends short packets with e.g. 100ms delay between packets to user Node-2.
 *
 *  Expected result:
 *    GW serving Node-1 should see interference only in some of the received packet (when Node-4 is sending).
 *    GW serving Node-2 should see inference in every packet it receives.
 *
 *  Notes:
 *    Node-1 and Node-2 are attached to IP router (different GW nodes) and Node-3 and Node-4 are attached to different UT nodes.
 *    Node-1 and Node-2 must be selected from different beams, so that packet sending happens in same channel at least in user or feeder link.
 */

// Default name to show for SatPerPacketRtnLinkUserTestCase test. When the second constructor is used, the text to append this name can be given.
static const char * defRtnUserName = "Test satellite per packet interference in Return Link with user defined scenario. ";

class SatPerPacketRtnLinkUserTestCase : public SatPerPacketBaseTestCase
{
public:
  SatPerPacketRtnLinkUserTestCase ();
  SatPerPacketRtnLinkUserTestCase ( std::string name, SatEnums::FadingModel_t fading );
  SatPerPacketRtnLinkUserTestCase ( std::string name, SatEnums::FadingModel_t fading,
                                    StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2);

  virtual ~SatPerPacketRtnLinkUserTestCase ();

private:
  virtual void DoRun (void);
};

SatPerPacketRtnLinkUserTestCase::SatPerPacketRtnLinkUserTestCase ()
  : SatPerPacketBaseTestCase (defRtnUserName, "IfTestRtnUsr", SatEnums::FADING_OFF, false,
                              StringValue ("0.01s"), UintegerValue (512), StringValue ("0.01s"), UintegerValue (32) )
{

}

SatPerPacketRtnLinkUserTestCase::SatPerPacketRtnLinkUserTestCase (std::string name, SatEnums::FadingModel_t fading)
  : SatPerPacketBaseTestCase (defRtnUserName + name, "IfTestRtnUsr_" + name, fading, false,
                              StringValue ("0.01s"), UintegerValue (512), StringValue ("0.01s"), UintegerValue (32) )
{

}

SatPerPacketRtnLinkUserTestCase::SatPerPacketRtnLinkUserTestCase (std::string name, SatEnums::FadingModel_t fading,
                                                                  StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2)
  : SatPerPacketBaseTestCase (defRtnUserName + name, "IfTestRtnUsr_" + name, fading, false,
                              ival1, pSize1, ival2, pSize2 )
{
}

SatPerPacketRtnLinkUserTestCase::~SatPerPacketRtnLinkUserTestCase ()
{
}

void
SatPerPacketRtnLinkUserTestCase::DoRun (void)
{
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();

  InitOutput (false);

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-per-packet-if", m_extName, true);


  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (m_fading));
  Config::SetDefault ("ns3::SatFwdLinkScheduler::DummyFrameSendingEnabled", BooleanValue (m_dummyFrames));

  Config::SetDefault ("ns3::SatHelper::UtCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatHelper::UtUsers", UintegerValue (1));
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_0));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_AllocatedBandwidthHz", DoubleValue (1.25e6));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (1.25e6));
  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (13));
  Config::SetDefault ("ns3::SatGwHelper::DaRtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::DaRtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace",BooleanValue (true));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();

  // create user defined scenario with beams 1 and 5
  SatBeamUserInfo beamInfo = SatBeamUserInfo (1,1);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[1] = beamInfo;
  beamMap[5] = beamInfo;

  helper->CreateUserDefinedScenario (beamMap);

  // set callback traces where we want results out

  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();
  uint16_t port = 9;

  // create Sink helper
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));

  // install Sink application on GW user to receive packet
  ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
  gwSink.Start (Seconds (0.1));
  gwSink.Stop (Seconds (0.5));

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));

  // create applications on UT in beam 1
  cbrHelper.SetAttribute ("Interval", m_Interval1);
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize1 );

  ApplicationContainer ut1Cbr = cbrHelper.Install (utUsers.Get (0));
  ut1Cbr.Start (Seconds (0.1));
  ut1Cbr.Stop (Seconds (0.4));

  // create applications on UT in beam 5
  cbrHelper.SetAttribute ("Interval", m_Interval2);
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize2 );

  ApplicationContainer ut5Cbr = cbrHelper.Install (utUsers.Get (1));
  ut5Cbr.Start (Seconds (0.1));
  ut5Cbr.Stop (Seconds (0.4));

  Simulator::Stop (Seconds (1.0));
  Simulator::Run ();
  PrintTraceInfo ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

// Default name to show for test. When the second constructor is used, the text to append this name can be given.
static const char * defRtnFullName = "Test satellite per packet interference in Return Link with full defined scenario. ";

class SatPerPacketRtnLinkFullTestCase : public SatPerPacketBaseTestCase
{
public:
  SatPerPacketRtnLinkFullTestCase ();

  SatPerPacketRtnLinkFullTestCase ( std::string name, SatEnums::FadingModel_t fading);
  SatPerPacketRtnLinkFullTestCase ( std::string name, SatEnums::FadingModel_t fading,
                                    StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2);
  virtual ~SatPerPacketRtnLinkFullTestCase ();

private:
  virtual void DoRun (void);
};

SatPerPacketRtnLinkFullTestCase::SatPerPacketRtnLinkFullTestCase ()
  : SatPerPacketBaseTestCase (defRtnFullName, "IfTestRtnFull", SatEnums::FADING_OFF, false,
                              StringValue ("0.01s"), UintegerValue (512), StringValue ("0.01s"), UintegerValue (32) )
{

}

SatPerPacketRtnLinkFullTestCase::SatPerPacketRtnLinkFullTestCase (std::string name, SatEnums::FadingModel_t fading)
  : SatPerPacketBaseTestCase (defRtnFullName + name, "IfTestRtnFull_" + name, fading, false,
                              StringValue ("0.01s"), UintegerValue (512), StringValue ("0.01s"), UintegerValue (32) )
{

}


SatPerPacketRtnLinkFullTestCase::SatPerPacketRtnLinkFullTestCase (std::string name, SatEnums::FadingModel_t fading,
                                                                  StringValue ival1, UintegerValue pSize1, StringValue ival2, UintegerValue pSize2)
  : SatPerPacketBaseTestCase (defRtnFullName + name, "IfTestRtnFull_" + name, fading, false,
                              ival1, pSize1, ival2, pSize2 )
{

}


SatPerPacketRtnLinkFullTestCase::~SatPerPacketRtnLinkFullTestCase ()
{
}

void
SatPerPacketRtnLinkFullTestCase::DoRun (void)
{
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();

  InitOutput (false);

  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-per-packet-if", m_extName, true);

  Config::SetDefault ("ns3::SatBeamHelper::FadingModel", EnumValue (m_fading));
  Config::SetDefault ("ns3::SatFwdLinkScheduler::DummyFrameSendingEnabled", BooleanValue (m_dummyFrames));

  Config::SetDefault ("ns3::SatHelper::UtCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatHelper::UtUsers", UintegerValue (1));
  Config::SetDefault ("ns3::SatConf::SuperFrameConfForSeq0", EnumValue (SatSuperframeConf::SUPER_FRAME_CONFIG_0));
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameCount", UintegerValue (1));
  Config::SetDefault ("ns3::SatSuperframeConf0::FrameConfigType", EnumValue (SatSuperframeConf::CONFIG_TYPE_0));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_AllocatedBandwidthHz", DoubleValue (1.25e6));
  Config::SetDefault ("ns3::SatSuperframeConf0::Frame0_CarrierAllocatedBandwidthHz", DoubleValue (1.25e6));
  Config::SetDefault ("ns3::SatWaveformConf::DefaultWfId", UintegerValue (13));
  Config::SetDefault ("ns3::SatGwHelper::DaRtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatGeoHelper::DaRtnLinkInterferenceModel",EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET));
  Config::SetDefault ("ns3::SatPhyRxCarrierConf::EnableIntfOutputTrace",BooleanValue (true));

  // Creating the reference system.
  Ptr<SatHelper> helper = CreateObject<SatHelper> ();
  helper->CreatePredefinedScenario (SatHelper::FULL);

  // set callback traces where we want results out

  Config::Connect ("/NodeList/*/DeviceList/*/SatPhy/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/UserPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  Config::Connect ("/NodeList/*/DeviceList/*/FeederPhy/*/PhyRx/RxCarrierList/*/LinkBudgetTrace",
                   MakeCallback (&LinkBudgetTraceCb));

  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();
  uint16_t port = 9;

  // create Sink helper
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));

  // install Sink application on GW user to receive packet
  ApplicationContainer gwSink = sinkHelper.Install (gwUsers.Get (0));
  gwSink.Start (Seconds (0.1));
  gwSink.Stop (Seconds (0.5));

  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port));

  // create applications on UT in beam 1
  cbrHelper.SetAttribute ("Interval", m_Interval1);
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize1 );

  ApplicationContainer ut1Cbr = cbrHelper.Install (utUsers.Get (0));
  ut1Cbr.Start (Seconds (0.1));
  ut1Cbr.Stop (Seconds (0.4));

  // create applications on UT in beam 5
  cbrHelper.SetAttribute ("Interval", m_Interval2);
  cbrHelper.SetAttribute ("PacketSize", m_PackageSize2 );

  ApplicationContainer ut5Cbr = cbrHelper.Install (utUsers.Get (4));
  ut5Cbr.Start (Seconds (0.1));
  ut5Cbr.Stop (Seconds (0.4));

  Simulator::Stop (Seconds (1.0));
  Simulator::Run ();
  PrintTraceInfo ();

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \ingroup satellite
 * \brief Test suite for Satellite interference unit test cases.
 */
class SatPerPacketIfTestSuite : public TestSuite
{
public:
  SatPerPacketIfTestSuite ();
};

SatPerPacketIfTestSuite::SatPerPacketIfTestSuite ()
  : TestSuite ("sat-per-packet-if-test", SYSTEM)
{
  AddTestCase (new SatPerPacketFwdLinkUserTestCase, TestCase::QUICK);
  AddTestCase (new SatPerPacketFwdLinkUserTestCase ("DummyFrames.",
                                                    SatEnums::FADING_OFF,
                                                    true),
               TestCase::QUICK);
  AddTestCase (new SatPerPacketFwdLinkUserTestCase ("Markov_Fading.",
                                                    SatEnums::FADING_MARKOV,
                                                    false),
               TestCase::QUICK);
  AddTestCase (new SatPerPacketFwdLinkUserTestCase ("Markov_Fading, DummyFrames.",
                                                    SatEnums::FADING_MARKOV,
                                                    true),
               TestCase::QUICK);
  AddTestCase (new SatPerPacketFwdLinkFullTestCase, TestCase::QUICK);

  AddTestCase (new SatPerPacketFwdLinkFullTestCase ("Markov_Fading.",
                                                    SatEnums::FADING_MARKOV,
                                                    false),
               TestCase::QUICK);
  AddTestCase (new SatPerPacketRtnLinkUserTestCase, TestCase::QUICK);
  AddTestCase (new SatPerPacketRtnLinkUserTestCase ("Markov_Fading.",
                                                    SatEnums::FADING_MARKOV),
               TestCase::QUICK);

  AddTestCase (new SatPerPacketRtnLinkFullTestCase, TestCase::QUICK);
  AddTestCase (new SatPerPacketRtnLinkFullTestCase ("Markov_Fading.",
                                                    SatEnums::FADING_MARKOV),
               TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatPerPacketIfTestSuite satSatPerPacketIfTestSuite;

