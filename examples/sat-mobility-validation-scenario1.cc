/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 *
 */


#include "ns3/core-module.h"
#include "ns3/traffic-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/config-store-module.h"


using namespace ns3;


/**
 * \file sat-mobility-validation-scenario1.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results with
 * a high degree of customization through XML file.
 *
 * execute command -> ./waf --run "sat-generic-launcher --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-mobility-scenario1");


int
main (int argc, char *argv[])
{
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/generic-input-attributes.xml";

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-mobility-scenario1");
  simulationHelper->DisableAllCapacityAssignmentCategories ();
  simulationHelper->EnableCrdsa ();

  CommandLine cmd;
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  // Manual setup
  simulationHelper->SetBeamSet ({12, 15});
  simulationHelper->SetUtCountPerBeam (1);
  simulationHelper->SetUserCountPerUt (1);
  simulationHelper->SetSimulationTime (Seconds (4.0));

  // Read XML file but don't override manual setup
  simulationHelper->ConfigureAttributesFromFile (inputFileNameWithPath, false);
  simulationHelper->StoreAttributesToFile ("parametersUsed.xml");

  // Switch UT from beam 12 to beam 15 after 2 seconds
  Ptr<Node> ut = simulationHelper->GetSatelliteHelper ()->GetBeamHelper ()->GetUtNodes (12).Get (0);
  for (uint32_t i = 0; i < ut->GetNDevices (); ++i)
    {
      Ptr<SatNetDevice> dev = DynamicCast<SatNetDevice> (ut->GetDevice (i));
      if (dev != NULL)
        {
          Ptr<SatRequestManager> rm = DynamicCast<SatUtLlc> (dev->GetLlc ())->GetRequestManager ();
          Simulator::Schedule (Seconds (2.0), &SatRequestManager::SendHandoverRecommendation, rm, 15);
        }
    }

  simulationHelper->RunSimulation ();
}
