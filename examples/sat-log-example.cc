/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

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
 * The output files are located in src/satellite/data/logs folder. The output file
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

  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_GENERIC, "", "Logging for generic messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_INFO, "", "Logging for info messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", "Logging for warning messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_ERROR, "", "Logging for error messages started");
  Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_CUSTOM, "_customTag", "Logging for custom messages started");

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
