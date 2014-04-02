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

#include <algorithm>
#include "ns3/log.h"
#include "satellite-utils.h"
#include "satellite-bbframe-container.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrameContainer");

namespace ns3 {


SatBbFrameContainer::SatBbFrameContainer ()
: m_totalDuration (Seconds (0))
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor of SatBbFrameContainer not supported.");
}

SatBbFrameContainer::SatBbFrameContainer (std::vector<SatEnums::SatModcod_t>& modcodsInUse, Ptr<SatBbFrameConf> conf)
 : m_totalDuration (Seconds (0)),
   m_bbFrameConf (conf)
{
  NS_LOG_FUNCTION (this);

  for (std::vector<SatEnums::SatModcod_t>::const_iterator it = modcodsInUse.begin (); it != modcodsInUse.end (); it++)
    {
      std::pair<FrameContainer_t::iterator, bool> result = m_container.insert (std::make_pair (*it, std::deque<Ptr<SatBbFrame> > ()) );

      if ( result.second == false )
        {
          NS_FATAL_ERROR ("Queue for MODCOD: " << *it << " already exists!!!");
        }
    }

}

SatBbFrameContainer::~SatBbFrameContainer ()
{
  NS_LOG_FUNCTION (this);

  m_container.clear ();
}

SatEnums::SatModcod_t
SatBbFrameContainer::GetModcod (uint32_t priorityClass, double cno)
{
  NS_LOG_FUNCTION (this);

  SatEnums::SatModcod_t modcod = m_bbFrameConf->GetDefaultModCod ();

  if ( priorityClass > 0)
    {
      modcod = m_bbFrameConf->GetBestModcod (cno, SatEnums::NORMAL_FRAME);
    }

  return modcod;
}

uint32_t
SatBbFrameContainer::GetMaxFramePayloadInBytes (uint32_t priorityClass, SatEnums::SatModcod_t modcod)
{
  NS_LOG_FUNCTION (this);

  uint32_t payloadBytes = 0;

  if ( priorityClass > 0)
    {
      payloadBytes = m_bbFrameConf->GetBbFramePayloadBits (m_bbFrameConf->GetDefaultModCod (), SatEnums::NORMAL_FRAME) / SatUtils::BITS_PER_BYTE;
    }
  else
    {
      payloadBytes = m_bbFrameConf->GetBbFramePayloadBits (modcod, SatEnums::NORMAL_FRAME) / SatUtils::BITS_PER_BYTE;
    }

  return payloadBytes;
}

uint32_t
SatBbFrameContainer::GetBytesLeftInTailFrame (uint32_t priorityClass, SatEnums::SatModcod_t modcod)
{
  NS_LOG_FUNCTION (this);

  uint32_t bytesLeft = GetMaxFramePayloadInBytes (priorityClass, modcod);

  if ( priorityClass > 0)
    {
      if ( m_container.at (modcod).empty () != true )
        {
          bytesLeft = m_container.at (modcod).back ()->GetSpaceLeftInBytes ();
        }
    }
  else
    {
      if ( m_ctrlContainer.empty () != true )
        {
          bytesLeft = m_ctrlContainer.back ()->GetSpaceLeftInBytes ();
        }
    }

  return bytesLeft;
}

void
SatBbFrameContainer::AddData (uint32_t priorityClass, SatEnums::SatModcod_t modcod, Ptr<Packet> data)
{
  NS_LOG_FUNCTION (this);

  if ( priorityClass > 0)
    {
      if ( m_container.at (modcod).empty () ||
           ( m_container.at (modcod).back ()->GetSpaceLeftInBytes () < data->GetSize () ) )
        {
          // create and add frame to tail
          Ptr<SatBbFrame> bbFrame = Create<SatBbFrame> (m_bbFrameConf->GetDefaultModCod (), SatEnums::NORMAL_FRAME, m_bbFrameConf);
          m_container.at (modcod).push_back (bbFrame);
          m_totalDuration += bbFrame->GetDuration ();
        }

      m_container.at (modcod).back ()->AddPayload (data);

    }
  else
    {
      if ( m_ctrlContainer.empty () ||
           ( m_ctrlContainer.back ()->GetSpaceLeftInBytes () < data->GetSize () ) )
        {
          // create and add frame to tail
          Ptr<SatBbFrame> bbFrame = Create<SatBbFrame> (modcod, SatEnums::NORMAL_FRAME, m_bbFrameConf);
          m_ctrlContainer.push_back (bbFrame);
          m_totalDuration += bbFrame->GetDuration ();
        }

      m_ctrlContainer.back ()->AddPayload (data);

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

  if ( m_ctrlContainer.empty () == false )
    {
      nextFrame = *m_ctrlContainer.begin ();
      m_ctrlContainer.pop_front ();
      m_totalDuration -= nextFrame->GetDuration();
    }
  else
    {
      std::vector< std::deque<Ptr<SatBbFrame> >* > nonEmpytQueues;

      for (FrameContainer_t::iterator it = m_container.begin (); it != m_container.end (); it++ )
        {
          if ( (*it).second.empty () == false )
            {
              nonEmpytQueues.push_back (&it->second);
            }
        }

      if ( nonEmpytQueues.empty () == false )
        {
          std::random_shuffle( nonEmpytQueues.begin (), nonEmpytQueues.end ());

          nextFrame = *(*nonEmpytQueues.begin ())->begin ();
          (*nonEmpytQueues.begin ())->pop_front ();
          m_totalDuration -= nextFrame->GetDuration();
        }
    }

  return nextFrame;
}


} // namespace ns3
