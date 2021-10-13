/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"

#include "ns3/lora-logical-channel-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraLogicalChannelHelper");

NS_OBJECT_ENSURE_REGISTERED (LoraLogicalChannelHelper);

TypeId
LoraLogicalChannelHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraLogicalChannelHelper")
    .SetParent<Object> ()
    .SetGroupName ("lorawan");
  return tid;
}

LoraLogicalChannelHelper::LoraLogicalChannelHelper () :
  m_nextAggregatedTransmissionTime (Seconds (0)),
  m_aggregatedDutyCycle (1)
{
  NS_LOG_FUNCTION (this);
}

LoraLogicalChannelHelper::~LoraLogicalChannelHelper ()
{
  NS_LOG_FUNCTION (this);
}

std::vector<Ptr <LoraLogicalChannel> >
LoraLogicalChannelHelper::GetChannelList (void)
{
  NS_LOG_FUNCTION (this);

  // Make a copy of the channel vector
  std::vector<Ptr<LoraLogicalChannel> > vector;
  vector.reserve (m_channelList.size ());
  std::copy (m_channelList.begin (), m_channelList.end (), std::back_inserter
               (vector));

  return vector;
}


std::vector<Ptr <LoraLogicalChannel> >
LoraLogicalChannelHelper::GetEnabledChannelList (void)
{
  NS_LOG_FUNCTION (this);

  // Make a copy of the channel vector
  std::vector<Ptr<LoraLogicalChannel> > vector;
  vector.reserve (m_channelList.size ());
  std::copy (m_channelList.begin (), m_channelList.end (), std::back_inserter
               (vector));     // Working on a copy

  std::vector<Ptr <LoraLogicalChannel> > channels;
  std::vector<Ptr <LoraLogicalChannel> >::iterator it;
  for (it = vector.begin (); it != vector.end (); it++)
    {
      if ((*it)->IsEnabledForUplink ())
        {
          channels.push_back (*it);
        }
    }

  return channels;
}

Ptr<LoraSubBand>
LoraLogicalChannelHelper::GetLoraSubBandFromChannel (Ptr<LoraLogicalChannel> channel)
{
  return GetLoraSubBandFromFrequency (channel->GetFrequency ());
}

Ptr<LoraSubBand>
LoraLogicalChannelHelper::GetLoraSubBandFromFrequency (double frequency)
{
  std::cout << "LoraLogicalChannelHelper::GetLoraSubBandFromFrequency " << frequency << std::endl;
  // Get the LoraSubBand this frequency belongs to
  std::list< Ptr< LoraSubBand > >::iterator it;
  for (it = m_subBandList.begin (); it != m_subBandList.end (); it++)
    {
      if ((*it)->BelongsToLoraSubBand (frequency))
        {
          return *it;
        }
    }

  NS_LOG_ERROR ("Requested frequency: " << frequency);
  NS_ABORT_MSG ("Warning: frequency is outside any known LoraSubBand.");

  return 0;     // If no LoraSubBand is found, return 0
}

void
LoraLogicalChannelHelper::AddChannel (double frequency)
{
  NS_LOG_FUNCTION (this << frequency);

  // Create the new channel and increment the counter
  Ptr<LoraLogicalChannel> channel = Create<LoraLogicalChannel> (frequency);

  // Add it to the list
  m_channelList.push_back (channel);

  NS_LOG_DEBUG ("Added a channel. Current number of channels in list is " <<
                m_channelList.size ());
}

void
LoraLogicalChannelHelper::AddChannel (Ptr<LoraLogicalChannel> logicalChannel)
{
  NS_LOG_FUNCTION (this << logicalChannel);

  // Add it to the list
  m_channelList.push_back (logicalChannel);
}

void
LoraLogicalChannelHelper::SetChannel (uint8_t chIndex,
                                      Ptr<LoraLogicalChannel> logicalChannel)

{
  NS_LOG_FUNCTION (this << chIndex << logicalChannel);

  m_channelList.at (chIndex) = logicalChannel;
}

void
LoraLogicalChannelHelper::AddLoraSubBand (double firstFrequency,
                                      double lastFrequency, double dutyCycle,
                                      double maxTxPowerDbm)
{
  NS_LOG_FUNCTION (this << firstFrequency << lastFrequency);

  Ptr<LoraSubBand> subBand = Create<LoraSubBand> (firstFrequency, lastFrequency,
                                          dutyCycle, maxTxPowerDbm);

  m_subBandList.push_back (subBand);
}

void
LoraLogicalChannelHelper::AddLoraSubBand (Ptr<LoraSubBand> subBand)
{
  NS_LOG_FUNCTION (this << subBand);

  m_subBandList.push_back (subBand);
}

void
LoraLogicalChannelHelper::RemoveChannel (Ptr<LoraLogicalChannel> logicalChannel)
{
  // Search and remove the channel from the list
  std::vector<Ptr<LoraLogicalChannel> >::iterator it;
  for (it = m_channelList.begin (); it != m_channelList.end (); it++)
    {
      Ptr<LoraLogicalChannel> currentChannel = *it;
      if (currentChannel == logicalChannel)
        {
          m_channelList.erase (it);
          return;
        }
    }
}

Time
LoraLogicalChannelHelper::GetAggregatedWaitingTime (void)
{
  // Aggregate waiting time
  Time aggregatedWaitingTime = m_nextAggregatedTransmissionTime - Simulator::Now ();

  // Handle case in which waiting time is negative
  aggregatedWaitingTime = Seconds (std::max (aggregatedWaitingTime.GetSeconds (), double(0)));

  NS_LOG_DEBUG ("Aggregated waiting time: " << aggregatedWaitingTime.GetSeconds ());

  return aggregatedWaitingTime;
}

Time
LoraLogicalChannelHelper::GetWaitingTime (Ptr<LoraLogicalChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);

  // LoraSubBand waiting time
  Time subBandWaitingTime = GetLoraSubBandFromChannel (channel)->GetNextTransmissionTime () - Simulator::Now ();

  // Handle case in which waiting time is negative
  subBandWaitingTime = Seconds (std::max (subBandWaitingTime.GetSeconds (), double(0)));

  NS_LOG_DEBUG ("Waiting time: " << subBandWaitingTime.GetSeconds ());

  return subBandWaitingTime;
}

void
LoraLogicalChannelHelper::AddEvent (Time duration,
                                    Ptr<LoraLogicalChannel> channel)
{
  NS_LOG_FUNCTION (this << duration << channel);

  Ptr<LoraSubBand> subBand = GetLoraSubBandFromChannel (channel);

  double dutyCycle = subBand->GetDutyCycle ();
  double timeOnAir = duration.GetSeconds ();

  // Computation of necessary waiting time on this sub-band
  subBand->SetNextTransmissionTime (Simulator::Now () + Seconds
                                      (timeOnAir / dutyCycle - timeOnAir));

  // Computation of necessary aggregate waiting time
  m_nextAggregatedTransmissionTime = Simulator::Now () + Seconds
      (timeOnAir / m_aggregatedDutyCycle - timeOnAir);

  NS_LOG_DEBUG ("Time on air: " << timeOnAir);
  NS_LOG_DEBUG ("m_aggregatedDutyCycle: " << m_aggregatedDutyCycle);
  NS_LOG_DEBUG ("Current time: " << Simulator::Now ().GetSeconds ());
  NS_LOG_DEBUG ("Next transmission on this sub-band allowed at time: " <<
                (subBand->GetNextTransmissionTime ()).GetSeconds ());
  NS_LOG_DEBUG ("Next aggregated transmission allowed at time " <<
                m_nextAggregatedTransmissionTime.GetSeconds ());
}

double
LoraLogicalChannelHelper::GetTxPowerForChannel (Ptr<LoraLogicalChannel>
                                                logicalChannel)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Get the maxTxPowerDbm from the LoraSubBand this channel is in
  std::list< Ptr< LoraSubBand > >::iterator it;
  for (it = m_subBandList.begin (); it != m_subBandList.end (); it++)
    {
      // Check whether this channel is in this LoraSubBand
      if ((*it)->BelongsToLoraSubBand (logicalChannel->GetFrequency ()))
        {
          return (*it)->GetMaxTxPowerDbm ();
        }
    }
  NS_ABORT_MSG ("Logical channel doesn't belong to a known LoraSubBand");

  return 0;
}

void
LoraLogicalChannelHelper::DisableChannel (int index)
{
  NS_LOG_FUNCTION (this << index);

  m_channelList.at (index)->DisableForUplink ();
}
}
