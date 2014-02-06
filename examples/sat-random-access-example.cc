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
* \brief Example for Random Access module.
*
*/

NS_LOG_COMPONENT_DEFINE ("sat-random-access-example");

int
main (int argc, char *argv[])
{
  /// Enable info logs
  LogComponentEnable ("sat-random-access-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatRandomAccess", LOG_LEVEL_INFO);

  /// Load default random access configuration
  Ptr<SatRandomAccessConf> randomAccessConf = CreateObject<SatRandomAccessConf> ();

  /// Create random access module with RA_SLOTTED_ALOHA as default
  Ptr<SatRandomAccess> randomAccess = CreateObject<SatRandomAccess> (randomAccessConf, SatRandomAccess::RA_SLOTTED_ALOHA);

  /// Run simulation
  for (uint32_t i = 0; i < 5; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatRandomAccess::DoRandomAccess, randomAccess);
    }

  /// Change random access model to RA_CRDSA
  Simulator::Schedule (Time (300000 + 6*500000), &SatRandomAccess::SetRandomAccessModel, randomAccess, SatRandomAccess::RA_CRDSA);

  /// Continue simulation
  for (uint32_t i = 7; i < 13; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatRandomAccess::DoRandomAccess, randomAccess);
    }

  /// Change random access model to RA_ANY_AVAILABLE
  Simulator::Schedule (Time (300000 + 14*500000), &SatRandomAccess::SetRandomAccessModel, randomAccess, SatRandomAccess::RA_ANY_AVAILABLE);

  /// Continue simulation
  for (uint32_t i = 15; i < 21; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatRandomAccess::DoRandomAccess, randomAccess);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
