/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions
 * Copyright (c) 2020 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "satellite-cno-helper.h"

#include "ns3/core-module.h"
#include "ns3/satellite-rx-cno-input-trace-container.h"
#include "ns3/satellite-id-mapper.h"
#include "ns3/singleton.h"
#include "ns3/enum.h"

NS_LOG_COMPONENT_DEFINE ("SatelliteCnoHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatCnoHelper);

TypeId
SatCnoHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatCnoHelper")
    .SetParent<Object> ()
    .AddConstructor<SatCnoHelper> ();
  return tid;
}

TypeId
SatCnoHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatCnoHelper::SatCnoHelper ()
  : m_satHelper (NULL),
  m_useTraces (false)
{
  Config::Set ("/ChannelList/*/$ns3::SatChannel/RxPowerCalculationMode", EnumValue (SatEnums::RX_CNO_INPUT_TRACE));
}

SatCnoHelper::SatCnoHelper (Ptr<SatHelper> satHelper)
  : m_satHelper (satHelper),
  m_useTraces (false)
{
  Config::Set ("/ChannelList/*/$ns3::SatChannel/RxPowerCalculationMode", EnumValue (SatEnums::RX_CNO_INPUT_TRACE));
}

void
SatCnoHelper::SetUseTraces (bool useTraces)
{
  m_useTraces = useTraces;
  ApplyConfiguration ();
}

void
SatCnoHelper::SetGwNodeCno (Ptr<Node> node, SatEnums::ChannelType_t channel, double cno)
{
  if (channel != SatEnums::FORWARD_FEEDER_CH && channel != SatEnums::RETURN_FEEDER_CH)
    {
      NS_FATAL_ERROR ("Can only apply custom GWs C/N0 on feeder channels");
    }
  if (CheckDuplicate(node, channel))
    {
      NS_FATAL_ERROR ("Trying to set custom C/N0 several time for same node and channel");
    }
  cnoCustomParams_s params;
  params.node = node;
  params.isGw = true;
  params.constant = true;
  params.channelType = channel;
  params.cno = cno;
  m_customCno.push_back(params);

  ApplyConfiguration ();
}

void
SatCnoHelper::SetUtNodeCno (Ptr<Node> node, SatEnums::ChannelType_t channel, double cno)
{
  if (channel != SatEnums::FORWARD_USER_CH && channel != SatEnums::RETURN_USER_CH)
    {
      NS_FATAL_ERROR ("Can only apply custom UTs C/N0 on user channels");
    }
  if (CheckDuplicate(node, channel))
    {
      NS_FATAL_ERROR ("Trying to set custom C/N0 several time for same node and channel");
    }
  cnoCustomParams_s params;
  params.node = node;
  params.isGw = false;
  params.constant = true;
  params.channelType = channel;
  params.cno = cno;
  m_customCno.push_back(params);

  ApplyConfiguration ();
}

void
SatCnoHelper::SetGwNodeCno (uint32_t nodeId, SatEnums::ChannelType_t channel, double cno)
{
  SetGwNodeCno (m_satHelper->GetBeamHelper ()->GetGwNodes ().Get (nodeId), channel, cno);
}

void
SatCnoHelper::SetUtNodeCno (uint32_t nodeId, SatEnums::ChannelType_t channel, double cno)
{
  SetUtNodeCno (m_satHelper->GetBeamHelper ()->GetUtNodes ().Get (nodeId), channel, cno);
}

void
SatCnoHelper::SetGwNodeCno (NodeContainer nodes, SatEnums::ChannelType_t channel, double cno)
{
  for(uint32_t i = 0; i < nodes.GetN (); i++)
    {
      SetGwNodeCno (nodes.Get (i), channel, cno);
    }
}

void
SatCnoHelper::SetUtNodeCno (NodeContainer nodes, SatEnums::ChannelType_t channel, double cno)
{
  for(uint32_t i = 0; i < nodes.GetN (); i++)
    {
      SetUtNodeCno (nodes.Get (i), channel, cno);
    }
}

void
SatCnoHelper::SetGwNodeCnoFile (Ptr<Node> node, SatEnums::ChannelType_t channel, std::string path)
{
  if (channel != SatEnums::FORWARD_FEEDER_CH && channel != SatEnums::RETURN_FEEDER_CH)
    {
      NS_FATAL_ERROR ("Can only apply custom GWs C/N0 on feeder channels");
    }
  if (CheckDuplicate(node, channel))
    {
      NS_FATAL_ERROR ("Trying to set custom C/N0 several time for same node and channel");
    }
  cnoCustomParams_s params;
  params.node = node;
  params.isGw = true;
  params.constant = false;
  params.channelType = channel;
  params.pathToFile = path;
  m_customCno.push_back(params);

  ApplyConfiguration ();
}

void
SatCnoHelper::SetUtNodeCnoFile (Ptr<Node> node, SatEnums::ChannelType_t channel, std::string path)
{
  if (channel != SatEnums::FORWARD_USER_CH && channel != SatEnums::RETURN_USER_CH)
    {
      NS_FATAL_ERROR ("Can only apply custom UTs C/N0 on user channels");
    }
  if (CheckDuplicate(node, channel))
    {
      NS_FATAL_ERROR ("Trying to set custom C/N0 several time for same node and channel");
    }
  cnoCustomParams_s params;
  params.node = node;
  params.isGw = false;
  params.constant = false;
  params.channelType = channel;
  params.pathToFile = path;
  m_customCno.push_back(params);

  ApplyConfiguration ();
}

void
SatCnoHelper::SetGwNodeCnoFile (uint32_t nodeId, SatEnums::ChannelType_t channel, std::string path)
{
  SetGwNodeCnoFile (m_satHelper->GetBeamHelper ()->GetGwNodes ().Get (nodeId), channel, path);
}

void
SatCnoHelper::SetUtNodeCnoFile (uint32_t nodeId, SatEnums::ChannelType_t channel, std::string path)
{
  SetUtNodeCnoFile (m_satHelper->GetBeamHelper ()->GetUtNodes ().Get (nodeId), channel, path);
}

void
SatCnoHelper::ApplyConfiguration ()
{
  Singleton<SatRxCnoInputTraceContainer>::Get ()->Reset ();

  std::pair<Address, SatEnums::ChannelType_t> key;
  // set default value for all nodes
  if (!m_useTraces)
    {
      // use power calculation from satellite-channel
      Ptr<Node> gwNode;
      for (uint32_t i = 0; i < m_satHelper->GetBeamHelper ()->GetGwNodes ().GetN (); i++)
        {
          gwNode = m_satHelper->GetBeamHelper ()->GetGwNodes ().Get (i);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetGwMacWithNode (gwNode), SatEnums::FORWARD_FEEDER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->SetRxCno (key, 0);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetGwMacWithNode (gwNode), SatEnums::RETURN_FEEDER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->SetRxCno (key, 0);
        }
      Ptr<Node> utNode;
      for (uint32_t i = 0; i < m_satHelper->GetBeamHelper ()->GetUtNodes ().GetN (); i++)
        {
          utNode = m_satHelper->GetBeamHelper ()->GetUtNodes ().Get (i);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (utNode), SatEnums::FORWARD_USER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->SetRxCno (key, 0);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (utNode), SatEnums::RETURN_USER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->SetRxCno (key, 0);
        }
    }
  else
    {
      // use input files from data/rxcnotraces/input folder
      Ptr<Node> gwNode;
      for (uint32_t i = 0; i < m_satHelper->GetBeamHelper ()->GetGwNodes ().GetN (); i++)
        {
          gwNode = m_satHelper->GetBeamHelper ()->GetGwNodes ().Get (i);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetGwMacWithNode (gwNode), SatEnums::FORWARD_FEEDER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->AddNode (key);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetGwMacWithNode (gwNode), SatEnums::RETURN_FEEDER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->AddNode (key);
        }
      Ptr<Node> utNode;
      for (uint32_t i = 0; i < m_satHelper->GetBeamHelper ()->GetUtNodes ().GetN (); i++)
        {
          utNode = m_satHelper->GetBeamHelper ()->GetUtNodes ().Get (i);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (utNode), SatEnums::FORWARD_USER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->AddNode (key);
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (utNode), SatEnums::RETURN_USER_CH);
          Singleton<SatRxCnoInputTraceContainer>::Get ()->AddNode (key);
        }
    }

  // set custom values. The values will cancel the default values set above
  for (cnoCustomParams_s params : m_customCno)
    {
      if (params.isGw)
        {
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetGwMacWithNode (params.node), params.channelType);
        }
      else
        {
          key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (params.node), params.channelType);
        }
      if (params.constant)
        {
          // Set constant value
          Singleton<SatRxCnoInputTraceContainer>::Get ()->SetRxCno (key, params.cno);
        }
      else
        {
          // Set custom input file
          Singleton<SatRxCnoInputTraceContainer>::Get ()->SetRxCnoFile (key, params.pathToFile);
        }
    }
}

bool
SatCnoHelper::CheckDuplicate(Ptr<Node> node, SatEnums::ChannelType_t channel)
{
  for (cnoCustomParams_s params : m_customCno)
    {
      if (params.node == node && params.channelType == channel)
        {
          return true;
        }
    }
  return false;
}

} // namespace ns3