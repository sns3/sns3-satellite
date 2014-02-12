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
: m_maxUniquePayloadPerBlockInBytes (0),
  m_maxConsecutiveBlockAccessedInBytes (0),
  m_minimumIdleBlockInBytes (0)
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
SatLowerLayerServiceConf::GetIndexAsServiceName (uint32_t index)
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
   AddAttribute ( GetIndexAsServiceName (index) + "_DaConstantAssignmentProvided", \
                  "Constant Assignment provided for " + GetIndexAsServiceName (index), \
                  BooleanValue (a1), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetServ ## index ## ConstantAssignmentProvided, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## ConstantAssignmentProvided), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_DaRbdcAllowed", \
                  "RBDC allowed for DA " + GetIndexAsServiceName (index), \
                  BooleanValue (a2), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetServ ## index ## RbdcAllowed, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## RbdcAllowed), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_DaVolumeAllowed", \
                  "Volume allowed for DA " + GetIndexAsServiceName (index), \
                  BooleanValue (a3), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetServ ## index ## VolumeAllowed, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## VolumeAllowed), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_DaConstantServiceRate", \
                  "A RandomVariableStream for constant service rate [kbps] for DA " + GetIndexAsServiceName (index), \
                  StringValue ("ns3::ConstantRandomVariable[Constant=" + GetNumberAsString<double> (a4) + "]"), \
                  MakePointerAccessor (&SatLowerLayerServiceConf::SetServ ## index ## ConstantServiceRateStream, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## ConstantServiceRateStream), \
                  MakePointerChecker<RandomVariableStream> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_DaMaximumServiceRate", \
                  "Maximum service rate [kbps] for DA " + GetIndexAsServiceName (index), \
                  DoubleValue (a5), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MaximumServiceRateInKbps, \
                                      &SatLowerLayerServiceConf::GetServ ## index ## MaximumServiceRateInKbps), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) +  "_DaMinimumServiceRate", \
                  "Minimum service rate [kbps] for DA " + GetIndexAsServiceName (index), \
                  DoubleValue (a6), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MinimumServiceRateInKbps, \
                                      &SatLowerLayerServiceConf::GetServ ## index ## MinimumServiceRateInKbps), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) +  "_DaMaximumBacklogSize", \
                  "Maximum backlog size [bytes] for DA " + GetIndexAsServiceName (index), \
                  UintegerValue (a7), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MaximumBacklogInBytes, \
                                      &SatLowerLayerServiceConf::GetServ ## index ## MaximumBacklogInBytes), \
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
   AddAttribute ( GetIndexAsServiceName (index) + "_RaMaximumUniquePayloadPerBlock", \
                  "Maximum unique payload per block [bytes] for RA " + GetIndexAsServiceName (index), \
                  UintegerValue (a1), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MaximumUniquePayloadPerBlockInBytes, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## MaximumUniquePayloadPerBlockInBytes), \
                  MakeUintegerChecker<uint32_t> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_RaMaximumConsecutiveBlockAccessed", \
                  "Maximum consecutive block accessed [bytes] for RA  " + GetIndexAsServiceName (index), \
                  UintegerValue (a2), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MaximumConsecutiveBlockAccessedInBytes, \
                                        &SatLowerLayerServiceConf::GetServ ## index ## MaximumConsecutiveBlockAccessedInBytes), \
                  MakeUintegerChecker<uint32_t> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_RaGetMinimumIdleBlock", \
                  "Minimum idle block [bytes] for RA  " + GetIndexAsServiceName (index), \
                  UintegerValue (a3), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MinimumIdleBlockInBytes, \
                                        &SatLowerLayerServiceConf::GetServ ## index ## MinimumIdleBlockInBytes), \
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

bool SatLowerLayerServiceConf::GetConstantAssignmentProvided (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantAssignmentProvided ();
}

void
SatLowerLayerServiceConf::SetConstantAssignmentProvided (uint32_t index, bool constAssignmentProvided)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantAssignmentProvided (constAssignmentProvided);
}

bool
SatLowerLayerServiceConf::GetRbdcAllowed (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetRbdcAllowed ();
}

void SatLowerLayerServiceConf::SetRbdcAllowed (uint32_t index, bool bdcAllowed)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetRbdcAllowed (bdcAllowed);
}

bool SatLowerLayerServiceConf::GetVolumeAllowed (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetVolumeAllowed ();
}

void
SatLowerLayerServiceConf::SetVolumeAllowed (uint32_t index, bool volumeAllowed)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetVolumeAllowed (volumeAllowed);
}

Ptr<RandomVariableStream>
SatLowerLayerServiceConf::GetConstantServiceRateStream (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantServiceRateStream ();
}

void
SatLowerLayerServiceConf::SetConstantServiceRateStream (uint32_t index, Ptr<RandomVariableStream> constantServiceRateStream)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantServiceRateStream (constantServiceRateStream);
}

double
SatLowerLayerServiceConf::GetMaximumServiceRateInKbps (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetMaximumServiceRateInKbps (uint32_t index, double maximumServiceRateKbps)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumServiceRateInKbps (maximumServiceRateKbps);
}

double
SatLowerLayerServiceConf::GetMinimumServiceRateInKbps (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMinimumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetMinimumServiceRateInKbps (uint32_t index, double minimumServiceRateKbps)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMinimumServiceRateInKbps (minimumServiceRateKbps);
}

uint32_t
SatLowerLayerServiceConf::GetMaximumBacklogInBytes (uint32_t index) const
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumBacklogInBytes ();
}

void
SatLowerLayerServiceConf::SetMaximumBacklogInBytes (uint32_t index, uint32_t maximumBacklogInBytes)
{
  if ( index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumBacklogInBytes (maximumBacklogInBytes);
}

uint32_t
SatLowerLayerServiceConf::GetMaximumUniquePayloadPerBlockInBytes (uint32_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumUniquePayloadPerBlockInBytes ();
}

void
SatLowerLayerServiceConf::SetMaximumUniquePayloadPerBlockInBytes (uint32_t index, uint32_t uniquePayloadPerBlockInBytes)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumUniquePayloadPerBlockInBytes (uniquePayloadPerBlockInBytes);
}

uint32_t
SatLowerLayerServiceConf::GetMaximumConsecutiveBlockAccessedInBytes (uint32_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumUniquePayloadPerBlockInBytes ();
}

void
SatLowerLayerServiceConf::SetMaximumConsecutiveBlockAccessedInBytes (uint32_t index, uint32_t consecutiveBlockAccessedInBytes)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumConsecutiveBlockAccessedInBytes (consecutiveBlockAccessedInBytes);
}

uint32_t
SatLowerLayerServiceConf::GetMinimumIdleBlockInBytes (uint32_t index) const
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMinimumIdleBlockInBytes ();
}

void
SatLowerLayerServiceConf::SetMinimumIdleBlockInBytes (uint32_t index, uint32_t minimumIdleBlockInBytes)
{
  if ( index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMinimumIdleBlockInBytes (minimumIdleBlockInBytes);
}

} // namespace ns3


