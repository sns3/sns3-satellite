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
 */


#include <iostream>

#include <ns3/core-module.h>
#include <ns3/traffic-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/satellite-module.h>
#include <ns3/config-store-module.h>


using namespace ns3;


/**
 * \file sat-mobility-position-generator.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results with
 * a high degree of customization through XML file.
 *
 * execute command -> ./waf --run "sat-mobility-position-generator --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-mobility-position-generator");


int
main (int argc, char *argv[])
{
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/generic-input-attributes.xml";
  uint32_t posCount = 1;

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("sat-mobility-position-generator");
  simulationHelper->DisableAllCapacityAssignmentCategories ();
  simulationHelper->EnableCrdsa ();

  // Parse command-line and XML file
  CommandLine cmd;
  cmd.AddValue ("PosCount", "Amount of positions to generate per beam", posCount);
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);
  simulationHelper->ConfigureAttributesFromFile (inputFileNameWithPath);

  Ptr<SatHelper> satHelper = simulationHelper->GetSatelliteHelper ();
  for (uint32_t beamId : simulationHelper->GetBeamSet ())
    {
      for (uint32_t posId = 0; posId < posCount; ++posId)
        {
          Ptr<SatSpotBeamPositionAllocator> positions = satHelper->GetBeamAllocator (beamId);
          GeoCoordinate coords = positions->GetNextGeoPosition (0);
          std::cout << "[" << beamId << "] " << coords << std::endl;
        }
    }
}
