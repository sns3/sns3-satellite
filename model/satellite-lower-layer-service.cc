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

NS_OBJECT_ENSURE_REGISTERED (SatLowerLayerServiceDaEntry);

 SatLowerLayerServiceDaEntry::SatLowerLayerServiceDaEntry ()
  : m_constantAssignmentProvided (false),
    m_rbdcAllowed (false),
    m_volumeAllowed (false),
    m_constantServiceRateStream (0),
    m_maximumServiceRateKbps (0.0),
    m_minimumServiceRateKbps (0.0),
    m_maximumBacklogInBytes (0)
{
   NS_LOG_FUNCTION (this);
}

SatLowerLayerServiceDaEntry::~SatLowerLayerServiceDaEntry ()
{
   NS_LOG_FUNCTION (this);
}

TypeId
SatLowerLayerServiceDaEntry::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLowerLayerServiceDaEntry")
    .SetParent<Object> ()
    .AddConstructor<SatLowerLayerServiceDaEntry> ()
  ;
  return tid;
}

TypeId
SatLowerLayerServiceDaEntry::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

NS_OBJECT_ENSURE_REGISTERED (SatLowerLayerServiceRaEntry);

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

TypeId
SatLowerLayerServiceRaEntry::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLowerLayerServiceRaEntry")
    .SetParent<Object> ()
    .AddConstructor<SatLowerLayerServiceRaEntry> ()
  ;
  return tid;
}

TypeId
SatLowerLayerServiceRaEntry::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
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
}

SatLowerLayerServiceConf::~SatLowerLayerServiceConf ()
{
   NS_LOG_FUNCTION (this);
}

std::string
SatLowerLayerServiceConf::GetIndexAsDaServiceName (uint32_t index)
{
  std::string name = "Service";
  return name + GetNumberAsString<uint32_t> (index);
}

std::string
SatLowerLayerServiceConf::GetIndexAsRaServiceName (uint32_t index)
{
  std::string name = "Service";
  return name + GetNumberAsString<uint32_t> (index);
}

/**
 * SAT_ADD_DA_SERVICE_ATTRIBUTES macro helps defining service specific attributes
 * for SatLowerLayerServiceConf in method GetTypeId.
 *
 * \param index Index of the service which attributes are added to configuration.
 * \param a1    'Constant assignment provided' attribute value [true or false]
 * \param a2    'RBDC allowed' attribute value [true or false]
 * \param a3    'Volume allowed' attribute value [true or false]
 * \param a4    'Constant service rate' attribute value [kbps]
 * \param a5    'Maximum service rate' attribute value [kbps]
 * \param a6    'Minimum service rate' attribute value [kbps]
 * \param a7    'Maximum backlog size' attribute value [bytes]
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
                  StringValue ("ns3::ConstantRandomVariable[Constant=" + GetNumberAsString<double> (a4) + "]"), \
                  MakePointerAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## ConstantServiceRateStream, \
                                       &SatLowerLayerServiceConf::GetDaServ ## index ## ConstantServiceRateStream), \
                  MakePointerChecker<RandomVariableStream> ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) + "_MaximumServiceRate", \
                  "Maximum service rate [kbps] for DA " + GetIndexAsDaServiceName (index), \
                  DoubleValue (a5), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## MaximumServiceRateInKbps, \
                                      &SatLowerLayerServiceConf::GetDaServ ## index ## MaximumServiceRateInKbps), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) +  "_MinimumServiceRate", \
                  "Minimum service rate [kbps] for DA " + GetIndexAsDaServiceName (index), \
                  DoubleValue (a6), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## MinimumServiceRateInKbps, \
                                      &SatLowerLayerServiceConf::GetDaServ ## index ## MinimumServiceRateInKbps), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsDaServiceName (index) +  "_MaximumBacklogSize", \
                  "Maximum backlog size [bytes] for DA " + GetIndexAsDaServiceName (index), \
                  UintegerValue (a7), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## MaximumBacklogInBytes, \
                                      &SatLowerLayerServiceConf::GetDaServ ## index ## MaximumBacklogInBytes), \
                  MakeUintegerChecker<uint32_t> ())


/**
 * SAT_ADD_RA_SERVICE_ATTRIBUTES macro helps defining RA service specific attributes
 * for SatLowerLayerServiceConf in method GetTypeId.
 *
 * \param index Index of the service which attributes are added to configuration.
 * \param a1    'Maximum unique payload per block' attribute value [bytes]
 * \param a2    'Maximum consecutive block accessed' attribute value [bytes]
 * \param a3    'Minimum idle block' attribute value [bytes]
 *
 * \return TypeId
 */
#define SAT_ADD_RA_SERVICE_ATTRIBUTES(index, a1, a2, a3) \
   AddAttribute ( GetIndexAsRaServiceName (index) + "_MaximumUniquePayloadPerBlock", \
                  "Maximum unique payload per block [bytes] for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a1), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MaximumUniquePayloadPerBlock, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MaximumUniquePayloadPerBlock), \
                  MakeUintegerChecker<uint32_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_MaximumConsecutiveBlockAccessed", \
                  "Maximum consecutive block accessed [bytes] for RA  " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a2), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MaximumConsecutiveBlockAccessed, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MaximumConsecutiveBlockAccessed), \
                  MakeUintegerChecker<uint32_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_MinimumIdleBlock", \
                  "Minimum idle block [bytes] for RA  " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a3), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MinimumIdleBlock, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MinimumIdleBlock), \
                  MakeUintegerChecker<uint32_t> ())

TypeId
SatLowerLayerServiceConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLowerLayerServiceConf")
    .SetParent<Object> ()
    .AddConstructor<SatLowerLayerServiceConf> ()
    .AddAttribute ( "DynamicRatePersistence",
                    "Dynamic rate persistence for the lower layer service.",
                     UintegerValue (5),
                     MakeUintegerAccessor (&SatLowerLayerServiceConf::m_dynamicRatePersistence),
                     MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "VolumeBacklogPersistence",
                    "Volume backlog persistence for the lower layer service",
                     UintegerValue (5),
                     MakeUintegerAccessor (&SatLowerLayerServiceConf::m_volumeBacklogPersistence),
                     MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "DefaultControlRandomizationInterval",
                    "Default control randomization interval for the lower layer service",
                     TimeValue ( MilliSeconds (50)),
                     MakeTimeAccessor (&SatLowerLayerServiceConf::m_defaultControlRandomizationInterval),
                     MakeTimeChecker ())
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (0, true, true, true, 100.0, 200.0, 50.0, 1000)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (1, true, false, true, 100.0, 200.0, 50.0, 1000)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (2, true, true, false, 100.0, 200.0, 50.0, 1000)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (3, true, false, false, 100.0, 200.0, 50.0, 1000)

    .SAT_ADD_RA_SERVICE_ATTRIBUTES (0, 500, 500, 500)
    .SAT_ADD_RA_SERVICE_ATTRIBUTES (1, 500, 500, 500)
    .SAT_ADD_RA_SERVICE_ATTRIBUTES (2, 500, 500, 500)
    .SAT_ADD_RA_SERVICE_ATTRIBUTES (3, 500, 500, 500)
  ;
  return tid;
}

TypeId
SatLowerLayerServiceConf::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

bool SatLowerLayerServiceConf::GetDaConstantAssignmentProvided (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantAssignmentProvided ();
}

void
SatLowerLayerServiceConf::SetDaConstantAssignmentProvided (uint32_t index, bool constAssignmentProvided)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantAssignmentProvided (constAssignmentProvided);
}

bool
SatLowerLayerServiceConf::GetDaRbdcAllowed (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetRbdcAllowed ();
}

void SatLowerLayerServiceConf::SetDaRbdcAllowed (uint32_t index, bool bdcAllowed)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetRbdcAllowed (bdcAllowed);
}

bool SatLowerLayerServiceConf::GetDaVolumeAllowed (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetVolumeAllowed ();
}

void
SatLowerLayerServiceConf::SetDaVolumeAllowed (uint32_t index, bool volumeAllowed)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetVolumeAllowed (volumeAllowed);
}

Ptr<RandomVariableStream>
SatLowerLayerServiceConf::GetDaConstantServiceRateStream (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantServiceRateStream ();
}

void
SatLowerLayerServiceConf::SetDaConstantServiceRateStream (uint32_t index, Ptr<RandomVariableStream> constantServiceRateStream)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantServiceRateStream (constantServiceRateStream);
}

double
SatLowerLayerServiceConf::GetDaMaximumServiceRateInKbps (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetDaMaximumServiceRateInKbps (uint32_t index, double maximumServiceRateKbps)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumServiceRateInKbps (maximumServiceRateKbps);
}

double
SatLowerLayerServiceConf::GetDaMinimumServiceRateInKbps (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMinimumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetDaMinimumServiceRateInKbps (uint32_t index, double minimumServiceRateKbps)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMinimumServiceRateInKbps (minimumServiceRateKbps);
}

uint32_t
SatLowerLayerServiceConf::GetDaMaximumBacklogInBytes (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumBacklogInBytes ();
}

void
SatLowerLayerServiceConf::SetDaMaximumBacklogInBytes (uint32_t index, uint32_t maximumBacklogInBytes)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumBacklogInBytes (maximumBacklogInBytes);
}

uint32_t
SatLowerLayerServiceConf::GetRaMaximumUniquePayloadPerBlock (uint32_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumUniquePayloadPerBlock ();
}

void
SatLowerLayerServiceConf::SetRaMaximumUniquePayloadPerBlock (uint32_t index, uint32_t uniquePayloadPerBlock)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumUniquePayloadPerBlock (uniquePayloadPerBlock);
}

uint32_t
SatLowerLayerServiceConf::GetRaMaximumConsecutiveBlockAccessed (uint32_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumUniquePayloadPerBlock ();
}

void
SatLowerLayerServiceConf::SetRaMaximumConsecutiveBlockAccessed (uint32_t index, uint32_t consecutiveBlockAccessed)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumConsecutiveBlockAccessed (consecutiveBlockAccessed);
}

uint32_t
SatLowerLayerServiceConf::GetRaMinimumIdleBlock (uint32_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMinimumIdleBlock ();
}

void
SatLowerLayerServiceConf::SetRaMinimumIdleBlock (uint32_t index, uint32_t minimumIdleBlock)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMinimumIdleBlock (minimumIdleBlock);
}

} // namespace ns3


