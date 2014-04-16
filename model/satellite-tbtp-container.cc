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

#include "ns3/mac48-address.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

#include "satellite-tbtp-container.h"
#include "satellite-control-message.h"

NS_LOG_COMPONENT_DEFINE ("SatTbtpContainer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatTbtpContainer);

TypeId
SatTbtpContainer::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatTbtpContainer")
      .SetParent<Object> ()
      .AddConstructor<SatTbtpContainer>()
      .AddAttribute ("MaxStoredTbtps",
                     "Maximum amount of stored TBTPs",
                     UintegerValue (100),
                     MakeUintegerAccessor (&SatTbtpContainer::m_maxStoredTbtps),
                     MakeUintegerChecker<uint32_t> ())
    ;
    return tid;
}

SatTbtpContainer::SatTbtpContainer ()
:m_address (),
 m_maxStoredTbtps (100),
 m_rcvdTbtps (0),
 m_superFrameDuration (0)
{
  NS_FATAL_ERROR ("SatTbtpContainer::SatTbtpContainer - Constructor not in use");
}

SatTbtpContainer::SatTbtpContainer (Time superFrameDuration)
:m_address (),
 m_maxStoredTbtps (100),
 m_rcvdTbtps (0),
 m_superFrameDuration (superFrameDuration)
{

}

SatTbtpContainer::~SatTbtpContainer ()
{

}

void SatTbtpContainer::DoDispose ()
{
  m_tbtps.clear ();
  Object::DoDispose ();
}


void
SatTbtpContainer::SetMacAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (this);
  m_address = address;
}

void
SatTbtpContainer::Add (Time startTime, Ptr<SatTbtpMessage> tbtp)
{
  NS_LOG_FUNCTION (this << startTime.GetSeconds ());

  ++m_rcvdTbtps;

  m_tbtps.insert (std::make_pair<Time, Ptr<SatTbtpMessage> > (startTime, tbtp));

  // If there are too many TBTPs in the container, erase the first
  if (m_tbtps.size () > m_maxStoredTbtps)
    {
      m_tbtps.erase (m_tbtps.begin ());
    }
}

void
SatTbtpContainer::RemovePastTbtps ()
{
  NS_LOG_FUNCTION (this);

  for (TbtpMap_t::iterator it = m_tbtps.begin (); it != m_tbtps.end (); )
    {
      if ((it->first + m_superFrameDuration) < Now ())
        {
          m_tbtps.erase (it++);
        }
      else
        {
          ++it;
        }
    }
}

bool
SatTbtpContainer::HasScheduledTimeSlots ()
{
  NS_LOG_FUNCTION (this);

  bool hasScheduledTimeSlots = false;

  if (!m_tbtps.empty ())
    {
      RemovePastTbtps ();

      SatTbtpMessage::DaTimeSlotInfoContainer_t slots;
      for (TbtpMap_t::const_reverse_iterator it = m_tbtps.rbegin ();
          it != m_tbtps.rend ();
          ++it)
        {
          slots = it->second->GetDaTimeslots (m_address);
          if (!slots.empty ())
            {
              hasScheduledTimeSlots = true;
              break;
            }
        }
    }
  return hasScheduledTimeSlots;
}


} // namespace


