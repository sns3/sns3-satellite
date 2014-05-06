/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include <ns3/log.h>
#include <ns3/traced-callback.h>
#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <ns3/packet.h>
#include "satellite-time-tag.h"
#include "satellite-on-off-application.h"

NS_LOG_COMPONENT_DEFINE ("SatOnOffApplication");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatOnOffApplication);

TypeId
SatOnOffApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatOnOffApplication")
    .SetParent<OnOffApplication> ()
    .AddConstructor<SatOnOffApplication> ()
    .AddAttribute ("EnableStatisticsTags",
                   "If true, some tags will be added to each transmitted packet to assist with statistics computation",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatOnOffApplication::EnableStatisticTags,
                                        &SatOnOffApplication::IsStatisticTagsEnabled),
                   MakeBooleanChecker ())
  ;
  return tid;
}


SatOnOffApplication::SatOnOffApplication ()
 :  m_isStatisticsTagsEnabled (false)
{
  NS_LOG_FUNCTION (this);
}

SatOnOffApplication::~SatOnOffApplication()
{
  NS_LOG_FUNCTION (this);
}

void SatOnOffApplication::EnableStatisticTags (bool enable)
{
  m_isStatisticsTagsEnabled = enable;

  if ( m_isStatisticsTagsEnabled )
    {
      TraceConnectWithoutContext ("Tx", MakeCallback (&SatOnOffApplication::SendPacketTrace, this) );
    }
  else
    {
      TraceDisconnectWithoutContext ("Tx", MakeCallback (&SatOnOffApplication::SendPacketTrace, this) );
    }
}

bool SatOnOffApplication::IsStatisticTagsEnabled () const
{
  return m_isStatisticsTagsEnabled;
}

void SatOnOffApplication::SendPacketTrace (Ptr<const Packet> packet)
{
   // Add a SatAppTimeTag tag for packet delay computation at the receiver end.
   packet->AddPacketTag (SatAppTimeTag (Simulator::Now ()));
}

} // Namespace ns3

