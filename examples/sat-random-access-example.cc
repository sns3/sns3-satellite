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

  uint32_t allocationChannel = 0;

  /// Load default lower layer service configuration
  Ptr<SatLowerLayerServiceConf> llsConf = CreateObject<SatLowerLayerServiceConf>  ();

  /// Load default random access configuration
  Ptr<SatRandomAccessConf> randomAccessConf = CreateObject<SatRandomAccessConf> (llsConf);

  /// Create random access module with RA_CRDSA as default
  Ptr<SatRandomAccess> randomAccess = CreateObject<SatRandomAccess> (randomAccessConf, SatRandomAccess::RA_CRDSA);

  /// Run simulation
  for (uint32_t i = 0; i < 49; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatRandomAccess::DoRandomAccess, randomAccess, allocationChannel);
    }

  /// Change random access model to RA_SLOTTED_ALOHA
  Simulator::Schedule (Time (300001 + 49*500000), &SatRandomAccess::SetRandomAccessModel, randomAccess, SatRandomAccess::RA_SLOTTED_ALOHA);

  /// Continue simulation
  for (uint32_t i = 50; i < 99; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatRandomAccess::DoRandomAccess, randomAccess, allocationChannel);
    }

  /// Change random access model to RA_ANY_AVAILABLE
  Simulator::Schedule (Time (300001 + 99*500000), &SatRandomAccess::SetRandomAccessModel, randomAccess, SatRandomAccess::RA_ANY_AVAILABLE);

  /// Continue simulation
  for (uint32_t i = 100; i < 149; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatRandomAccess::DoRandomAccess, randomAccess, allocationChannel);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
