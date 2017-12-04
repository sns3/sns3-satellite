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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file sat-multicast-example.cc
 * \ingroup satellite
 *
 * \brief  Multicast example application to test multicasting in satellite network.
 *         Test scenario (larger of full), pre-defined multicast for larger scenario
 *         can be given in command line as user argument.
 *
 *         To see help and more info for user arguments:
 *         execute command -> ./waf --run "sat-multicast-example --PrintHelp"
 */

NS_LOG_COMPONENT_DEFINE ("sat-multicast-example");

/**
 * Receive RX traces from packet sinks
 *
 * \param context Context of the receive (multicast group and UT/GW user info)
 */
static void SinkReceive (std::string context, Ptr<const Packet>, const Address &)
{
  NS_LOG_INFO (" Packet received from/by: " << context);
}

/**
 * Construct information of the given user node.
 *
 * \param helper Pointer to satellite helper
 * \param node Pointer to user node connected to UT or GW
 * \return Information of the user (UT or GW node connected user with user index in the node)
 */
static std::string GetUserInfo (Ptr<SatHelper> helper, Ptr<Node> node)
{
  std::stringstream ss;   //create a string stream
  Ptr<Node> utNode = helper->GetUserHelper ()->GetUtNode (node);
  NodeContainer nodeUsers;

  if ( utNode )
    {
      nodeUsers = helper->GetUserHelper ()->GetUtUsers (utNode);
      ss << "UT" << Singleton<SatIdMapper>::Get ()->GetUtIdWithMac (Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (utNode)) << "-user-";
    }
  else
    {
      nodeUsers = helper->GetUserHelper ()->GetGwUsers ();
      ss << "GW-user-";
    }

  uint32_t userIndex = 0;

  for ( uint32_t j = 0; ((j < nodeUsers.GetN ()) && (userIndex == 0)); j++)
    {
      if ( nodeUsers.Get (j) == node )
        {
          userIndex = j + 1;
        }
    }

  ss << userIndex;

  return ss.str ();
}

/**
 * Log empty line. Used to trim log outputs between multicast groups.
 */
static void LogEmptyLine ()
{
  NS_LOG_INFO ("");
}

/**
 * Establish multicast group and generate traffic in that group.
 *
 * \param helper Pointer to satellite helper
 * \param source Pointer to source node of the multicast group
 * \param groupReceivers Container of the receiver node pointers of the multicast group
 * \param groupAddress Address of the multicast group
 * \param port Port of the multicast group
 * \param startTime Time to start group traffic (sending start with some delay).
 * \param sinkToAll Add packet sink to all users for multicast group.
 * \return Time when all group packets should have been received by group receivers.
 */
static Time EstablishMulticastGroup (Ptr<SatHelper> helper, Ptr<Node> source, NodeContainer groupReceivers, Ipv4Address groupAddress, uint16_t port, Time startTime, bool sinkToAll )
{
  Time time = startTime;

  NS_LOG_INFO ("--- Create multicast group " << groupAddress << " ---");

  helper->SetMulticastGroupRoutes (source, groupReceivers, helper->GetUserAddress (source), groupAddress);

  NS_LOG_INFO ("--- Creating traffic generator for " << groupAddress << " ---");

  // CBR traffic generator for multicast group with interval 0.4s and packet size 512 bytes
  CbrHelper cbrHelper ("ns3::UdpSocketFactory", InetSocketAddress (groupAddress, port));
  cbrHelper.SetAttribute ("Interval", StringValue ("0.4s"));
  cbrHelper.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer cbr = cbrHelper.Install (source);
  cbr.Start (startTime + Seconds (0.4));
  cbr.Stop (startTime + Seconds (1.0));

  NS_LOG_INFO ("--- Group " << groupAddress << " source: " << GetUserInfo (helper, source) << " ---");

  ApplicationContainer sink;

  std::stringstream receiverInfo;
  receiverInfo << "--- Group " << groupAddress << " receivers: ";

  if ( sinkToAll )
    {
      NodeContainer users = NodeContainer (helper->GetUserHelper ()->GetGwUsers (), helper->GetUserHelper ()->GetUtUsers () );

      PacketSinkHelper sinkHelperGroup ("ns3::UdpSocketFactory", InetSocketAddress (groupAddress, port));

      // Sinks for group receivers
      for (uint32_t i = 0; i < users.GetN (); i++)
        {
          sink.Add (sinkHelperGroup.Install (users.Get (i)));

          std::string nodeName = GetUserInfo (helper, users.Get (i));
          std::stringstream context;
          context << groupAddress <<  "/" << nodeName;

          DynamicCast<PacketSink> (sink.Get (i))->TraceConnect ("Rx", context.str (), MakeCallback (&SinkReceive));
        }

      // output real group receivers
      for (uint32_t i = 0; i < groupReceivers.GetN (); i++)
        {
          std::string nodeName = GetUserInfo (helper, groupReceivers.Get (i));
          receiverInfo << nodeName << " ";
        }
    }
  else
    {
      PacketSinkHelper sinkHelperGroup ("ns3::UdpSocketFactory", InetSocketAddress (groupAddress, port));

      // Sinks for group receivers
      for (uint32_t i = 0; i < groupReceivers.GetN (); i++)
        {
          sink.Add (sinkHelperGroup.Install (groupReceivers.Get (i)));

          std::string nodeName = GetUserInfo (helper, groupReceivers.Get (i));
          receiverInfo << nodeName << " ";

          std::stringstream context;
          context << groupAddress <<  "/" << nodeName;

          DynamicCast<PacketSink> (sink.Get (i))->TraceConnect ("Rx", context.str (), MakeCallback (&SinkReceive));
        }
    }

  receiverInfo << "---";
  NS_LOG_INFO (receiverInfo.str ());

  sink.Start (startTime);
  sink.Stop (startTime + Seconds (2));

  Simulator::Schedule (startTime + Seconds (1.41), &LogEmptyLine);

  return startTime + Seconds (1);
}


int
main (int argc, char *argv[])
{
  /// Enable info logs
  LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("sat-multicast-example", LOG_LEVEL_INFO);

  NS_LOG_INFO ("--- Starting sat-multicast-example ---");

  enum PreDefinedGroup
  {
    ALL_GROUPS,
    GROUP_1,
    GROUP_2,
    GROUP_3,
    GROUP_4,
    GROUP_5,
    GROUP_6,
    GROUP_7,
    GROUP_8,
    GROUP_9,
    GROUP_10,
    GROUP_11,
    GROUP_12,
    END_OF_GROUP
  };

  uint32_t preDefinedGroup = (uint32_t) GROUP_1;
  uint32_t fullScenarioReceivers = 10;
  std::string scenario = "larger";
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::LARGER;
  bool sinkToAll = false; // when set sink(s) are added to all user nodes in order to test that unnecessary routes are not added

  Ptr<Node> groupSource;
  NodeContainer groupReceivers;

  /// Set simulation output details
  auto simulationHelper = CreateObject<SimulationHelper> ("example-multicast");
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (true));

  /// Read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue ("scenario", "Test scenario to use. (larger or full", scenario);
  cmd.AddValue ("preDefinedGroup", "Pre-defined multicast group for larger scenario. (0 = all)", preDefinedGroup);
  cmd.AddValue ("fullScenarioReceivers", "Number of the receivers in full scenario", fullScenarioReceivers);
  cmd.AddValue ("sinkToAll", "Add multicast sink to all users.", sinkToAll);
  simulationHelper->AddDefaultUiArguments (cmd);
  cmd.Parse (argc, argv);

  /// Set network types which support multicast
  Config::SetDefault ("ns3::SatUserHelper::BackboneNetworkType",EnumValue (SatUserHelper::NETWORK_TYPE_CSMA));
  Config::SetDefault ("ns3::SatUserHelper::SubscriberNetworkType",EnumValue (SatUserHelper::NETWORK_TYPE_CSMA));

  Time startTime = Seconds (1.2);
  simulationHelper->SetOutputTag (scenario);
  simulationHelper->SetSimulationTime (startTime + Seconds (3.0));

  if ( scenario == "larger")
    {
      satScenario = SatHelper::LARGER;
    }
  else if ( scenario == "full")
    {
      satScenario = SatHelper::FULL;
    }
  else
    {
      NS_FATAL_ERROR (satScenario << " not supported!!!");
    }

  /// Remove next line from comments to run real time simulation
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /// Create satellite helper with given scenario default=larger

  // Creating the reference system. Note, currently the satellite module supports
  // only one reference system, which is named as "Scenario72". The string is utilized
  // in mapping the scenario to the needed reference system configuration files. Arbitrary
  // scenario name results in fatal error.
  Ptr<SatHelper> helper = simulationHelper->CreateSatScenario (satScenario);

  NS_LOG_INFO ("--- Creating scenario: " << scenario << " ---");

  if (sinkToAll)
    {
      NS_LOG_INFO ("--- Add sink to each users ---");
    }


  uint16_t multicastPort = 9;   // Discard port (RFC 863)

  /// Get users
  NodeContainer utUsers = helper->GetUtUsers ();
  NodeContainer gwUsers = helper->GetGwUsers ();


  if ( scenario == "larger")
    {
      uint32_t currentGroup = preDefinedGroup;

      if ( preDefinedGroup == ALL_GROUPS )
        {
          currentGroup = 1;
        }

      for ( uint32_t i = currentGroup; i < END_OF_GROUP; i++ )
        {
          groupReceivers = NodeContainer ();

          switch (i)
            {
            // Pre-defined group 1, Source= GW-user-1, Receivers= UT1-user-1, UT1-user-2, UT2-user-1, UT3-user-1, UT4-user-1
            case GROUP_1:
              groupSource = gwUsers.Get (0);
              groupReceivers.Add (utUsers.Get (0));
              groupReceivers.Add (utUsers.Get (1));
              groupReceivers.Add (utUsers.Get (2));
              groupReceivers.Add (utUsers.Get (3));
              groupReceivers.Add (utUsers.Get (4));
              break;

            case GROUP_2:
              // Pre-defined group 2, Source= GW-user-1, Receivers= UT1-user-2, UT2-user-1, UT4-user-1
              groupSource = gwUsers.Get (0);
              groupReceivers.Add (utUsers.Get (1));
              groupReceivers.Add (utUsers.Get (2));
              groupReceivers.Add (utUsers.Get (4));
              break;

            case GROUP_3:
              // Pre-defined group 3, Source= UT1-user-1, Receivers= UT1-user-2, UT2-user-1, UT4-user-1
              groupSource = utUsers.Get (0);
              groupReceivers.Add (utUsers.Get (1));
              groupReceivers.Add (utUsers.Get (2));
              groupReceivers.Add (utUsers.Get (4));
              break;

            case GROUP_4:
              // Pre-defined group 4, Source= UT1-user-2, Receivers= UT2-user-1, UT4-user-1
              groupSource = utUsers.Get (1);
              groupReceivers.Add (utUsers.Get (2));
              groupReceivers.Add (utUsers.Get (4));
              break;

            case GROUP_5:
              // Pre-defined group 5, Source= UT1-user-1, Receivers= UT3-user-1, UT4-user-1
              groupSource = utUsers.Get (0);
              groupReceivers.Add (utUsers.Get (3));
              groupReceivers.Add (utUsers.Get (4));
              break;

            case GROUP_6:
              // Pre-defined group 6, Source= UT1-user-2, Receivers= UT2-user-1, GW-user-1
              groupSource = utUsers.Get (1);
              groupReceivers.Add (utUsers.Get (2));
              groupReceivers.Add (gwUsers.Get (0));
              break;

            case GROUP_7:
              // Pre-defined group 7, Source= UT1-user-1, Receivers= GW-user-1
              groupSource = utUsers.Get (0);
              groupReceivers.Add (gwUsers.Get (0));
              break;

            case GROUP_8:
              // Pre-defined group 8, Source= UT1-user-2, Receivers= UT2-user-1
              groupSource = utUsers.Get (1);
              groupReceivers.Add (utUsers.Get (2));
              break;

            case GROUP_9:
              // Pre-defined group 9, Source= UT1-user-1, Receivers= GW-user-1, UT4-user-1
              groupSource = utUsers.Get (0);
              groupReceivers.Add (gwUsers.Get (0));
              groupReceivers.Add (utUsers.Get (4));
              break;

            case GROUP_10:
              // Pre-defined group 10, Source= UT1-user-1, UT3-user-1
              groupSource = utUsers.Get (0);
              groupReceivers.Add (utUsers.Get (3));
              break;

            case GROUP_11:
              // Pre-defined group 10, Source= UT1-user-1, UT4-user-1
              groupSource = utUsers.Get (0);
              groupReceivers.Add (utUsers.Get (4));
              break;

            case GROUP_12:
              // Pre-defined group 10, Source= UT1-user-1, UT2-user-1
              groupSource = utUsers.Get (0);
              groupReceivers.Add (utUsers.Get (2));
              break;

            default:
              NS_FATAL_ERROR ("Not supported pre-defined group!!!");
              break;
            }

          NS_LOG_INFO ("--- Creating multicast pre-defined group " << preDefinedGroup << " ---");

          /// Create multicast groups 255.1.x.1, x predefined group number

          std::stringstream groupAddress;
          groupAddress << "225.1." << i << ".1";
          Ipv4Address multicastGroup (groupAddress.str ().c_str ());

          startTime = EstablishMulticastGroup (helper, groupSource, groupReceivers, multicastGroup, multicastPort, startTime, sinkToAll );
        }
    }
  else
    {
      NS_LOG_INFO ("--- Creating multicast group in full scenario ---");

      // get all users
      NodeContainer users = NodeContainer (gwUsers, utUsers);

      // add two sources from GW users and UT users randomly
      std::vector<uint32_t> sources;
      sources.push_back (std::rand () % gwUsers.GetN ());
      sources.push_back ( (std::rand () % utUsers.GetN ()) + gwUsers.GetN ());

      // create two multicast groups with UT and GW sources
      for (uint32_t i = 0; i < sources.size (); i++)
        {
          std::vector<uint32_t> ids;

          for (uint32_t j = 0; j < users.GetN (); j++)
            {
              // add user only to possible receivers only if not source
              if (j != sources[i] )
                {
                  ids.push_back (j);
                }
            }

          // randomize users
          std::random_shuffle (ids.begin (), ids.end ());

          // set source GW or UT users
          groupSource = users.Get (sources[i]);

          // select reeivers
          groupReceivers = NodeContainer ();

          for ( uint32_t j = 0; ( (j < fullScenarioReceivers) && ( j < ids.size ())); j++ )
            {
              groupReceivers.Add (users.Get (ids.at (j)));
            }

          std::stringstream groupAddress;
          groupAddress << "225.1." << i << ".1";
          Ipv4Address multicastGroup (groupAddress.str ().c_str ());

          startTime = EstablishMulticastGroup (helper, groupSource, groupReceivers, multicastGroup, multicastPort, startTime, sinkToAll );
        }
    }

  NS_LOG_INFO ("--- Running simulation ---");

  simulationHelper->RunSimulation ();

  NS_LOG_INFO ("--- Finished ---");
  return 0;
}
