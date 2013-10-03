/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/satellite-channel.h"

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
  std::cout << time << " " << chType << " " << fadingValue << std::endl;
}

int
main (int argc, char *argv[])
{

  // create default Markov configuration
  Ptr<SatMarkovConf> markovConf = CreateObject<SatMarkovConf>();

  GeoCoordinate currentPosition = GeoCoordinate(20, -20, 35000000);

  // create fading container based on default configuration
  Ptr<SatFadingContainer> markovContainer = CreateObject<SatFadingContainer>(markovConf,currentPosition);

  markovContainer->TraceConnect("FadingTrace","The trace for fading values",MakeCallback (&FadingTraceCb));

  // Run Markov chain
  for (uint32_t i = 0; i < 100000; i++)
    {
      Simulator::Schedule( MilliSeconds(1 * i), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
