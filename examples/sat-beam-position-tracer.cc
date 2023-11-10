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

using namespace ns3;

/**
 * \file sat-beam-position-tracer.cc
 * \ingroup satellite
 *
 * \brief Script used to get center of each beam at simulation start.
 *
 * execute command -> ./ns3 run "sat-beam-position-tracer --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE("sat-beam-position-tracer");

int
main(int argc, char* argv[])
{
    std::string scenario = "constellation-eutelsat-geo-2-sats-no-isls";

    Config::SetDefault("ns3::SatConf::ForwardLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));
    Config::SetDefault("ns3::SatConf::ReturnLinkRegenerationMode",
                       EnumValue(SatEnums::REGENERATION_NETWORK));

    Ptr<SimulationHelper> simulationHelper =
        CreateObject<SimulationHelper>("sat-beam-position-tracer");

    CommandLine cmd;
    cmd.AddValue("Scenario", "Scenario to load", scenario);
    simulationHelper->AddDefaultUiArguments(cmd);
    cmd.Parse(argc, argv);

    simulationHelper->LoadScenario(scenario);

    Ptr<SatHelper> satHelper = simulationHelper->CreateSatScenario(SatHelper::FULL);
    Ptr<SatAntennaGainPatternContainer> antennaGainPatterns = satHelper->GetAntennaGainPatterns();

    NodeContainer satNodes = satHelper->GeoSatNodes();
    uint32_t beamCount = satHelper->GetBeamCount();
    for (uint32_t satId = 0; satId < satNodes.GetN(); satId++)
    {
        Ptr<SatMobilityModel> mobility = antennaGainPatterns->GetAntennaMobility(satId);
        for (uint32_t beamId = 0; beamId < beamCount; beamId++)
        {
            Ptr<SatAntennaGainPattern> pattern =
                antennaGainPatterns->GetAntennaGainPattern(beamId + 1);
            std::cout << "Center of beam " << beamId << " for satellite " << satId << " is "
                      << pattern->GetCenterLatitude(mobility) << ";"
                      << pattern->GetCenterLongitude(mobility) << std::endl;
        }
    }
}
