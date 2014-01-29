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
* \brief Example for CRDSA random access module.
*
*/

NS_LOG_COMPONENT_DEFINE ("sat-crdsa-example");

int
main (int argc, char *argv[])
{
  /// enable info logs
  LogComponentEnable ("sat-crdsa-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatCrdsa", LOG_LEVEL_INFO);

  /// load default random access configuration
  Ptr<SatRandomAccessConf> randomAccessConf = CreateObject<SatRandomAccessConf> ();

  /// create CRDSA module
  Ptr<SatCrdsa> crdsa = CreateObject<SatCrdsa> (randomAccessConf);

  /// run simulation
  for (uint32_t i = 0; i < 5; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatCrdsa::DoCrdsa, crdsa);
    }

  Simulator::Schedule (Time (300000 + 6*500000), &SatCrdsa::UpdateVariables, crdsa, 1, 7, 2);

  for (uint32_t i = 7; i < 12; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatCrdsa::DoCrdsa, crdsa);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
