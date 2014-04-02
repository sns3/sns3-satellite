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

NS_LOG_COMPONENT_DEFINE ("SatBbFrame");

namespace ns3 {


SatBbFrame::SatBbFrame ()
 : m_modCod (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
   m_freeSpaceInBytes (0),
   m_maxSpaceInBytes (0),
   m_containsControlPdu (false),
   m_frameType ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor of SatBbFrame not supported.");
}


SatBbFrame::SatBbFrame (SatEnums::SatModcod_t modCod, SatEnums::SatBbFrameType_t type, Ptr<SatBbFrameConf> conf)
  :m_modCod (modCod),
   m_containsControlPdu (false),
   m_frameType (type)
{
  NS_LOG_FUNCTION (this << modCod << type);

  switch (type)
  {
    case SatEnums::SHORT_FRAME:
    case SatEnums::NORMAL_FRAME:
      m_maxSpaceInBytes = conf->GetBbFramePayloadBits (modCod, type) / SatUtils::BITS_PER_BYTE;
      m_freeSpaceInBytes = m_maxSpaceInBytes - conf->GetBbFrameHeaderSizeInBytes ();
      m_duration = conf->GetBbFrameDuration (modCod, type);
      break;

    case SatEnums::DUMMY_FRAME:
      // TODO: now we use given MODCOD and short frame. Configuration needed if normal frame is wanted to use.
      m_maxSpaceInBytes = conf->GetBbFramePayloadBits (modCod, SatEnums::SHORT_FRAME) / SatUtils::BITS_PER_BYTE;
      m_freeSpaceInBytes = m_maxSpaceInBytes - conf->GetBbFrameHeaderSizeInBytes ();
      m_duration = conf->GetDummyBbFrameDuration ();
      break;

    default:
      NS_FATAL_ERROR ("Invalid BBFrame type!!!");
      break;
  }

  m_freeSpaceInBytes = m_maxSpaceInBytes;
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
      std::cout << dataLengthInBytes << std::endl;
      NS_ASSERT (0);
    }

  return GetSpaceLeftInBytes();
}

uint32_t
SatBbFrame::GetSpaceLeftInBytes () const
{
  NS_LOG_FUNCTION (this);
  return m_freeSpaceInBytes;
}

uint32_t
SatBbFrame::GetMaxSpaceInBytes () const
{
  NS_LOG_FUNCTION (this);
  return m_maxSpaceInBytes;
}

} // namespace ns3
