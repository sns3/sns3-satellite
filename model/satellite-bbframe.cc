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
#include "satellite-bbframe.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrame");

namespace ns3 {


SatBbFrame::SatBbFrame ()
 : m_modCod (3),
   m_freeBytes (m_shortBbFrameLengthInBytes),
   m_TotalBytes (m_shortBbFrameLengthInBytes),
   m_containsControlData (false),
   m_duration(Seconds (0.00009))

{
  NS_LOG_FUNCTION (this);
}


SatBbFrame::SatBbFrame (uint32_t modCod, FrameType_t type)
  :m_modCod (modCod),
   m_containsControlData (false)
{
  NS_LOG_FUNCTION (this << modCod << type);

  switch (type)
  {
    case SatBbFrame::SHORT_FRAME:
      m_freeBytes = m_shortBbFrameLengthInBytes;
      m_TotalBytes = m_shortBbFrameLengthInBytes;
      m_duration = Seconds (0.00009);
      break;

    case SatBbFrame::NORMAL_FRAME:
      m_freeBytes = m_normalBbFrameLengthInBytes;
      m_TotalBytes = m_normalBbFrameLengthInBytes;
      m_duration = Seconds (0.00036);
      break;

    case SatBbFrame::DUMMY_FRAME:
      m_freeBytes = m_shortBbFrameLengthInBytes;
      m_TotalBytes = m_shortBbFrameLengthInBytes;
      m_duration = Seconds (0.00009);
      break;

    default:
      NS_FATAL_ERROR ("Invalid BBFrame type!!!");
      break;

  }

  m_freeBytes = m_TotalBytes;
}

SatBbFrame::~SatBbFrame ()
{
  NS_LOG_FUNCTION (this);
}

const SatBbFrame::SatBbFrameData&
SatBbFrame::GetTransmitData ()
{
  NS_LOG_FUNCTION (this);
  return frameData;
}

uint32_t
SatBbFrame::AddTransmitData (Ptr<Packet> data, bool controlData)
{
  NS_LOG_FUNCTION (this);

  uint32_t dataLengthInBytes = data->GetSize ();

  if ( dataLengthInBytes <= m_freeBytes )
    {
      frameData.push_back (data);
      m_freeBytes -= dataLengthInBytes;
      m_containsControlData |= controlData;
    }

  return GetBytesLeft();
}

bool
SatBbFrame::ContainsControlData () const
{
  NS_LOG_FUNCTION (this);
  return m_containsControlData;
}

uint32_t
SatBbFrame::GetBytesLeft () const
{
  // TODO: Needed calculate according to ModCod
  return m_freeBytes;
}


} // namespace ns3
