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
* \brief Example for Slotted ALOHA random access module.
*
*/

NS_LOG_COMPONENT_DEFINE ("sat-slotted-aloha-example");

int
main (int argc, char *argv[])
{
  /// Enable info logs
  LogComponentEnable ("sat-slotted-aloha-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatSlottedAloha", LOG_LEVEL_INFO);

  /// Load default random access configuration
  Ptr<SatRandomAccessConf> randomAccessConf = CreateObject<SatRandomAccessConf> ();

  /// Create Slotted ALOHA module
  Ptr<SatSlottedAloha> slottedAloha = CreateObject<SatSlottedAloha> (randomAccessConf);

  /// Run simulation
  for (uint32_t i = 0; i < 5; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatSlottedAloha::DoSlottedAloha, slottedAloha);
    }

  /// Update Slotted ALOHA variables
  Simulator::Schedule (Time (300000 + 6*500000), &SatSlottedAloha::UpdateVariables, slottedAloha, 4.5, 5.5);


  /// Continue simulation
  for (uint32_t i = 7; i < 12; i++)
    {
      Simulator::Schedule (Time (300000 + i*500000), &SatSlottedAloha::DoSlottedAloha, slottedAloha);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
