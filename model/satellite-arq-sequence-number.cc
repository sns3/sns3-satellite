/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */


#include "ns3/log.h"
#include "satellite-arq-sequence-number.h"

NS_LOG_COMPONENT_DEFINE ("SatArqSequenceNumber");

namespace ns3 {


SatArqSequenceNumber::SatArqSequenceNumber ()
  : m_seqNoMap (),
    m_currSeqNo (-1),
    m_windowSize (0),
    m_maxSn (std::numeric_limits<uint8_t>::max ())
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * Default constructor is not meant to be used!
   */
}

SatArqSequenceNumber::SatArqSequenceNumber (uint8_t windowSize)
  : m_seqNoMap (),
    m_currSeqNo (-1),
    m_windowSize (windowSize),
    m_maxSn (std::numeric_limits<uint8_t>::max ())
{
  NS_LOG_FUNCTION (this << (uint32_t) windowSize );
}

bool
SatArqSequenceNumber::SeqNoAvailable () const
{
  NS_LOG_FUNCTION (this);
  return (m_seqNoMap.size () < m_windowSize);
}


uint8_t
SatArqSequenceNumber::NextSequenceNumber ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (SeqNoAvailable ());

  m_currSeqNo++;
  uint8_t sn = uint8_t (m_currSeqNo % m_maxSn);
  m_seqNoMap[m_currSeqNo] = false;

  return sn;
}

void
SatArqSequenceNumber::Release (uint8_t seqNo)
{
  NS_LOG_FUNCTION (this << (uint32_t) seqNo);

  uint32_t factor = uint32_t (m_currSeqNo / m_maxSn);
  uint32_t mod = uint32_t (m_currSeqNo % m_maxSn);
  uint32_t sn;

  // Same seqNo window
  if (seqNo <= mod)
    {
      sn = factor * m_maxSn + seqNo;
    }
  // Different seqNo window
  else
    {
      sn = (factor - 1) * m_maxSn + seqNo;
    }

  m_seqNoMap[sn] = true;

  CleanUp ();
}

void
SatArqSequenceNumber::CleanUp ()
{
  NS_LOG_FUNCTION (this);

  std::map<uint32_t, bool>::iterator it = m_seqNoMap.begin ();

  while (it != m_seqNoMap.end () && it->second == true)
    {
      m_seqNoMap.erase (it);
      it = m_seqNoMap.begin ();
    }
}

} // namespace
