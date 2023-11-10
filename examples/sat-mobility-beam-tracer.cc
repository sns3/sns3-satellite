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

#include <ns3/config-store-module.h>
#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/network-module.h>
#include <ns3/satellite-module.h>
#include <ns3/traffic-module.h>

#include <iostream>

using namespace ns3;

Ptr<SatMobilityModel> satMobility = nullptr;
std::set<std::pair<uint32_t, uint32_t>> visitedBeams;
std::map<uint32_t, std::vector<double>> relativeSpeeds;

static void
SatCourseChange(std::string context, Ptr<const SatMobilityModel> position)
{
    auto tracedPosition = DynamicCast<const SatTracedMobilityModel>(position);
    NS_ASSERT_MSG(tracedPosition != NULL, "Course changed for a non-mobile UT");

    uint32_t sat = tracedPosition->GetSatId();
    uint32_t beam = tracedPosition->GetBestBeamId(true);
    visitedBeams.insert(std::make_pair(sat, beam));
    relativeSpeeds[sat].push_back(tracedPosition->GetRelativeSpeed(satMobility));
}

/**
 * \file sat-mobility-beam-tracer.cc
 * \ingroup satellite
 *
 * \brief Simulation script to run example simulation results with
 * a high degree of customization through XML file.
 *
 * execute command -> ./ns3 run "sat-mobility-beam-tracer --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE("sat-mobility-beam-tracer");

int
main(int argc, char* argv[])
{
    std::string inputFileNameWithPath =
        Singleton<SatEnvVariables>::Get()->LocateDirectory("contrib/satellite/examples") +
        "/generic-input-attributes.xml";
    std::string mobileUtTraceFile(Singleton<SatEnvVariables>::Get()->LocateDataDirectory() +
                                  "/additional-input/utpositions/mobiles/scenario6/trajectory");

    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("sat-mobility-position-generator");
    simulationHelper->EnableProgressLogs();
    simulationHelper->DisableAllCapacityAssignmentCategories();
    simulationHelper->EnableCrdsa();

    // Parse command-line and XML file
    CommandLine cmd;
    cmd.AddValue("TraceFile", "Path to the trace file to check beams from", mobileUtTraceFile);
    simulationHelper->AddDefaultUiArguments(cmd, inputFileNameWithPath);
    cmd.Parse(argc, argv);
    simulationHelper->ReadInputAttributesFromFile(inputFileNameWithPath);
    simulationHelper->SetSimulationTime(Seconds(100));

    std::set<uint32_t> beamSetAll;
    for (uint32_t i = 1; i < 73; i++)
    {
        beamSetAll.insert(i);
    }
    simulationHelper->SetBeamSet(beamSetAll);
    simulationHelper->SetUserCountPerUt(1);

    simulationHelper->LoadScenario("constellation-eutelsat-geo-2-sats-no-isls");

    if (mobileUtTraceFile != "")
    {
        Ptr<SatHelper> satHelper =
            simulationHelper->CreateSatScenario(SatHelper::NONE, mobileUtTraceFile);
        satMobility =
            satHelper->GetBeamHelper()->GetGeoSatNodes().Get(0)->GetObject<SatMobilityModel>();

        uint32_t satNb = satHelper->GeoSatNodes().GetN();
        for (uint32_t i = 0; i < satNb; i++)
        {
            Ptr<Node> node = satHelper->LoadMobileUtFromFile(mobileUtTraceFile);
            node->GetObject<SatMobilityModel>()->TraceConnect("SatCourseChange",
                                                              "BeamTracer",
                                                              MakeCallback(SatCourseChange));
        }

        simulationHelper->RunSimulation();
        Simulator::Destroy();

        std::cout << "Visited beams are:";
        for (std::pair<uint32_t, uint32_t> satAndBeam : visitedBeams)
        {
            std::cout << " (" << satAndBeam.first << ", " << satAndBeam.second << ")";
        }
        std::cout << std::endl;

        for (uint32_t satId = 0; satId < satNb; satId++)
        {
            double minSpeed = 0.0, maxSpeed = 0.0, totalSpeed = 0.0;
            uint32_t valuesCount = 0;
            for (double& speed : relativeSpeeds[satId])
            {
                if (!minSpeed)
                {
                    minSpeed = speed;
                }
                if (speed)
                {
                    minSpeed = std::min(minSpeed, speed);
                    maxSpeed = std::max(maxSpeed, speed);
                    totalSpeed += speed;
                    ++valuesCount;
                }
            }
            std::cout << "Speeding stats (m/s) for  satellite " << satId << ":\n\tmin: " << minSpeed
                      << "\n\tmax: " << maxSpeed << "\n\tmean: " << totalSpeed / valuesCount
                      << std::endl;
        }
    }
}
