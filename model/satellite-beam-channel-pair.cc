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


#include <ns3/log.h>
#include <ns3/fatal-error.h>

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
  : m_frequencies (),
  m_fwdChannels (),
  m_rtnChannels ()
{
  NS_LOG_FUNCTION (this);
}


SatChannelPair::~SatChannelPair ()
{
  NS_LOG_FUNCTION (this);
  m_frequencies.clear ();
  m_fwdChannels.clear ();
  m_rtnChannels.clear ();
}


SatChannelPair::ChannelPair_t
SatChannelPair::GetChannelPair (uint32_t beamId) const
{
  NS_LOG_FUNCTION (this << beamId);

  std::map<uint32_t, std::pair<uint32_t, uint32_t> >::const_iterator frequencyIterator = m_frequencies.find (beamId);
  if (frequencyIterator == m_frequencies.end ())
    {
      NS_FATAL_ERROR ("No SatChannel stored for this beam");
    }

  std::pair<uint32_t, uint32_t> frequenciesIds = frequencyIterator->second;
  return std::make_pair (GetForwardChannel (frequenciesIds.first), GetReturnChannel (frequenciesIds.second));
}


Ptr<SatChannel>
SatChannelPair::GetForwardChannel (uint32_t frequencyId) const
{
  std::map<uint32_t, Ptr<SatChannel>>::const_iterator channelIterator = m_fwdChannels.find (frequencyId);
  if (channelIterator == m_fwdChannels.end ())
    {
      NS_FATAL_ERROR ("No SatChannel stored for the forward frequency " << frequencyId);
    }

  return channelIterator->second;
}


Ptr<SatChannel>
SatChannelPair::GetReturnChannel (uint32_t frequencyId) const
{
  std::map<uint32_t, Ptr<SatChannel>>::const_iterator channelIterator = m_rtnChannels.find (frequencyId);
  if (channelIterator == m_rtnChannels.end ())
    {
      NS_FATAL_ERROR ("No SatChannel stored for the return frequency " << frequencyId);
    }

  return channelIterator->second;
}


bool
SatChannelPair::HasFwdChannel (uint32_t frequencyId) const
{
  NS_LOG_FUNCTION (this << frequencyId);

  std::map<uint32_t, Ptr<SatChannel> >::const_iterator channel = m_fwdChannels.find (frequencyId);
  return channel != m_fwdChannels.end ();
}


bool
SatChannelPair::HasRtnChannel (uint32_t frequencyId) const
{
  NS_LOG_FUNCTION (this << frequencyId);

  std::map<uint32_t, Ptr<SatChannel> >::const_iterator channel = m_rtnChannels.find (frequencyId);
  return channel != m_rtnChannels.end ();
}


void
SatChannelPair::UpdateBeamsForFrequency (uint32_t beamId, uint32_t fwdFrequencyId, uint32_t rtnFrequencyId)
{
  NS_LOG_FUNCTION (this << beamId << fwdFrequencyId << rtnFrequencyId);

  std::pair<uint32_t, uint32_t> frequenciesIds = std::make_pair (fwdFrequencyId, rtnFrequencyId);
  std::pair<uint32_t, std::pair<uint32_t, uint32_t>> frequencyKey = std::make_pair (beamId, frequenciesIds);
  std::pair<std::map<uint32_t, std::pair<uint32_t, uint32_t>>::iterator, bool> frequencyCreated = m_frequencies.insert (frequencyKey);
  if (!frequencyCreated.second)
    {
      NS_FATAL_ERROR ("SatChannel pair already created for this beam");
    }
}


void
SatChannelPair::StoreChannelPair (uint32_t beamId,
                                  uint32_t fwdFrequencyId,
                                  Ptr<SatChannel> fwdChannel,
                                  uint32_t rtnFrequencyId,
                                  Ptr<SatChannel> rtnChannel)
{
  NS_LOG_FUNCTION (this << beamId << fwdFrequencyId << fwdChannel << rtnFrequencyId << rtnChannel);

  std::map<uint32_t, Ptr<SatChannel>>::iterator fwdChannelIterator = m_fwdChannels.find (fwdFrequencyId);
  if (fwdChannelIterator != m_fwdChannels.end () && fwdChannelIterator->second != fwdChannel)
    {
      NS_FATAL_ERROR ("SatChannel already created for the forward frequency " << fwdFrequencyId);
    }

  std::map<uint32_t, Ptr<SatChannel>>::iterator rtnChannelIterator = m_rtnChannels.find (rtnFrequencyId);
  if (rtnChannelIterator != m_rtnChannels.end () && rtnChannelIterator->second != rtnChannel)
    {
      NS_FATAL_ERROR ("SatChannel already created for the return frequency " << rtnFrequencyId);
    }

  m_fwdChannels.emplace (fwdFrequencyId, fwdChannel);
  m_rtnChannels.emplace (rtnFrequencyId, rtnChannel);

  UpdateBeamsForFrequency (beamId, fwdFrequencyId, rtnFrequencyId);
}

}
