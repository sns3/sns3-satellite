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
#include "ns3/enum.h"
#include "satellite-utils.h"
#include "satellite-const-variables.h"
#include "satellite-bbframe-container.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrameContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBbFrameContainer);

SatBbFrameContainer::SatBbFrameContainer ()
  : m_totalDuration (Seconds (0)),
    m_defaultBbFrameType (SatEnums::NORMAL_FRAME)
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

  m_defaultBbFrameType = SatEnums::NORMAL_FRAME;

  if (m_bbFrameConf->GetBbFrameUsageMode () == SatBbFrameConf::SHORT_FRAMES )
    {
      m_defaultBbFrameType = SatEnums::SHORT_FRAME;
    }

}

SatBbFrameContainer::~SatBbFrameContainer ()
{
  NS_LOG_FUNCTION (this);

  m_container.clear ();
}

TypeId
SatBbFrameContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatBbFrameContainer")
    .SetParent<Object> ()
    .AddConstructor<SatBbFrameContainer> ()
    .AddTraceSource ("BBFrameMergeTrace",
                     "Trace for merged BB Frames.",
                     MakeTraceSourceAccessor (&SatBbFrameContainer::m_bbFrameMergeTrace),
                     "ns3::SatBbFrame::BbFrameMergeCallback")
  ;
  return tid;
}

SatEnums::SatModcod_t
SatBbFrameContainer::GetModcod (uint32_t priorityClass, double cno)
{
  NS_LOG_FUNCTION (this);

  SatEnums::SatModcod_t modcod = m_bbFrameConf->GetDefaultModCod ();

  if (priorityClass == 0)
    {
      modcod = m_bbFrameConf->GetMostRobustModcod (m_defaultBbFrameType);
    }
  else if ( std::isnan (cno) == false )
    {
      modcod = m_bbFrameConf->GetBestModcod (cno, m_defaultBbFrameType);
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
      payloadBytes = m_bbFrameConf->GetBbFramePayloadBits (modcod, m_defaultBbFrameType) / SatConstVariables::BITS_PER_BYTE;
    }
  else
    {
      payloadBytes = m_bbFrameConf->GetBbFramePayloadBits (m_bbFrameConf->GetMostRobustModcod (m_defaultBbFrameType), m_defaultBbFrameType) / SatConstVariables::BITS_PER_BYTE;
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
          bytesLeft -= m_container.at (modcod).back ()->GetSpaceUsedInBytes ();
        }
    }
  else
    {
      if ( m_ctrlContainer.empty () != true )
        {
          bytesLeft -= m_ctrlContainer.back ()->GetSpaceUsedInBytes ();
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
      if ( m_container.at (modcod).empty ()
           || GetBytesLeftInTailFrame (priorityClass, modcod) < data->GetSize () )
        {
          CreateFrameToTail (priorityClass, modcod);
        }
      else if ( ( m_bbFrameConf->GetBbFrameUsageMode () == SatBbFrameConf::SHORT_AND_NORMAL_FRAMES )
                && ( m_container.at (modcod).back ()->GetFrameType () == SatEnums::SHORT_FRAME ) )
        {
          m_totalDuration += m_container.at (modcod).back ()->Extend (m_bbFrameConf);
        }

      m_container.at (modcod).back ()->AddPayload (data);

    }
  else
    {
      if ( m_ctrlContainer.empty ()
           || GetBytesLeftInTailFrame (priorityClass, modcod) < data->GetSize () )
        {
          // create and add frame to tail
          CreateFrameToTail (priorityClass, m_bbFrameConf->GetMostRobustModcod (m_defaultBbFrameType) );
        }
      else if ( ( m_bbFrameConf->GetBbFrameUsageMode () == SatBbFrameConf::SHORT_AND_NORMAL_FRAMES )
                && ( m_container.at (modcod).back ()->GetFrameType () == SatEnums::SHORT_FRAME ) )
        {
          m_totalDuration += m_ctrlContainer.back ()->Extend (m_bbFrameConf);
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
      nextFrame = m_ctrlContainer.front ();
      m_ctrlContainer.pop_front ();
      m_totalDuration -= nextFrame->GetDuration ();
    }
  else
    {
      std::vector< std::deque<Ptr<SatBbFrame> >* > nonEmptyQueues;

      for (FrameContainer_t::iterator it = m_container.begin (); it != m_container.end (); ++it )
        {
          if ( (*it).second.empty () == false )
            {
              nonEmptyQueues.push_back (&it->second);
            }
        }

      if ( nonEmptyQueues.empty () == false )
        {
          std::random_shuffle ( nonEmptyQueues.begin (), nonEmptyQueues.end ());

          nextFrame = (*nonEmptyQueues.begin ())->front ();
          (*nonEmptyQueues.begin ())->pop_front ();
          m_totalDuration -= nextFrame->GetDuration ();
        }
    }

  return nextFrame;
}

void
SatBbFrameContainer::CreateFrameToTail (uint32_t priorityClass, SatEnums::SatModcod_t modcod)
{
  NS_LOG_FUNCTION (this << modcod );

  Ptr<SatBbFrame> frame = Create<SatBbFrame> (modcod, m_defaultBbFrameType, m_bbFrameConf);

  if ( frame != NULL )
    {
      if ( priorityClass > 0)
        {
          m_container.at (modcod).push_back (frame);
        }
      else
        {
          m_ctrlContainer.push_back (frame);
        }

      m_totalDuration += frame->GetDuration ();
    }
  else
    {
      NS_FATAL_ERROR ("BB Frame creation failed!!!");
    }
}

void
SatBbFrameContainer::MergeBbFrames (double carrierBandwidthInHz)
{
  // go through all BB Frame containers from the most efficient to the robust
  for (FrameContainer_t::reverse_iterator itFromMerge = m_container.rbegin (); itFromMerge != m_container.rend (); itFromMerge++ )
    {
      // BB Frames currently exists in the BB Frame container for this MODCOD.
      if ( itFromMerge->second.empty () == false )
        {
          // Get occupancy i.e. ratio of used space to maximum space in buffer at the back of the list.
          // Occupancy is not necessarily efficiency.
          double occupancy = itFromMerge->second.back ()->GetOccupancy ();

          // GetBbFrameHighOccupancyThreshold () returns a configured parameter. Part of a high-low threshold hysteresis damper.
          // Current occupancy is no good. Need to off load the contents to some other BB Frame.
          if (occupancy < m_bbFrameConf->GetBbFrameHighOccupancyThreshold ())
            {
              // weighted occupancy takes into account the spectra efficiency of the current frame (MODCOD and frame length).
              double weightedOccupancy = itFromMerge->second.back ()->GetSpectralEfficiency (carrierBandwidthInHz) * occupancy;

              double maxNewOccupancyIfMerged = 0.0; // holder variable during a maximum value search
              Ptr<SatBbFrame> frameToMerge = NULL;  // holder variable for frame to potentially merge

              // check rest of the containers to find frame to merge.
              for ( FrameContainer_t::reverse_iterator itToMerge = ++FrameContainer_t::reverse_iterator (itFromMerge); itToMerge != m_container.rend (); itToMerge++ )
                {
                  // BB Frames currently exists in the BB Frame container for this MODCOD.
                  if (itToMerge->second.empty () == false)
                    {
                      /* check whether there is enough space in the frame */
                      // GetBbFrameLowOccupancyThreshold() returns a configured parameter. Part of a high-low threshold hysteresis damper.
                      // Current occupancy is no good. Need to fill in more.
                      double occupancy2 = itToMerge->second.back ()->GetOccupancy ();

                      if (occupancy2 < m_bbFrameConf->GetBbFrameLowOccupancyThreshold ())
                        {
                          Ptr<SatBbFrame> frame = itFromMerge->second.back ();

                          double newOccupancyIfMerged = itToMerge->second.back ()->GetOccupancyIfMerged (frame);

                          if (newOccupancyIfMerged > maxNewOccupancyIfMerged)
                            {
                              maxNewOccupancyIfMerged = newOccupancyIfMerged;
                              frameToMerge = itToMerge->second.back ();
                            }
                        }
                    }
                }

              // check control message container tail still, if it is not empty and MODCOD match
              // control messages are used default MODCOD
              if ( ( m_ctrlContainer.empty () == false ) && ( m_ctrlContainer.back ()->GetModcod () <= itFromMerge->first ) )
                {
                  if (m_ctrlContainer.back ()->GetOccupancy () < m_bbFrameConf->GetBbFrameLowOccupancyThreshold ())
                    {
                      double newOccupancyIfMerged = m_ctrlContainer.back ()->GetOccupancyIfMerged (itFromMerge->second.back ());

                      if (newOccupancyIfMerged > maxNewOccupancyIfMerged)
                        {
                          maxNewOccupancyIfMerged = newOccupancyIfMerged;
                          frameToMerge = m_ctrlContainer.back ();
                        }
                    }
                }

              // frame found where merging can be tried
              if ( frameToMerge )
                {
                  double newWeightedOccupancyIfMerged = frameToMerge->GetSpectralEfficiency (carrierBandwidthInHz) * maxNewOccupancyIfMerged;

                  if ( newWeightedOccupancyIfMerged > weightedOccupancy )
                    {
                      // Merge two frames

                      if ( frameToMerge->MergeWithFrame (itFromMerge->second.back (), m_bbFrameMergeTrace) )
                        {
                          m_totalDuration -= itFromMerge->second.back ()->GetDuration ();
                          itFromMerge->second.pop_back ();
                        }
                    }
                }
            }
        }
    }

  // if both short and normal frames are used then try to shrink normal frames
  // which are last ones in containers
  if ( m_bbFrameConf->GetBbFrameUsageMode () == SatBbFrameConf::SHORT_AND_NORMAL_FRAMES )
    {
      // go through all MODCOD based BB Frame containers and try to shrink last frame in the container
      for (FrameContainer_t::reverse_iterator it = m_container.rbegin (); it != m_container.rend (); it++ )
        {
          if ( it->second.empty () == false)
            {
              m_totalDuration -= it->second.back ()->Shrink (m_bbFrameConf);
            }
        }

      if ( m_ctrlContainer.empty () == false )
        {
          m_totalDuration -= m_ctrlContainer.back ()->Shrink (m_bbFrameConf);
        }
    }
}


} // namespace ns3
