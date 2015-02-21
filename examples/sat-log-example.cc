/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 *
 */

#include "ns3/core-module.h"
#include "ns3/satellite-module.h"


using namespace ns3;

/**
 * \file sat-log-example.cc
 * \ingroup satellite
 *
 * \brief Example for message log class.
 *
 * The format for messages is (type, custom file tag, message). Custom file tag
 * is in effect only with LOG_CUSTOM. With other types it does not matter and can
 * be left empty. It is also possible to define a simulation specific tag, which
 * is useful with simulation campaigns for avoiding log file overwrite.
 *
 * The class specifies the following log types:
 * LOG_GENERIC - for all messages
 * LOG_INFO - for info messages
 * LOG_WARNING - for warning messages
 * LOG_ERROR - for error messages
 * LOG_CUSTOM - for custom messages specified by the second parameter
 *
 * The output files are located in contrib/satellite/data/logs folder. The output file
 * format is the following: log<type><custom file tag><simulation tag>
 *
 * With (LOG_CUSTOM, "_exampleTag", "Example message for custom log") and simulation tag
 * "_ut30_beam1" the file log_exampleTag_ut30_beam1 would contain the message
 * "Example message for custom log".
 */

NS_LOG_COMPONENT_DEFINE ("sat-log-example");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("sat-log-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatLog", LOG_LEVEL_INFO);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::SimulationCampaignName", StringValue ("example-log"));
  Config::SetDefault ("ns3::SatEnvVariables::SimulationTag", StringValue (""));
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_GENERIC, "", "Logging for generic messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_INFO, "", "Logging for info messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", "Logging for warning messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_ERROR, "", "Logging for error messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_CUSTOM, "_customTag", "Logging for custom messages started");

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
