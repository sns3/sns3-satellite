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
* \brief
*
*/

NS_LOG_COMPONENT_DEFINE ("fading-trace-example");

static void FadingTraceCb ( std::string context, double time, SatChannel::ChannelType_t chType, double fadingValue)
{
  std::cout << time << " " << chType << " " << 20 * log10(fadingValue) << std::endl;
}

int
main (int argc, char *argv[])
{

  /// create default Markov & Loo configurations
  Ptr<SatMarkovConf> markovConf = CreateObject<SatMarkovConf>();
  Ptr<SatLooConf> looConf = CreateObject<SatLooConf>();

  /// set position
  GeoCoordinate currentPosition = GeoCoordinate(20, -20, 35000000);

  /// create fading container based on default configuration
  Ptr<SatFadingContainer> markovContainer = CreateObject<SatFadingContainer>(markovConf,looConf,currentPosition);

  markovContainer->TraceConnect("FadingTrace","The trace for fading values",MakeCallback (&FadingTraceCb));

  /// run simulation
  for (uint32_t i = 0; i < 100000; i++)
    {
      Simulator::Schedule( MilliSeconds(1 * i), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
    }

  Simulator::Schedule( MilliSeconds(0), &SatFadingContainer::LockToSetAndState, markovContainer, 0,0);
  Simulator::Schedule( MilliSeconds(20000), &SatFadingContainer::LockToSetAndState, markovContainer, 0,1);
  Simulator::Schedule( MilliSeconds(40000), &SatFadingContainer::LockToSetAndState, markovContainer, 0,2);
  Simulator::Schedule( MilliSeconds(60000), &SatFadingContainer::LockToSetAndState, markovContainer, 0,0);
  Simulator::Schedule( MilliSeconds(80000), &SatFadingContainer::LockToSetAndState, markovContainer, 0,1);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
