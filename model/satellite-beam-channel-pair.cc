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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */


#include "ns3/log.h"
#include "ns3/fatal-error.h"

#include "satellite-enums.h"
#include "satellite-beam-channel-pair.h"


NS_LOG_COMPONENT_DEFINE ("SatChannelPair");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatChannelPair);

TypeId
SatChannelPair::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatChannelPair")
    .SetParent<Object> ()
    .AddConstructor<SatChannelPair> ()
  ;
  return tid;
}


SatChannelPair::SatChannelPair ()
  : m_channels ()
{
  NS_LOG_FUNCTION (this);
}


SatChannelPair::~SatChannelPair ()
{
  NS_LOG_FUNCTION (this);
}


SatChannelPair::ChannelPair_t
SatChannelPair::GetChannelPair (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);

  std::map<uint32_t, uint32_t>::iterator frequencyIterator = m_frequencies.find (beamId);
  if (frequencyIterator == m_frequencies.end ())
    {
      NS_FATAL_ERROR ("No SatChannel stored for this beam");
    }

  uint32_t frequencyId = frequencyIterator->second;
  std::map<uint32_t, ChannelPair_t >::iterator channelIterator = m_channels.find (frequencyId);
  if (channelIterator == m_channels.end ())
    {
      NS_FATAL_ERROR ("No SatChannel stored for the frequency ID this beam is associated to");
    }

  return channelIterator->second;
}


bool
SatChannelPair::HasChannelPair (uint32_t frequencyId)
{
  NS_LOG_FUNCTION (this << frequencyId);

  std::map<uint32_t, ChannelPair_t>::iterator channel = m_channels.find (frequencyId);
  return channel != m_channels.end ();
}


void
SatChannelPair::UpdateBeamsForFrequency (uint32_t beamId, uint32_t frequencyId)
{
  NS_LOG_FUNCTION (this << beamId << frequencyId);

  std::pair<uint32_t, uint32_t> frequencyKey = std::make_pair (beamId, frequencyId);
  std::pair<std::map<uint32_t, uint32_t>::iterator, bool> frequencyCreated = m_frequencies.insert (frequencyKey);
  if (!frequencyCreated.second)
    {
      NS_FATAL_ERROR ("SatChannel pair already created for this beam");
    }
}


void
SatChannelPair::StoreChannelPair (uint32_t beamId, uint32_t frequencyId, ChannelPair_t channelPair)
{
  NS_LOG_FUNCTION (this << beamId << frequencyId);

  std::pair<uint32_t, ChannelPair_t> channelsKey = std::make_pair (frequencyId, channelPair);
  std::pair<std::map<uint32_t, ChannelPair_t>::iterator, bool> channelsCreated = m_channels.insert (channelsKey);
  if (!channelsCreated.second)
    {
      NS_FATAL_ERROR ("SatChannel pair already created for this frequency");
    }

  UpdateBeamsForFrequency (beamId, frequencyId);
}

}
