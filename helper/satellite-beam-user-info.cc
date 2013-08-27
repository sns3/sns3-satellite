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

#include "ns3/assert.h"
#include "ns3/log.h"
#include "satellite-beam-user-info.h"

NS_LOG_COMPONENT_DEFINE ("SatBeamUserInfo");

namespace ns3 {

SatBeamUserInfo::SatBeamUserInfo()
{

}

SatBeamUserInfo::SatBeamUserInfo (uint32_t utCount, uint32_t userCount)
{
  NS_ASSERT(utCount > 0);
  NS_ASSERT(userCount > 0);

  for (uint32_t i = 0; i < utCount; i++)
    {
      m_userCount.push_back(userCount);
    }
}

uint32_t
SatBeamUserInfo::GetUtCount ()
{
  return m_userCount.size();
}

uint32_t
SatBeamUserInfo::GetUtUserCount (uint32_t utIndex)
{
  NS_ASSERT(utIndex < m_userCount.size());

  return m_userCount[utIndex];
}

void SatBeamUserInfo::SetUtUserCount (uint32_t utIndex, uint32_t userCount)
{
  NS_ASSERT(utIndex < m_userCount.size());

  m_userCount[utIndex] = userCount;
}

void SatBeamUserInfo::AddUt (uint32_t userCount)
{
  NS_ASSERT(userCount > 0);

  m_userCount.push_back(userCount);
}

} // namespace ns3
