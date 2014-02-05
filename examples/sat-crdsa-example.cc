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
  /// Enable info logs
  LogComponentEnable ("sat-crdsa-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatCrdsa", LOG_LEVEL_INFO);

  /// Load default random access configuration
  Ptr<SatRandomAccessConf> randomAccessConf = CreateObject<SatRandomAccessConf> ();

  /// Create CRDSA module with default configuration
  Ptr<SatCrdsa> crdsa = CreateObject<SatCrdsa> (randomAccessConf);

  /// Run simulation
  for (uint32_t i = 0; i < 30; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatCrdsa::DoCrdsa, crdsa);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
