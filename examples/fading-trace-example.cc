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

static void FadingTraceCb ( std::string context, double time, SatEnums::ChannelType_t chType, double fadingValue)
{
  std::cout << time << " " << chType << " " << 20 * log10(fadingValue) << std::endl;
}

static double g_elevation = 45;
static double g_velocity = 0;

static double GetElevation ()
{
   return g_elevation;
}

static double GetVelocity ()
{
   return g_velocity;
}

int
main (int argc, char *argv[])
{

  /// create default Markov & Loo configurations
  Ptr<SatMarkovConf> markovConf = CreateObject<SatMarkovConf>();

  SatBaseFading::ElevationCallback elevationCb = MakeCallback (&GetElevation);
  SatBaseFading::VelocityCallback velocityCb = MakeCallback (&GetVelocity);

  /// create fading container based on default configuration
  Ptr<SatMarkovContainer> markovContainer = CreateObject<SatMarkovContainer>(markovConf,elevationCb,velocityCb);

  markovContainer->TraceConnect("FadingTrace","The trace for fading values",MakeCallback (&FadingTraceCb));

  /// run simulation
  for (uint32_t i = 0; i < 100000; i++)
    {
      Simulator::Schedule( MilliSeconds(1 * i), &SatMarkovContainer::DoGetFading, markovContainer, SatEnums::FORWARD_USER_CH);
    }

  Simulator::Schedule( MilliSeconds(0), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,0);
  Simulator::Schedule( MilliSeconds(20000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,1);
  Simulator::Schedule( MilliSeconds(40000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,2);
  Simulator::Schedule( MilliSeconds(60000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,0);
  Simulator::Schedule( MilliSeconds(80000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,1);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
