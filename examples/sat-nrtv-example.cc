/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/satellite-module.h>
#include <ns3/traffic-module.h>
#include <iomanip>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("SatNrtvExample");


class NrtvCalculator
{
public:
  NrtvCalculator (ApplicationContainer apps, Time duration);
  void Print ();
private:
  void RxCallback (std::string context, Ptr<const Packet> packet);
  static std::string GetContext (uint32_t appId);
  static uint32_t GetAppId (std::string context);
  ApplicationContainer m_apps;
  Time m_duration;
  std::vector<uint64_t> m_rxBytes;
};

NrtvCalculator::NrtvCalculator (ApplicationContainer apps, Time duration)
  : m_apps (apps),
    m_duration (duration)
{
  uint32_t n = apps.GetN ();
  m_rxBytes.resize (n, 0);

  uint32_t appId = 0;

  for (ApplicationContainer::Iterator it = apps.Begin ();
       it != apps.End (); it++)
    {
      NS_ASSERT ((*it)->GetObject<NrtvClient> () != 0);
      NS_ASSERT (appId < n);
      std::string context = GetContext (appId);
      (*it)->TraceConnect ("Rx", context,
                           MakeCallback (&NrtvCalculator::RxCallback, this));
      appId++;
    }
}

void
NrtvCalculator::Print ()
{
  uint64_t sumRxBytes = 0;
  double throughput = 0.0;
  uint32_t sumRxPackets = 0;
  Time sumRxDelay = MilliSeconds (0);
  Time delay = MilliSeconds (0);
  double duration = m_duration.GetSeconds ();
  uint32_t n = m_rxBytes.size ();

  NS_LOG_INFO (this << " NRTV clients round-up statistics:");
  NS_LOG_INFO (this << " -----------------------------------------");
  NS_LOG_INFO (this << std::setw (5) << "#"
                    << std::setw (12) << "bytes"
                    << std::setw (12) << "kbps"
                    << std::setw (12) << "delay");
  NS_LOG_INFO (this << " -----------------------------------------");

  for (uint32_t i = 0; i < n; i++)
    {
      throughput = static_cast<double> (m_rxBytes[i] * 8) / 1000.0 / duration;
      Ptr<NrtvClient> app = m_apps.Get (i)->GetObject<NrtvClient> ();
      delay = app->GetDelayAverage ();
      NS_LOG_INFO (this << std::setw (5) << i
                        << std::setw (12) << m_rxBytes[i]
                        << std::setw (12) << throughput
                        << std::setw (12) << delay.GetSeconds ());
      sumRxBytes += m_rxBytes[i];
      sumRxPackets += app->GetNumOfRxSlices ();
      sumRxDelay += app->GetDelaySum ();
    }

  throughput = static_cast<double> (sumRxBytes * 8) / 1000.0 / duration;
  delay = Seconds (sumRxDelay.GetSeconds () / static_cast<double> (sumRxPackets));
  NS_LOG_INFO (this << " -----------------------------------------");
  NS_LOG_INFO (this << std::setw (5) << "sum"
                    << std::setw (12) << sumRxBytes
                    << std::setw (12) << throughput);
  NS_LOG_INFO (this << std::setw (5) << "avg"
                    << std::setw (12) << static_cast<double> (sumRxBytes) / n
                    << std::setw (12) << static_cast<double> (throughput) / n
                    << std::setw (12) << delay.GetSeconds () / n);
  NS_LOG_INFO (this << " -----------------------------------------");

  NS_UNUSED (throughput);
  NS_UNUSED (delay);
}

void
NrtvCalculator::RxCallback (std::string context, Ptr<const Packet> packet)
{
  uint32_t appId = GetAppId (context);
  NS_ASSERT (appId < m_rxBytes.size ());
  m_rxBytes[appId] += packet->GetSize ();
}

std::string
NrtvCalculator::GetContext (uint32_t appId)
{
  std::ostringstream context;
  context << appId;
  return context.str ();
}

uint32_t
NrtvCalculator::GetAppId (std::string context)
{
  std::stringstream ss (context);
  uint32_t appId;
  if (!(ss >> appId))
    {
      NS_FATAL_ERROR ("Cannot convert context '" << context << "' to number");
    }
  return appId;
}


/**
 * \ingroup satellite
 *
 * \brief Example of using NRTV traffic model in a satellite network.
 *
 * One NRTV server application is installed in the first GW user. One NRTV
 * client application is installed in each UT user, configured to point to the
 * server.
 *
 * By default, the SIMPLE test scenario is used. Another test scenario can be
 * given from command line as user argument, e.g.:
 *
 *     $ ./waf --run="sat-nrtv-example --scenario=larger"
 *     $ ./waf --run="sat-nrtv-example --scenario=full"
 *
 * Simulation runs for 1000 seconds by default. This can be changed from the
 * command line argument as well, e.g.:
 *
 *     $ ./waf --run="sat-nrtv-example --duration=500"
 *
 * To see help for user arguments:
 *
 *     $ ./waf --run "sat-nrtv-example --PrintHelp"
 *
 */
int
main (int argc, char *argv[])
{
  // a workaround to solve weird splitting in lower layer
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType",
                      StringValue ("ns3::TcpRfc793"));

  std::string scenario = "simple";
  double duration = 1000;
  std::string scenarioLogFile = "";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("scenario", "Test scenario to use. (simple, larger or full)",
                scenario);
  cmd.AddValue ("duration", "Simulation duration (in seconds)",
                duration);
  cmd.AddValue ("logFile", "File name for scenario creation log",
                scenarioLogFile);
  cmd.Parse (argc, argv);

  if (scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if (scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }

  //LogComponentEnableAll (LOG_PREFIX_ALL);
  //LogComponentEnable ("NrtvClient", LOG_LEVEL_ALL);
  //LogComponentEnable ("NrtvServer", LOG_LEVEL_ALL);
  LogComponentEnable ("SatNrtvExample", LOG_LEVEL_INFO);

  // remove next line from comments to run real time simulation
  // GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  // create satellite helper with given scenario default=simple

  // Create reference system, two options:
  // - "Scenario72"
  // - "Scenario98"
  std::string scenarioName = "Scenario72";
  // std::string scenarioName = "Scenario98";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  if (scenarioLogFile != "")
    {
      helper->EnableCreationTraces (scenarioLogFile, false);
    }

  helper->CreateScenario (satScenario);

  // get users
  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();

  NrtvHelper nrtvHelper ("ns3::TcpSocketFactory");
  nrtvHelper.InstallUsingIpv4 (gwUsers.Get (0), utUsers);
  nrtvHelper.GetServer ().Start (Seconds (1.0));
  nrtvHelper.GetClients ().Start (Seconds (3.0));

  // install calculator
  NrtvCalculator nrtvCalculator (nrtvHelper.GetClients (), Seconds (duration));

  NS_LOG_INFO ("--- sat-nrtv-example ---");
  NS_LOG_INFO ("  Scenario used: " << scenario);
  NS_LOG_INFO ("  Creation logFile: " << scenarioLogFile);
  NS_LOG_INFO ("  ");

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();

  nrtvCalculator.Print ();

  Simulator::Destroy ();

  return 0;
}
