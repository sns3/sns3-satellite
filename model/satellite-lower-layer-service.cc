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
#include "satellite-lower-layer-service.h"

NS_LOG_COMPONENT_DEFINE ("SatLowerLayerServ");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLowerLayerServiceEntry);

 SatLowerLayerServiceEntry::SatLowerLayerServiceEntry ()
  : m_constantAssignmentProvided (false),
    m_rbdcAllowed (false),
    m_volumeAllowed (false),
    m_constantServiceRateKbps (0.0),
    m_maximumServiceRateKbps (0.0),
    m_minimumServiceRateKbps (0.0),
    m_maximumBacklogInBytes (0)
{
   NS_LOG_FUNCTION (this);
}

SatLowerLayerServiceEntry::~SatLowerLayerServiceEntry ()
{
   NS_LOG_FUNCTION (this);
}

TypeId
SatLowerLayerServiceEntry::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLowerLayerServiceEntry")
    .SetParent<Object> ()
    .AddConstructor<SatLowerLayerServiceEntry> ()
  ;
  return tid;
}

TypeId
SatLowerLayerServiceEntry::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

NS_OBJECT_ENSURE_REGISTERED (SatLowerLayerServiceConf);

SatLowerLayerServiceConf::SatLowerLayerServiceConf ()
 : m_dynamicRatePersistence (0),
   m_volumeBacklogPersistence (0),
   m_defaultControlRandomizationInterval (0.0),
   m_serviceCount (0)
{
   NS_LOG_FUNCTION (this);
}

SatLowerLayerServiceConf::~SatLowerLayerServiceConf ()
{
   NS_LOG_FUNCTION (this);
}

std::string
SatLowerLayerServiceConf::GetNumberAsString (uint32_t number)
{
  std::stringstream ss;   //create a string stream
  ss << number;           //add number to the stream

  return ss.str();
}

std::string
SatLowerLayerServiceConf::GetIndexAsServiceName (uint32_t index)
{
  std::string name = "Service";
  return name + GetNumberAsString (index);
}

/**
 * SAT_ADD_SERVICE_ATTRIBUTES macro helps defining service specific attributes
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
#define SAT_ADD_SERVICE_ATTRIBUTES(index, a1, a2, a3, a4, a5, a6, a7) \
   AddAttribute ( GetIndexAsServiceName (index) + "_ConstantAssignmentProvided", \
                  "Constant Assignment provided for " + GetIndexAsServiceName (index), \
                  BooleanValue (a1), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetServ ## index ## ConstantAssignmentProvided, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## ConstantAssignmentProvided), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_RbdcAllowed", \
                  "RBDC allowed for " + GetIndexAsServiceName (index), \
                  BooleanValue (a2), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetServ ## index ## RbdcAllowed, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## RbdcAllowed), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_VolumeAllowed", \
                  "Volume allowed for " + GetIndexAsServiceName (index), \
                  BooleanValue (a3), \
                  MakeBooleanAccessor (&SatLowerLayerServiceConf::SetServ ## index ## VolumeAllowed, \
                                       &SatLowerLayerServiceConf::GetServ ## index ## VolumeAllowed), \
                  MakeBooleanChecker ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_ConstantServiceRate", \
                  "Constant service rate [kbps] for " + GetIndexAsServiceName (index), \
                  DoubleValue (a4), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetServ ## index ## ConstantServiceRateInKbps, \
                                      &SatLowerLayerServiceConf::GetServ ## index ## ConstantServiceRateInKbps), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) + "_MaximumServiceRate", \
                  "Maximum service rate [kbps] for " + GetIndexAsServiceName (index), \
                  DoubleValue (a5), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MaximumServiceRateInKbps, \
                                      &SatLowerLayerServiceConf::GetServ ## index ## MaximumServiceRateInKbps), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) +  "_MinimumServiceRate", \
                  "Minimum service rate [kbps] for " + GetIndexAsServiceName (index), \
                  DoubleValue (a6), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MinimumServiceRateInKbps, \
                                      &SatLowerLayerServiceConf::GetServ ## index ## MinimumServiceRateInKbps), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsServiceName (index) +  "_MaximumBacklogSize", \
                  "Maximum backlog size [bytes] for " + GetIndexAsServiceName (index), \
                  UintegerValue (a7), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetServ ## index ## MaximumBacklogInBytes, \
                                      &SatLowerLayerServiceConf::GetServ ## index ## MaximumBacklogInBytes), \
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
    .SAT_ADD_SERVICE_ATTRIBUTES (0, true, true, true, 100.0, 200.0, 50.0, 1000)
    .SAT_ADD_SERVICE_ATTRIBUTES (1, true, false, true, 100.0, 200.0, 50.0, 1000)
    .SAT_ADD_SERVICE_ATTRIBUTES (2, true, true, false, 100.0, 200.0, 50.0, 1000)
    .SAT_ADD_SERVICE_ATTRIBUTES (3, true, false, false, 100.0, 200.0, 50.0, 1000)
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
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_service[index].GetConstantAssignmentProvided ();
}

void
SatLowerLayerServiceConf::SetConstantAssignmentProvided (uint32_t index, bool constAssignmentProvided)
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_service[index].SetConstantAssignmentProvided (constAssignmentProvided);
}

bool
SatLowerLayerServiceConf::GetRbdcAllowed (uint32_t index) const
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_service[index].GetRbdcAllowed ();
}

void SatLowerLayerServiceConf::SetRbdcAllowed (uint32_t index, bool bdcAllowed)
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_service[index].SetRbdcAllowed (bdcAllowed);
}

bool SatLowerLayerServiceConf::GetVolumeAllowed (uint32_t index) const
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_service[index].GetVolumeAllowed ();
}

void
SatLowerLayerServiceConf::SetVolumeAllowed (uint32_t index, bool volumeAllowed)
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_service[index].SetVolumeAllowed (volumeAllowed);
}

double
SatLowerLayerServiceConf::GetConstantServiceRateInKbps (uint32_t index) const
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_service[index].GetConstantServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetConstantServiceRateInKbps (uint32_t index, double constantServiceRateKbps)
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_service[index].SetConstantServiceRateInKbps (constantServiceRateKbps);
}

double
SatLowerLayerServiceConf::GetMaximumServiceRateInKbps (uint32_t index) const
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_service[index].GetMaximumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetMaximumServiceRateInKbps (uint32_t index, double maximumServiceRateKbps)
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_service[index].SetMaximumServiceRateInKbps (maximumServiceRateKbps);
}

double
SatLowerLayerServiceConf::GetMinimumServiceRateInKbps (uint32_t index) const
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_service[index].GetMinimumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetMinimumServiceRateInKbps (uint32_t index, double minimumServiceRateKbps)
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_service[index].SetMinimumServiceRateInKbps (minimumServiceRateKbps);
}

uint32_t
SatLowerLayerServiceConf::GetMaximumBacklogInBytes (uint32_t index) const
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_service[index].GetMaximumBacklogInBytes ();
}

void
SatLowerLayerServiceConf::SetMaximumBacklogInBytes (uint32_t index, uint32_t maximumBacklogBytes)
{
  if ( index >= m_maxServices)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_service[index].SetMaximumBacklogInBytes (maximumBacklogBytes);
}

} // namespace ns3


