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

  m_dynamicRateRequestedInKbps = std::vector<uint16_t> (m_llsConf->GetDaServiceCount (), 0.0);
  m_volumeBacklogRequestedInBytes = std::vector<uint32_t> (m_llsConf->GetDaServiceCount (), 0);
}

SatDamaEntry::~SatDamaEntry ()
{
  NS_LOG_FUNCTION (this);
}

uint8_t
SatDamaEntry::GetRcCount () const
{
  NS_LOG_FUNCTION (this);

  return m_llsConf->GetDaServiceCount ();
}

uint32_t
SatDamaEntry::GetCraBasedBytes (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);

  uint32_t totalBytes = 0;

  for ( uint8_t i = 0; i < m_llsConf->GetDaServiceCount (); i++)
    {
      if (m_llsConf->GetDaConstantAssignmentProvided (i))
        {
          totalBytes += (1000.0 * m_llsConf->GetDaConstantServiceRateInKbps (i) * duration.GetSeconds ()) / 8;
        }
    }

  return totalBytes;
}

uint32_t
SatDamaEntry::GetMinRbdcBasedBytes (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);

  uint32_t totalBytes = 0;

  for ( uint8_t i = 0; i < m_llsConf->GetDaServiceCount (); i++)
    {
      uint16_t minRateInKbps = m_llsConf->GetDaMinimumServiceRateInKbps (i) - m_llsConf->GetDaConstantServiceRateInKbps (i);
      totalBytes += (1000.0 * minRateInKbps * duration.GetSeconds ()) / 8;
    }

  return totalBytes;
}

uint32_t
SatDamaEntry::GetRbdcBasedBytes (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);

  uint32_t totalBytes = 0;

  for ( uint8_t i = 0; i < m_dynamicRateRequestedInKbps.size (); i++)
    {
      totalBytes += (1000.0 * m_dynamicRateRequestedInKbps[i] * duration.GetSeconds ()) / 8;
    }

  return totalBytes;
}

uint32_t
SatDamaEntry::GetVbdcBasedBytes () const
{
  uint32_t totalBytes = 0;

  for ( uint8_t i = 0; i < m_volumeBacklogRequestedInBytes.size (); i++)
    {
      totalBytes +=  m_volumeBacklogRequestedInBytes[i];
    }

  return totalBytes;
}

uint16_t
SatDamaEntry::GetCraInKbps (uint8_t index) const
{
  NS_LOG_FUNCTION (this);

  if ( index >= m_llsConf->GetDaServiceCount ())
    {
       NS_FATAL_ERROR ("RC index requested is out of range!!!");
    }

  uint16_t cra (0);
  if (m_llsConf->GetDaConstantAssignmentProvided (index))
    {
      cra = m_llsConf->GetDaConstantServiceRateInKbps (index);
    }

  return cra;
}

uint16_t
SatDamaEntry::GetMinRbdcInKbps (uint8_t index) const
{
  NS_LOG_FUNCTION (this);

  if ( index >= m_llsConf->GetDaServiceCount ())
    {
       NS_FATAL_ERROR ("RC index requested is out of range!!!");
    }

  uint16_t minRbdc (0);
  if (m_llsConf->GetDaRbdcAllowed (index))
    {
      minRbdc = m_llsConf->GetDaMinimumServiceRateInKbps (index);
    }

  return minRbdc;
}

uint16_t
SatDamaEntry::GetRbdcInKbps (uint8_t index) const
{
  NS_LOG_FUNCTION (this);

  if ( index >= m_dynamicRateRequestedInKbps.size ())
    {
       NS_FATAL_ERROR ("RC index requested is out of range!!!");
    }

  return m_dynamicRateRequestedInKbps[index];
}

void
SatDamaEntry::UpdateRbdcInKbps (uint8_t index, uint16_t rateInKbps)
{
  NS_LOG_FUNCTION (this);

  if ( m_llsConf->GetDaRbdcAllowed (index) )
    {
      double craRbdcSum = m_llsConf->GetDaConstantServiceRateInKbps (index) + rateInKbps;

      if (craRbdcSum < m_llsConf->GetDaMinimumServiceRateInKbps (index) )
        {
          m_dynamicRateRequestedInKbps[index] = m_llsConf->GetDaMinimumServiceRateInKbps (index) - m_llsConf->GetDaConstantServiceRateInKbps (index);
        }
      else if (craRbdcSum > m_llsConf->GetDaMaximumServiceRateInKbps (index))
        {
          m_dynamicRateRequestedInKbps[index] = std::max<double> ( 0.0, (m_llsConf->GetDaMaximumServiceRateInKbps (index) - m_llsConf->GetDaConstantServiceRateInKbps (index)));
        }
      else
        {
          m_dynamicRateRequestedInKbps[index] = rateInKbps;
        }

      ResetDynamicRatePersistence ();
    }
}

uint32_t
SatDamaEntry::GetVbdcInBytes (uint8_t index) const
{
  NS_LOG_FUNCTION (this);

  if ( index >= m_volumeBacklogRequestedInBytes.size ())
    {
       NS_FATAL_ERROR ("RC index requested is out of range!!!");
    }

  return m_volumeBacklogRequestedInBytes[index];
}

void
SatDamaEntry::UpdateVbdcInBytes (uint8_t index, uint32_t volumeInBytes)
{
  NS_LOG_FUNCTION (this);

  if ( m_llsConf->GetDaVolumeAllowed (index) )
    {
      SetVbdcInBytes (index, m_volumeBacklogRequestedInBytes[index] + volumeInBytes);
    }
}

void
SatDamaEntry::SetVbdcInBytes (uint8_t index, uint32_t volumeInBytes)
{
  NS_LOG_FUNCTION (this);

  if ( m_llsConf->GetDaVolumeAllowed (index) )
    {
      m_volumeBacklogRequestedInBytes[index] = volumeInBytes;

      if ( m_volumeBacklogRequestedInBytes[index] > (1024 * m_llsConf->GetDaMaximumBacklogInKbytes (index)))
        {
          m_volumeBacklogRequestedInBytes[index] = (1024 * m_llsConf->GetDaMaximumBacklogInKbytes (index));
        }

      ResetVolumeBacklogPersistence ();
    }
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

  if (m_dynamicRatePersistence == 0)
    {
      std::fill (m_dynamicRateRequestedInKbps.begin (), m_dynamicRateRequestedInKbps.end (), 0.0);
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

  if (m_volumeBacklogPersistence == 0)
      {
        std::fill (m_volumeBacklogRequestedInBytes.begin (), m_volumeBacklogRequestedInBytes.end (), 0);
      }
}

} // namespace ns3
