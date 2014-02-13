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
#include "satellite-dama-entry.h"

NS_LOG_COMPONENT_DEFINE ("SatDamaEntry");

namespace ns3 {


SatDamaEntry::SatDamaEntry ()
  : m_dynamicRatePersistence (0),
    m_volumeBacklogPersistence (0)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("The default version of the constructor not supported!!!");
}

SatDamaEntry::SatDamaEntry (Ptr<SatLowerLayerServiceConf> llsConf)
 : m_llsConf (llsConf)
{
  NS_LOG_FUNCTION (this);

  ResetDynamicRatePersistence ();
  ResetVolumeBacklogPersistence ();
}

SatDamaEntry::~SatDamaEntry ()
{
  NS_LOG_FUNCTION (this);
}

void
SatDamaEntry::ResetDynamicRatePersistence ()
{
  NS_LOG_FUNCTION (this);

  m_dynamicRatePersistence = m_llsConf->GetDynamicRatePersistence ();
}

void
SatDamaEntry::DecrementDynamicRatePersistence ()
{
  NS_LOG_FUNCTION (this);

  if ( m_dynamicRatePersistence > 0)
    {
      m_dynamicRatePersistence--;
    }
}

void
SatDamaEntry::ResetVolumeBacklogPersistence ()
{
  NS_LOG_FUNCTION (this);

  m_volumeBacklogPersistence = m_llsConf->GetVolumeBacklogPersistence ();
}

void
SatDamaEntry::DecrementVolumeBacklogPersistence ()
{
  NS_LOG_FUNCTION (this);

  if ( m_volumeBacklogPersistence > 0)
      {
      m_volumeBacklogPersistence--;
      }
}

} // namespace ns3
