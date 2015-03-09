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

#include "ns3/log.h"
#include "satellite-utils.h"
#include "satellite-bbframe.h"
#include "satellite-const-variables.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrame");

namespace ns3 {


SatBbFrame::SatBbFrame ()
  : m_modCod (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
    m_freeSpaceInBytes (0),
    m_maxSpaceInBytes (0),
    m_headerSizeInBytes (0),
    m_frameType (SatEnums::NORMAL_FRAME)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor of SatBbFrame not supported.");
}


SatBbFrame::SatBbFrame (SatEnums::SatModcod_t modCod, SatEnums::SatBbFrameType_t type, Ptr<SatBbFrameConf> conf)
  : m_modCod (modCod),
    m_frameType (type)
{
  NS_LOG_FUNCTION (this << modCod << type);

  switch (type)
    {
    case SatEnums::SHORT_FRAME:
    case SatEnums::NORMAL_FRAME:
      m_maxSpaceInBytes = (conf->GetBbFramePayloadBits (modCod, type) / SatConstVariables::BITS_PER_BYTE);
      m_headerSizeInBytes = conf->GetBbFrameHeaderSizeInBytes ();
      m_freeSpaceInBytes = m_maxSpaceInBytes - m_headerSizeInBytes;
      m_duration = conf->GetBbFrameDuration (modCod, type);
      break;

    case SatEnums::DUMMY_FRAME:
      /**
       * Dummy frame is assumed to be a short frame but with no valid data.
       */
      m_maxSpaceInBytes = conf->GetBbFramePayloadBits (modCod, SatEnums::SHORT_FRAME) / SatConstVariables::BITS_PER_BYTE;
      m_headerSizeInBytes = conf->GetBbFrameHeaderSizeInBytes ();
      m_freeSpaceInBytes = m_maxSpaceInBytes - m_headerSizeInBytes;
      m_duration = conf->GetDummyBbFrameDuration ();
      break;

    default:
      NS_FATAL_ERROR ("Invalid BBFrame type!!!");
      break;
    }
}

SatBbFrame::~SatBbFrame ()
{
  NS_LOG_FUNCTION (this);
}

const SatBbFrame::SatBbFramePayload_t&
SatBbFrame::GetPayload ()
{
  NS_LOG_FUNCTION (this);
  return m_framePayload;
}

uint32_t
SatBbFrame::AddPayload (Ptr<Packet> data)
{
  NS_LOG_FUNCTION (this);

  uint32_t dataLengthInBytes = data->GetSize ();

  if ( dataLengthInBytes <= m_freeSpaceInBytes )
    {
      m_framePayload.push_back (data);
      m_freeSpaceInBytes -= dataLengthInBytes;
    }
  else
    {
      NS_FATAL_ERROR ("Data cannot be added to BB frame (length, free space): " << dataLengthInBytes << ", " << m_freeSpaceInBytes);
    }

  return GetSpaceLeftInBytes ();
}

uint32_t
SatBbFrame::GetSpaceLeftInBytes () const
{
  NS_LOG_FUNCTION (this);
  return m_freeSpaceInBytes;
}

uint32_t
SatBbFrame::GetSpaceUsedInBytes () const
{
  NS_LOG_FUNCTION (this);
  return (m_maxSpaceInBytes - m_freeSpaceInBytes);
}

uint32_t
SatBbFrame::GetMaxSpaceInBytes () const
{
  NS_LOG_FUNCTION (this);
  return m_maxSpaceInBytes;
}

double
SatBbFrame::GetOccupancy () const
{
  NS_LOG_FUNCTION (this);
  return ( (double) GetSpaceUsedInBytes () / (double) m_maxSpaceInBytes);
}

double
SatBbFrame::GetOccupancyIfMerged (Ptr<SatBbFrame> mergedFrame) const
{
  NS_LOG_FUNCTION (this);

  double ifMergedOccupancy = 0.0;

  uint32_t mergedFrameDataBytes = mergedFrame->GetSpaceUsedInBytes () - mergedFrame->GetFrameHeaderSize ();

  if ( mergedFrameDataBytes <= m_freeSpaceInBytes )
    {
      ifMergedOccupancy = ((double) GetSpaceUsedInBytes () + (double) mergedFrameDataBytes) / (double) m_maxSpaceInBytes;
    }

  return ifMergedOccupancy;
}

double
SatBbFrame::GetSpectralEfficiency (double carrierBandwidthInHz) const
{
  NS_LOG_FUNCTION (this << carrierBandwidthInHz);

  return ( (double) (SatConstVariables::BITS_PER_BYTE * m_maxSpaceInBytes) / m_duration.GetSeconds () / carrierBandwidthInHz);
}

bool
SatBbFrame::MergeWithFrame (Ptr<SatBbFrame> mergedFrame, TracedCallback<Ptr<SatBbFrame>, Ptr<SatBbFrame> > mergeTraceCb)
{
  NS_LOG_FUNCTION (this);

  bool merged = false;

  uint32_t dataBytes = mergedFrame->GetSpaceUsedInBytes () - mergedFrame->GetFrameHeaderSize ();

  if ( dataBytes <= m_freeSpaceInBytes )
    {
      mergeTraceCb (this, mergedFrame);
      m_framePayload.insert ( m_framePayload.end (), mergedFrame->GetPayload ().begin (), mergedFrame->GetPayload ().end () );
      m_freeSpaceInBytes -= dataBytes;
      merged = true;
    }

  return merged;
}

Time SatBbFrame::Shrink (Ptr<SatBbFrameConf> conf)
{
  NS_LOG_FUNCTION (this);

  Time durationDecrease (0);

  if ( m_frameType == SatEnums::NORMAL_FRAME )
    {
      uint32_t maxShortFrameSpaceInBytes = (conf->GetBbFramePayloadBits (m_modCod, SatEnums::SHORT_FRAME) / SatConstVariables::BITS_PER_BYTE);
      uint32_t spaceUsedInbytes = GetSpaceUsedInBytes ();

      // shrink only if data used in normal frame can fit in short frame
      if ( spaceUsedInbytes < maxShortFrameSpaceInBytes)
        {
          m_frameType = SatEnums::SHORT_FRAME;
          m_maxSpaceInBytes = maxShortFrameSpaceInBytes - m_headerSizeInBytes;
          m_freeSpaceInBytes = m_maxSpaceInBytes - spaceUsedInbytes;

          Time oldDuration = m_duration;
          m_duration = conf->GetBbFrameDuration (m_modCod, SatEnums::SHORT_FRAME);
          durationDecrease = oldDuration - m_duration;
        }
    }

  return durationDecrease;
}

Time SatBbFrame::Extend (Ptr<SatBbFrameConf> conf)
{
  NS_LOG_FUNCTION (this);

  Time durationIncrease (0);

  if ( m_frameType == SatEnums::SHORT_FRAME )
    {
      uint32_t spaceUsedInbytes = GetSpaceUsedInBytes ();

      m_frameType = SatEnums::NORMAL_FRAME;
      m_maxSpaceInBytes = (conf->GetBbFramePayloadBits (m_modCod, SatEnums::NORMAL_FRAME) / SatConstVariables::BITS_PER_BYTE);
      m_freeSpaceInBytes = m_maxSpaceInBytes - spaceUsedInbytes;

      Time oldDuration = m_duration;
      m_duration = conf->GetBbFrameDuration (m_modCod, SatEnums::NORMAL_FRAME);
      durationIncrease = m_duration - oldDuration;
    }

  return durationIncrease;
}


} // namespace ns3
