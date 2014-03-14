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

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "satellite-lower-layer-service.h"

NS_LOG_COMPONENT_DEFINE ("SatLowerLayerServ");

namespace ns3 {

 SatLowerLayerServiceDaEntry::SatLowerLayerServiceDaEntry ()
  : m_constantAssignmentProvided (false),
    m_rbdcAllowed (false),
    m_volumeAllowed (false),
    m_constantServiceRateStream (0),
    m_maximumServiceRateKbps (0.0),
    m_minimumServiceRateKbps (0.0),
    m_maximumBacklogInKbytes (0)
{
   NS_LOG_FUNCTION (this);
}

SatLowerLayerServiceDaEntry::~SatLowerLayerServiceDaEntry ()
{
   NS_LOG_FUNCTION (this);
}

SatLowerLayerServiceRaEntry::SatLowerLayerServiceRaEntry ()
: m_maxUniquePayloadPerBlock (0),
  m_maxConsecutiveBlockAccessed (0),
  m_minimumIdleBlock (0)
{
   NS_LOG_FUNCTION (this);
}

SatLowerLayerServiceRaEntry::~SatLowerLayerServiceRaEntry ()
{
   NS_LOG_FUNCTION (this);
}

NS_OBJECT_ENSURE_REGISTERED (SatLowerLayerServiceConf);

SatLowerLayerServiceConf::SatLowerLayerServiceConf ()
 : m_dynamicRatePersistence (0),
   m_volumeBacklogPersistence (0),
   m_defaultControlRandomizationInterval (0.0),
   m_daServiceEntryCount (0),
   m_raServiceEntryCount (0)
{
   NS_LOG_FUNCTION (this);

   NS_ASSERT ( m_minRaServiceEntries <= m_maxRaServiceEntries);
   NS_ASSERT ( m_minDaServiceEntries <= m_maxDaServiceEntries);
}

SatLowerLayerServiceConf::~SatLowerLayerServiceConf ()
{
   NS_LOG_FUNCTION (this);
}

std::string
SatLowerLayerServiceConf::GetIndexAsDaServiceName (uint8_t index)
{
  std::string name = "DaService";
  return name + GetNumberAsString<uint8_t> (index);
}

std::string
SatLowerLayerServiceConf::GetIndexAsRaServiceName (uint8_t index)
{
  std::string name = "RaService";
  return name + GetNumberAsString<uint8_t> (index);
}

/**
 * SAT_ADD_DA_SERVICE_ATTRIBUTES macro helps defining service specific attributes
 * for SatLowerLayerServiceConf in method GetTypeId.
 *
 * \param index Index of the service which attributes are added to configuration.
 * \param a1    'Constant assignment provided' attribute value [true or false]
 * \param a2    'RBDC allowed' attribute value [true or false]
 * \param a3    'Volume allowed' attribute value [true or false]
 * \param a4    'Constant service rate' attribute value [KBps]
 * \param a5    'Maximum service rate' attribute value [KBps]
 * \param a6    'Minimum service rate' attribute value [KBps]
 * \param a7    'Maximum backlog size' attribute value [KBytes]
 *
 * \return TypeId
 */
#define SAT_ADD_DA_SERVICE_ATTRIBUTES(index, a1, a2, a3, a4, a5, a6, a7) \
   AddAttribute ( GetIndexAsDaServiceName (index) + "_ConstantAssignmentProvided", \
                  "Constant Assignment provided for " + GetIndexAsDaServiceName (index), \
                  BooleanValue (a1), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## ConstantAssignmentProvided, \
                                       &SatLowerLayerServiceConf::GetDaServ ## index ## ConstantAssignmentProvided), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) + "_RbdcAllowed", \
                  "RBDC allowed for DA " + GetIndexAsDaServiceName (index), \
                  BooleanValue (a2), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## RbdcAllowed, \
                                       &SatLowerLayerServiceConf::GetDaServ ## index ## RbdcAllowed), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) + "_VolumeAllowed", \
                  "Volume allowed for DA " + GetIndexAsDaServiceName (index), \
                  BooleanValue (a3), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## VolumeAllowed, \
                                       &SatLowerLayerServiceConf::GetDaServ ## index ## VolumeAllowed), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) + "_ConstantServiceRate", \
                  "A RandomVariableStream for constant service rate [kbps] for DA " + GetIndexAsDaServiceName (index), \
                  StringValue ("ns3::ConstantRandomVariable[Constant=" + GetNumberAsString<uint16_t> (a4) + "]"), \
                  MakePointerAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## ConstantServiceRateStream, \
                                       &SatLowerLayerServiceConf::GetDaServ ## index ## ConstantServiceRateStream), \
                  MakePointerChecker<RandomVariableStream> ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) + "_MaximumServiceRate", \
                  "Maximum service rate [kbps] for DA " + GetIndexAsDaServiceName (index), \
                  UintegerValue (a5), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## MaximumServiceRateInKbps, \
                                        &SatLowerLayerServiceConf::GetDaServ ## index ## MaximumServiceRateInKbps), \
                  MakeUintegerChecker<uint16_t> ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) +  "_MinimumServiceRate", \
                  "Minimum service rate [kbps] for DA " + GetIndexAsDaServiceName (index), \
                  UintegerValue (a6), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## MinimumServiceRateInKbps, \
                                        &SatLowerLayerServiceConf::GetDaServ ## index ## MinimumServiceRateInKbps), \
                  MakeUintegerChecker<uint16_t> ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) +  "_MaximumBacklogSize", \
                  "Maximum backlog size [bytes] for DA " + GetIndexAsDaServiceName (index), \
                  UintegerValue (a7), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## MaximumBacklogInKbytes, \
                                        &SatLowerLayerServiceConf::GetDaServ ## index ## MaximumBacklogInKbytes), \
                  MakeUintegerChecker<uint8_t> ())


/**
 * SAT_ADD_RA_SERVICE_ATTRIBUTES macro helps defining RA service specific attributes
 * for SatLowerLayerServiceConf in method GetTypeId.
 *
 * \param index Index of the service which attributes are added to configuration.
 * \param a1    'Maximum unique payload per block' attribute value
 * \param a2    'Maximum consecutive block accessed' attribute value
 * \param a3    'Minimum idle block' attribute value
 *
 * \return TypeId
 */
#define SAT_ADD_RA_SERVICE_ATTRIBUTES(index, a1, a2, a3) \
   AddAttribute ( GetIndexAsRaServiceName (index) + "_MaximumUniquePayloadPerBlock", \
                  "Maximum unique payload per block for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a1), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MaximumUniquePayloadPerBlock, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MaximumUniquePayloadPerBlock), \
                  MakeUintegerChecker<uint8_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_MaximumConsecutiveBlockAccessed", \
                  "Maximum consecutive block accessed for RA  " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a2), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MaximumConsecutiveBlockAccessed, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MaximumConsecutiveBlockAccessed), \
                  MakeUintegerChecker<uint8_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_MinimumIdleBlock", \
                  "Minimum idle block for RA  " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a3), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MinimumIdleBlock, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MinimumIdleBlock), \
                  MakeUintegerChecker<uint8_t> ())

TypeId
SatLowerLayerServiceConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLowerLayerServiceConf")
    .SetParent<Object> ()
    .AddConstructor<SatLowerLayerServiceConf> ()
    .AddAttribute ( "DaServiceCount",
                    "DA services in use.",
                     UintegerValue (2),
                     MakeUintegerAccessor (&SatLowerLayerServiceConf::m_daServiceEntryCount),
                     MakeUintegerChecker<uint8_t> (SatLowerLayerServiceConf::m_minDaServiceEntries, SatLowerLayerServiceConf::m_maxDaServiceEntries))
    .AddAttribute ( "RaServiceCount",
                    "RA services in use.",
                     UintegerValue (2),
                     MakeUintegerAccessor (&SatLowerLayerServiceConf::m_raServiceEntryCount),
                     MakeUintegerChecker<uint8_t> (SatLowerLayerServiceConf::m_minRaServiceEntries, SatLowerLayerServiceConf::m_maxRaServiceEntries))
    .AddAttribute ( "DynamicRatePersistence",
                    "Dynamic rate persistence for the lower layer service.",
                     UintegerValue (5),
                     MakeUintegerAccessor (&SatLowerLayerServiceConf::m_dynamicRatePersistence),
                     MakeUintegerChecker<uint8_t> ())
    .AddAttribute ( "VolumeBacklogPersistence",
                    "Volume backlog persistence for the lower layer service",
                     UintegerValue (5),
                     MakeUintegerAccessor (&SatLowerLayerServiceConf::m_volumeBacklogPersistence),
                     MakeUintegerChecker<uint8_t> ())
    .AddAttribute ( "DefaultControlRandomizationInterval",
                    "Default control randomization interval for the lower layer service",
                     TimeValue (MilliSeconds (10)),
                     MakeTimeAccessor (&SatLowerLayerServiceConf::m_defaultControlRandomizationInterval),
                     MakeTimeChecker (MilliSeconds (0), MilliSeconds (std::numeric_limits<uint8_t>::max ())))

    /*
     * RC index, CRA allowed, RBDC allowed, VBDC allowed, CRA rate, Max RBDC rate, Min RBDC rate, Max volume backlog
     */
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (0, true, false, false, 50, 200, 50, 100)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (1, false, false, true, 50, 512, 100, 100)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (2, true, true, false, 100, 200, 50, 100)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (3, true, false, false, 100, 200, 50, 100)

    .SAT_ADD_RA_SERVICE_ATTRIBUTES (0, 3, 6, 2)
    .SAT_ADD_RA_SERVICE_ATTRIBUTES (1, 3, 6, 2)
    .SAT_ADD_RA_SERVICE_ATTRIBUTES (2, 3, 6, 2)
    .SAT_ADD_RA_SERVICE_ATTRIBUTES (3, 3, 6, 2)
  ;
  return tid;
}

TypeId
SatLowerLayerServiceConf::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

bool SatLowerLayerServiceConf::GetDaConstantAssignmentProvided (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantAssignmentProvided ();
}

void
SatLowerLayerServiceConf::SetDaConstantAssignmentProvided (uint8_t index, bool constAssignmentProvided)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantAssignmentProvided (constAssignmentProvided);
}

bool
SatLowerLayerServiceConf::GetDaRbdcAllowed (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetRbdcAllowed ();
}

void SatLowerLayerServiceConf::SetDaRbdcAllowed (uint8_t index, bool bdcAllowed)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetRbdcAllowed (bdcAllowed);
}

bool SatLowerLayerServiceConf::GetDaVolumeAllowed (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetVolumeAllowed ();
}

void
SatLowerLayerServiceConf::SetDaVolumeAllowed (uint8_t index, bool volumeAllowed)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetVolumeAllowed (volumeAllowed);
}

uint16_t
SatLowerLayerServiceConf::GetDaConstantServiceRateInKbps (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantServiceRateInKbps ();
}


Ptr<RandomVariableStream>
SatLowerLayerServiceConf::GetDaConstantServiceRateStream (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantServiceRateStream ();
}

void
SatLowerLayerServiceConf::SetDaConstantServiceRateStream (uint8_t index, Ptr<RandomVariableStream> constantServiceRateStream)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantServiceRateStream (constantServiceRateStream);
}

uint16_t
SatLowerLayerServiceConf::GetDaMaximumServiceRateInKbps (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetDaMaximumServiceRateInKbps (uint8_t index, uint16_t maximumServiceRateKbps)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumServiceRateInKbps (maximumServiceRateKbps);
}

uint16_t
SatLowerLayerServiceConf::GetDaMinimumServiceRateInKbps (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMinimumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetDaMinimumServiceRateInKbps (uint8_t index, uint16_t minimumServiceRateKbps)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMinimumServiceRateInKbps (minimumServiceRateKbps);
}

uint8_t
SatLowerLayerServiceConf::GetDaMaximumBacklogInKbytes (uint8_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumBacklogInKbytes ();
}

void
SatLowerLayerServiceConf::SetDaMaximumBacklogInKbytes (uint8_t index, uint8_t maximumBacklogInKbytes)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumBacklogInKbytes (maximumBacklogInKbytes);
}

uint8_t
SatLowerLayerServiceConf::GetRaMaximumUniquePayloadPerBlock (uint8_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumUniquePayloadPerBlock ();
}

void
SatLowerLayerServiceConf::SetRaMaximumUniquePayloadPerBlock (uint8_t index, uint8_t uniquePayloadPerBlock)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumUniquePayloadPerBlock (uniquePayloadPerBlock);
}

uint8_t
SatLowerLayerServiceConf::GetRaMaximumConsecutiveBlockAccessed (uint8_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumConsecutiveBlockAccessed ();
}

void
SatLowerLayerServiceConf::SetRaMaximumConsecutiveBlockAccessed (uint8_t index, uint8_t consecutiveBlockAccessed)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumConsecutiveBlockAccessed (consecutiveBlockAccessed);
}

uint8_t
SatLowerLayerServiceConf::GetRaMinimumIdleBlock (uint8_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMinimumIdleBlock ();
}

void
SatLowerLayerServiceConf::SetRaMinimumIdleBlock (uint8_t index, uint8_t minimumIdleBlock)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMinimumIdleBlock (minimumIdleBlock);
}

} // namespace ns3


