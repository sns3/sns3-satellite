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

NS_LOG_COMPONENT_DEFINE ("markov-example");

int
main (int argc, char *argv[])
{
  // enable info logs
  LogComponentEnable ("markov-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatFadingContainer", LOG_LEVEL_INFO);
  LogComponentEnable ("SatMarkovModel", LOG_LEVEL_INFO);
  LogComponentEnable ("SatMarkovConf", LOG_LEVEL_INFO);

  // create default Markov configuration
  Ptr<SatMarkovConf> markovConf = CreateObject<SatMarkovConf>();
  Ptr<SatLooConf> looConf = CreateObject<SatLooConf>();

  GeoCoordinate currentPosition = GeoCoordinate(20, -20, 35000000);

  // create fading container based on default configuration
  Ptr<SatFadingContainer> markovContainer = CreateObject<SatFadingContainer>(markovConf,looConf,currentPosition);

  // Run Markov chain

  GeoCoordinate position1 = GeoCoordinate(50, -50, 35000000);
  GeoCoordinate position2 = GeoCoordinate(10, -10, 35000000);
  GeoCoordinate position3 = GeoCoordinate(30, -30, 35000000);

  Simulator::Schedule(Time("5ms"), &SatFadingContainer::SetPosition, markovContainer, position1);
  Simulator::Schedule(Time("10ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
  Simulator::Schedule(Time("30ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
  Simulator::Schedule(Time("45ms"), &SatFadingContainer::SetElevation, markovContainer, 70);
  Simulator::Schedule(Time("50ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
  Simulator::Schedule(Time("60ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
  Simulator::Schedule(Time("65ms"), &SatFadingContainer::SetPosition, markovContainer, position2);
  Simulator::Schedule(Time("65ms"), &SatFadingContainer::SetElevation, markovContainer, 10);
  Simulator::Schedule(Time("90ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
  Simulator::Schedule(Time("100ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
  Simulator::Schedule(Time("130ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);
  Simulator::Schedule(Time("140ms"), &SatFadingContainer::SetPosition, markovContainer, position3);
  Simulator::Schedule(Time("200ms"), &SatFadingContainer::GetFading, markovContainer, SatChannel::FORWARD_USER_CH);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
