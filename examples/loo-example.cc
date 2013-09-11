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

NS_LOG_COMPONENT_DEFINE ("loo-example");

int
main (int argc, char *argv[])
{
  // enable info logs
  LogComponentEnable ("loo-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatLooModel", LOG_LEVEL_INFO);

  Ptr<SatLooModel> looFaderLineOfSight = CreateObject<SatLooModel> (0.5,0.5,-8.0,10,1);
  Ptr<SatLooModel> looFaderLightShadowing = CreateObject<SatLooModel> (0.5,0.5,-8.0,10,1);
  Ptr<SatLooModel> looFaderHeavyShadowing = CreateObject<SatLooModel> (-17.0,3.5,-12.0,10,1);

  for (uint32_t i = 0; i < 1000; i++)
    {
      Simulator::Schedule(Time(300000 + i*500000), &SatLooModel::GetChannelGainDb, looFaderLineOfSight);
      Simulator::Schedule(Time(500000 + i*500000), &SatLooModel::GetChannelGainDb, looFaderLightShadowing);
      Simulator::Schedule(Time(700000 + i*500000), &SatLooModel::GetChannelGainDb, looFaderHeavyShadowing);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
