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
  Config::SetDefault ("ns3::SatChannel::RxPowerCalculationMode", EnumValue (SatEnums::RX_CNO_INPUT_TRACE));
  //Config::Set ("/NodeList/*/DeviceList/*/$ns3::SatChannel/RxPowerCalculationMode",
  //           EnumValue (SatEnums::RX_CNO_INPUT_TRACE));
}

SatCnoHelper::SatCnoHelper (Ptr<SatHelper> satHelper)
  : m_satHelper (satHelper),
  m_useTraces (false)
{
  Config::SetDefault ("ns3::SatChannel::RxPowerCalculationMode", EnumValue (SatEnums::RX_CNO_INPUT_TRACE));
  //Config::Set ("/NodeList/*/DeviceList/*/$ns3::SatChannel/RxPowerCalculationMode",
  //          EnumValue (SatEnums::RX_CNO_INPUT_TRACE));
}

void
SatCnoHelper::SetUseTraces (bool useTraces)
{
  m_useTraces = useTraces;
}

void
SatCnoHelper::SetGwNodeCno (Ptr<Node> node, SatEnums::ChannelType_t channel, double cno)
{
  if (channel != SatEnums::FORWARD_FEEDER_CH && channel != SatEnums::RETURN_FEEDER_CH)
    {
      NS_FATAL_ERROR ("Can only apply custom GWs C/N0 on feeder channels");
    }
  cnoCustomParams_s params;
  params.node = node;
  params.isGw = true;
  params.constant = true;
  params.channelType = channel;
  params.cno = cno;
  m_customCno.push_back(params);
}

void
SatCnoHelper::SetUtNodeCno (Ptr<Node> node, SatEnums::ChannelType_t channel, double cno)
{
  if (channel != SatEnums::FORWARD_USER_CH && channel != SatEnums::RETURN_USER_CH)
    {
      NS_FATAL_ERROR ("Can only apply custom UTs C/N0 on user channels");
    }
  cnoCustomParams_s params;
  params.node = node;
  params.isGw = false;
  params.constant = true;
  params.channelType = channel;
  params.cno = cno;
  m_customCno.push_back(params);
}

void
SatCnoHelper::ApplyConfiguration ()
{
  Singleton<SatRxCnoInputTraceContainer>::Get ()->Reset ();

  std::pair<Address, SatEnums::ChannelType_t> key;
  if (!m_useTraces)
    {
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

  for (cnoCustomParams_s params : m_customCno)
    {
      if (params.constant)
        {
          if (params.isGw)
            {
              key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetGwMacWithNode (params.node), params.channelType);
            }
          else
            {
              key = std::make_pair(Singleton<SatIdMapper>::Get ()->GetUtMacWithNode (params.node), params.channelType);
            }
          Singleton<SatRxCnoInputTraceContainer>::Get ()->SetRxCno (key, params.cno);
        }
      else
        {
          //TODO
        }
    }
}

} // namespace ns3