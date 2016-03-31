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
#include "satellite-const-variables.h"
#include "satellite-utils.h"
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
  : m_dynamicRatePersistence (0),
    m_volumeBacklogPersistence (0),
    m_llsConf (llsConf)
{
  NS_LOG_FUNCTION (this);

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
          totalBytes += (SatConstVariables::BITS_IN_KBIT * m_llsConf->GetDaConstantServiceRateInKbps (i) * duration.GetSeconds ()) / (double)(SatConstVariables::BITS_PER_BYTE);
        }
    }

  return totalBytes;
}

uint32_t
SatDamaEntry::GetMinRateBasedBytes (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);

  uint32_t totalBytes = 0;

  for ( uint8_t i = 0; i < m_llsConf->GetDaServiceCount (); i++)
    {
      uint16_t minRateInKbps = 0;

      if ( m_llsConf->GetDaConstantAssignmentProvided (i) && m_llsConf->GetDaRbdcAllowed (i) )
        {
          minRateInKbps = std::max<uint16_t> (m_llsConf->GetDaMinimumServiceRateInKbps (i),  m_llsConf->GetDaConstantServiceRateInKbps (i) );
        }
      else if (m_llsConf->GetDaConstantAssignmentProvided (i))
        {
          minRateInKbps = m_llsConf->GetDaConstantServiceRateInKbps (i);
        }
      else if (m_llsConf->GetDaRbdcAllowed (i))
        {
          minRateInKbps = m_llsConf->GetDaMinimumServiceRateInKbps (i);
        }

      totalBytes += (SatConstVariables::BITS_IN_KBIT * minRateInKbps * duration.GetSeconds ()) / (double)(SatConstVariables::BITS_PER_BYTE);
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
      totalBytes += (SatConstVariables::BITS_IN_KBIT * m_dynamicRateRequestedInKbps[i] * duration.GetSeconds ()) / (double)(SatConstVariables::BITS_PER_BYTE);
    }

  return totalBytes;
}

uint32_t
SatDamaEntry::GetVbdcBasedBytes () const
{
  NS_LOG_FUNCTION (this);

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
  NS_LOG_FUNCTION (this << (uint32_t) index);

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
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if ( index >= m_llsConf->GetDaServiceCount ())
    {
      NS_FATAL_ERROR ("RC index requested is out of range!!!");
    }

  uint16_t minRbdc (0);

  if (m_llsConf->GetDaRbdcAllowed (index) && (m_dynamicRatePersistence > 0) )
    {
      minRbdc = std::max<uint16_t> (m_llsConf->GetDaMinimumServiceRateInKbps (index),  GetCraInKbps (index) );
    }

  return minRbdc;
}

uint16_t
SatDamaEntry::GetRbdcInKbps (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if ( index >= m_dynamicRateRequestedInKbps.size ())
    {
      NS_FATAL_ERROR ("RC index requested is out of range!!!");
    }

  return m_dynamicRateRequestedInKbps[index];
}

void
SatDamaEntry::UpdateRbdcInKbps (uint8_t index, uint16_t rateInKbps)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << rateInKbps);

  if ( m_llsConf->GetDaRbdcAllowed (index) )
    {
      double craRbdcSum = GetCraInKbps (index) + rateInKbps;

      if (craRbdcSum < GetMinRbdcInKbps (index) )
        {
          m_dynamicRateRequestedInKbps[index] = GetMinRbdcInKbps (index) - GetCraInKbps (index);
        }
      else if (craRbdcSum > m_llsConf->GetDaMaximumServiceRateInKbps (index))
        {
          m_dynamicRateRequestedInKbps[index] = std::max<double> ( 0.0, (m_llsConf->GetDaMaximumServiceRateInKbps (index) - m_llsConf->GetDaConstantServiceRateInKbps (index)));
        }
      else
        {
          m_dynamicRateRequestedInKbps[index] = rateInKbps;
        }
    }
}

uint32_t
SatDamaEntry::GetVbdcInBytes (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if ( index >= m_volumeBacklogRequestedInBytes.size ())
    {
      NS_FATAL_ERROR ("RC index requested is out of range!!!");
    }

  return m_volumeBacklogRequestedInBytes[index];
}

void
SatDamaEntry::UpdateVbdcInBytes (uint8_t index, uint32_t volumeInBytes)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << volumeInBytes);

  if ( m_llsConf->GetDaVolumeAllowed (index) )
    {
      NS_LOG_INFO ("Update VBDC! RC index: " << index <<
                    " existing VBDC bytes: " << m_volumeBacklogRequestedInBytes[index] <<
                    " updated with " << volumeInBytes << " bytes!");

      SetVbdcInBytes (index, m_volumeBacklogRequestedInBytes[index] + volumeInBytes);
    }
}

void
SatDamaEntry::SetVbdcInBytes (uint8_t index, uint32_t volumeInBytes)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << volumeInBytes);

  if ( m_llsConf->GetDaVolumeAllowed (index) )
    {
      NS_LOG_INFO ("Set VBDC bytes to " << volumeInBytes << " for RC index: " << index);

      m_volumeBacklogRequestedInBytes[index] = volumeInBytes;

      if ( m_volumeBacklogRequestedInBytes[index] > (SatConstVariables::BYTES_IN_KBYTE * m_llsConf->GetDaMaximumBacklogInKbytes (index)))
        {
          uint32_t maxVolumeBacklogInBytes = SatConstVariables::BYTES_IN_KBYTE * m_llsConf->GetDaMaximumBacklogInKbytes (index);
          NS_LOG_INFO ("Max volume backlog reached! Set VBDC bytes to " << maxVolumeBacklogInBytes << " for RC index: " << index);
          m_volumeBacklogRequestedInBytes[index] = maxVolumeBacklogInBytes;
        }
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
