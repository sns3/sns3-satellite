/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#include "ns3/log.h"
#include "satellite-bbframe-container.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrameContainer");

namespace ns3 {


SatBbFrameContainer::SatBbFrameContainer ()
: m_priorityClassCount (2),
  m_totalDuration (Seconds (0))
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor of SatBbFrameContainer not supported.");
}

SatBbFrameContainer::SatBbFrameContainer (uint32_t priorityClassCount)
 : m_priorityClassCount (priorityClassCount),
   m_totalDuration (Seconds (0))
{
  NS_LOG_FUNCTION (this << priorityClassCount);
}

SatBbFrameContainer::~SatBbFrameContainer ()
{
  NS_LOG_FUNCTION (this);

  m_container.clear ();
}

void
SatBbFrameContainer::AddFrame (uint32_t priorityClass, Ptr<SatBbFrame> frame)
{
  NS_LOG_FUNCTION (this);

  if ( priorityClass < m_priorityClassCount )
    {
      m_container.insert (std::make_pair (priorityClass, frame));
      m_totalDuration += frame->GetDuration();
    }
  else
    {
      NS_FATAL_ERROR ("Priority out of range!!!");
    }
}

Time
SatBbFrameContainer::GetTotalDuration () const
{
  return m_totalDuration;
}

Ptr<SatBbFrame>
SatBbFrameContainer::GetNextFrame ()
{
  Ptr<SatBbFrame> nextFrame = NULL;

  FrameContainer_t::iterator it = m_container.begin ();

  if ( it != m_container.end () )
    {
      nextFrame = it->second;
      m_container.erase (it);
      m_totalDuration -= nextFrame->GetDuration();
    }

  return nextFrame;
}


} // namespace ns3
