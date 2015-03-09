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
 * \file sat-markov-logic-example.cc
 * \ingroup satellite
 *
 * \brief Example for Markov model class logic. Demonstrates the functionality
 * of Markov-fading. The example creates Markov-fading object and executes the
 * fading calculations with different elevation angles.
 */

NS_LOG_COMPONENT_DEFINE ("sat-markov-logic-example");

static double g_elevation = 45;
static double g_velocity = 0;

static double GetElevation ()
{
  return g_elevation;
}

static void SetElevation (double elevation)
{
  g_elevation = elevation;
}

static double GetVelocity ()
{
  return g_velocity;
}

static void SetVelocity (double velocity)
{
  g_velocity = velocity;
}

int
main (int argc, char *argv[])
{
  /// Enable info logs
  LogComponentEnable ("sat-markov-logic-example", LOG_LEVEL_INFO);
  LogComponentEnable ("SatMarkovContainer", LOG_LEVEL_INFO);
  LogComponentEnable ("SatMarkovModel", LOG_LEVEL_INFO);
  LogComponentEnable ("SatMarkovConf", LOG_LEVEL_INFO);

  /// Set simulation output details
  Config::SetDefault ("ns3::SatEnvVariables::SimulationCampaignName", StringValue ("example-markov-logic"));
  Config::SetDefault ("ns3::SatEnvVariables::SimulationTag", StringValue (""));
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));

  /// Create default Markov & Loo configurations
  Ptr<SatMarkovConf> markovConf = CreateObject<SatMarkovConf> ();

  SatBaseFading::ElevationCallback elevationCb = MakeCallback (&GetElevation);
  SatBaseFading::VelocityCallback velocityCb = MakeCallback (&GetVelocity);

  /// Create fading container based on default configuration
  Ptr<SatMarkovContainer> markovContainer = CreateObject<SatMarkovContainer> (markovConf,elevationCb,velocityCb);

  Address macAddress;

  /// Run simulation
  Simulator::Schedule (Time ("5ms"), &SetVelocity, 0);
  Simulator::Schedule (Time ("10ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Time ("30ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Time ("45ms"), &SetElevation, 55);
  Simulator::Schedule (Time ("50ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Time ("60ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Time ("90ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Time ("95ms"), &SetElevation, 75);
  Simulator::Schedule (Time ("100ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Time ("130ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
  Simulator::Schedule (Time ("200ms"), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
