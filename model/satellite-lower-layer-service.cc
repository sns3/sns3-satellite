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
#include "../model/satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatLowerLayerServiceConf");

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
    m_minimumIdleBlock (0),
    m_backOffTimeInMilliSeconds (0),
    m_highLoadBackOffTimeInMilliSeconds (0),
    m_backOffProbability (0),
    m_highLoadBackOffProbability (0),
    m_numberOfInstances (0),
    m_averageNormalizedOfferedLoadThreshold (0.0)
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

  if (m_minRaServiceEntries > m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("SatLowerLayerServiceConf::SatLowerLayerServiceConf - m_minRaServiceEntries > m_maxRaServiceEntries");
    }

  if (m_minDaServiceEntries > m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("SatLowerLayerServiceConf::SatLowerLayerServiceConf - m_minDaServiceEntries > m_maxDaServiceEntries");
    }
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
 * \param a4    'Constant service rate' attribute value [Kbps]
 * \param a5    'Maximum service rate' attribute value [Kbps]
 * \param a6    'Minimum service rate' attribute value [Kbps]
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
                  "Maximum backlog size [Kbytes] for DA " + GetIndexAsDaServiceName (index), \
                  UintegerValue (a7), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetDaServ ## index ## MaximumBacklogInKbytes, \
                                        &SatLowerLayerServiceConf::GetDaServ ## index ## MaximumBacklogInKbytes), \
                  MakeUintegerChecker<uint16_t> ())


/**
 * SAT_ADD_RA_SERVICE_ATTRIBUTES macro helps defining RA service specific attributes
 * for SatLowerLayerServiceConf in method GetTypeId.
 *
 * \param index Index of the service which attributes are added to configuration.
 * \param a1    'Maximum unique payload per block' attribute value
 * \param a2    'Maximum consecutive block accessed' attribute value
 * \param a3    'Minimum idle block' attribute value
 * \param a4    'Back off time in milliseconds' attribute value
 * \param a5    'High load back off time in milliseconds' attribute value
 * \param a6    'Back off probability' attribute value
 * \param a7    'High load back off probability' attribute value
 * \param a8    'Number of instances' attribute value
 * \param a9    'Average normalized offered load threshold' attribute value
 *
 * \return TypeId
 */
#define SAT_ADD_RA_SERVICE_ATTRIBUTES(index, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  AddAttribute ( GetIndexAsRaServiceName (index) + "_MaximumUniquePayloadPerBlock", \
                 "Maximum unique payload per block for RA " + GetIndexAsRaServiceName (index), \
                 UintegerValue (a1), \
                 MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MaximumUniquePayloadPerBlock, \
                                       &SatLowerLayerServiceConf::GetRaServ ## index ## MaximumUniquePayloadPerBlock), \
                 MakeUintegerChecker<uint8_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_MaximumConsecutiveBlockAccessed", \
                  "Maximum consecutive block accessed for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a2), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MaximumConsecutiveBlockAccessed, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MaximumConsecutiveBlockAccessed), \
                  MakeUintegerChecker<uint8_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_MinimumIdleBlock", \
                  "Minimum idle block for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a3), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## MinimumIdleBlock, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## MinimumIdleBlock), \
                  MakeUintegerChecker<uint8_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_BackOffTimeInMilliSeconds", \
                  "Back off time in milliseconds for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a4), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## BackOffTimeInMilliSeconds, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## BackOffTimeInMilliSeconds), \
                  MakeUintegerChecker<uint16_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_HighLoadBackOffTimeInMilliSeconds", \
                  "High load back off time in milliseconds for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a5), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## HighLoadBackOffTimeInMilliSeconds, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## HighLoadBackOffTimeInMilliSeconds), \
                  MakeUintegerChecker<uint16_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_BackOffProbability", \
                  "Back off probability for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a6), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## BackOffProbability, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## BackOffProbability), \
                  MakeUintegerChecker<uint16_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_HighLoadBackOffProbability", \
                  "High load back off probability for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a7), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## HighLoadBackOffProbability, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## HighLoadBackOffProbability), \
                  MakeUintegerChecker<uint16_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_NumberOfInstances", \
                  "Number of instances for RA " + GetIndexAsRaServiceName (index), \
                  UintegerValue (a8), \
                  MakeUintegerAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## NumberOfInstances, \
                                        &SatLowerLayerServiceConf::GetRaServ ## index ## NumberOfInstances), \
                  MakeUintegerChecker<uint8_t> ()) \
  .AddAttribute ( GetIndexAsRaServiceName (index) + "_AverageNormalizedOfferedLoadThreshold", \
                  "Average normalized offered load threshold for RA " + GetIndexAsRaServiceName (index), \
                  DoubleValue (a9), \
                  MakeDoubleAccessor (&SatLowerLayerServiceConf::SetRaServ ## index ## AverageNormalizedOfferedLoadThreshold, \
                                      &SatLowerLayerServiceConf::GetRaServ ## index ## AverageNormalizedOfferedLoadThreshold), \
                  MakeDoubleChecker<double> ())

TypeId
SatLowerLayerServiceConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatLowerLayerServiceConf")
    .SetParent<Object> ()
    .AddConstructor<SatLowerLayerServiceConf> ()
    .AddAttribute ( "DaServiceCount",
                    "DA services in use.",
                    UintegerValue (4),
                    MakeUintegerAccessor (&SatLowerLayerServiceConf::m_daServiceEntryCount),
                    MakeUintegerChecker<uint8_t> (SatLowerLayerServiceConf::m_minDaServiceEntries, SatLowerLayerServiceConf::m_maxDaServiceEntries))
    .AddAttribute ( "RaServiceCount",
                    "RA services in use.",
                    UintegerValue (1),
                    MakeUintegerAccessor (&SatLowerLayerServiceConf::m_raServiceEntryCount),
                    MakeUintegerChecker<uint8_t> (SatLowerLayerServiceConf::m_minRaServiceEntries, SatLowerLayerServiceConf::m_maxRaServiceEntries))
    .AddAttribute ( "DynamicRatePersistence",
                    "Dynamic rate persistence for the lower layer service.",
                    UintegerValue (5),
                    MakeUintegerAccessor (&SatLowerLayerServiceConf::m_dynamicRatePersistence),
                    MakeUintegerChecker<uint8_t> ())
    .AddAttribute ( "VolumeBacklogPersistence",
                    "Volume backlog persistence for the lower layer service",
                    UintegerValue (7),
                    MakeUintegerAccessor (&SatLowerLayerServiceConf::m_volumeBacklogPersistence),
                    MakeUintegerChecker<uint8_t> ())
    .AddAttribute ( "DefaultControlRandomizationInterval",
                    "Default control randomization interval for the lower layer service",
                    TimeValue (MilliSeconds (100)),
                    MakeTimeAccessor (&SatLowerLayerServiceConf::m_defaultControlRandomizationInterval),
                    MakeTimeChecker (MilliSeconds (0), MilliSeconds (std::numeric_limits<uint8_t>::max ())))
    /*
     * RC index, CRA allowed, RBDC allowed, VBDC allowed, CRA rate, Max RBDC rate, Min RBDC rate, Max volume backlog
     */
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (0, true, false, false, 50, 9216, 10, 384)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (1, false, false, false, 50, 9216, 10, 384)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (2, false, false, false, 50, 9216, 10, 384)
    .SAT_ADD_DA_SERVICE_ATTRIBUTES (3, false, true, false, 50, 9216, 10, 384)

    .SAT_ADD_RA_SERVICE_ATTRIBUTES (0, 3, 4, 2, 250, 500, 10000, 30000, 3, 0.5)
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
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantAssignmentProvided ();
}

void
SatLowerLayerServiceConf::SetDaConstantAssignmentProvided (uint8_t index, bool constAssignmentProvided)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << constAssignmentProvided);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantAssignmentProvided (constAssignmentProvided);
}

bool
SatLowerLayerServiceConf::GetDaRbdcAllowed (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetRbdcAllowed ();
}

void SatLowerLayerServiceConf::SetDaRbdcAllowed (uint8_t index, bool bdcAllowed)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << bdcAllowed);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetRbdcAllowed (bdcAllowed);
}

bool SatLowerLayerServiceConf::GetDaVolumeAllowed (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetVolumeAllowed ();
}

void
SatLowerLayerServiceConf::SetDaVolumeAllowed (uint8_t index, bool volumeAllowed)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << volumeAllowed);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetVolumeAllowed (volumeAllowed);
}

uint16_t
SatLowerLayerServiceConf::GetDaConstantServiceRateInKbps (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantServiceRateInKbps ();
}


Ptr<RandomVariableStream>
SatLowerLayerServiceConf::GetDaConstantServiceRateStream (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetConstantServiceRateStream ();
}

void
SatLowerLayerServiceConf::SetDaConstantServiceRateStream (uint8_t index, Ptr<RandomVariableStream> constantServiceRateStream)
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetConstantServiceRateStream (constantServiceRateStream);
}

uint16_t
SatLowerLayerServiceConf::GetDaMaximumServiceRateInKbps (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetDaMaximumServiceRateInKbps (uint8_t index, uint16_t maximumServiceRateKbps)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << maximumServiceRateKbps);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumServiceRateInKbps (maximumServiceRateKbps);
}

uint16_t
SatLowerLayerServiceConf::GetDaMinimumServiceRateInKbps (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMinimumServiceRateInKbps ();
}

void
SatLowerLayerServiceConf::SetDaMinimumServiceRateInKbps (uint8_t index, uint16_t minimumServiceRateKbps)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << minimumServiceRateKbps);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMinimumServiceRateInKbps (minimumServiceRateKbps);
}

uint16_t
SatLowerLayerServiceConf::GetDaMaximumBacklogInKbytes (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_daServiceEntries[index].GetMaximumBacklogInKbytes ();
}

void
SatLowerLayerServiceConf::SetDaMaximumBacklogInKbytes (uint8_t index, uint16_t maximumBacklogInKbytes)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << maximumBacklogInKbytes);

  if (index >= m_maxDaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_daServiceEntries[index].SetMaximumBacklogInKbytes (maximumBacklogInKbytes);
}

uint8_t
SatLowerLayerServiceConf::GetRaMaximumUniquePayloadPerBlock (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumUniquePayloadPerBlock ();
}

void
SatLowerLayerServiceConf::SetRaMaximumUniquePayloadPerBlock (uint8_t index, uint8_t uniquePayloadPerBlock)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << uniquePayloadPerBlock);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumUniquePayloadPerBlock (uniquePayloadPerBlock);
}

uint8_t
SatLowerLayerServiceConf::GetRaMaximumConsecutiveBlockAccessed (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMaximumConsecutiveBlockAccessed ();
}

void
SatLowerLayerServiceConf::SetRaMaximumConsecutiveBlockAccessed (uint8_t index, uint8_t consecutiveBlockAccessed)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << consecutiveBlockAccessed);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMaximumConsecutiveBlockAccessed (consecutiveBlockAccessed);
}

uint8_t
SatLowerLayerServiceConf::GetRaMinimumIdleBlock (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetMinimumIdleBlock ();
}

void
SatLowerLayerServiceConf::SetRaMinimumIdleBlock (uint8_t index, uint8_t minimumIdleBlock)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << minimumIdleBlock);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetMinimumIdleBlock (minimumIdleBlock);
}

uint16_t
SatLowerLayerServiceConf::GetRaBackOffTimeInMilliSeconds (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetBackOffTimeInMilliSeconds ();
}

void
SatLowerLayerServiceConf::SetRaBackOffTimeInMilliSeconds (uint8_t index, uint16_t backOffTimeInMilliSeconds)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << backOffTimeInMilliSeconds);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetBackOffTimeInMilliSeconds (backOffTimeInMilliSeconds);
}

uint16_t
SatLowerLayerServiceConf::GetRaHighLoadBackOffTimeInMilliSeconds (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetHighLoadBackOffTimeInMilliSeconds ();
}

void
SatLowerLayerServiceConf::SetRaHighLoadBackOffTimeInMilliSeconds (uint8_t index, uint16_t backOffTimeInMilliSeconds)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << backOffTimeInMilliSeconds);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetHighLoadBackOffTimeInMilliSeconds (backOffTimeInMilliSeconds);
}

uint16_t
SatLowerLayerServiceConf::GetRaBackOffProbability (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetBackOffProbability ();
}

void
SatLowerLayerServiceConf::SetRaBackOffProbability (uint8_t index, uint16_t backOffProbability)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << backOffProbability);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetBackOffProbability (backOffProbability);
}
uint16_t
SatLowerLayerServiceConf::GetRaHighLoadBackOffProbability (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetHighLoadBackOffProbability ();
}

void
SatLowerLayerServiceConf::SetRaHighLoadBackOffProbability (uint8_t index, uint16_t highLoadBackOffProbability)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << highLoadBackOffProbability );

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetHighLoadBackOffProbability (highLoadBackOffProbability);
}

uint8_t
SatLowerLayerServiceConf::GetRaNumberOfInstances (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetNumberOfInstances ();
}

void
SatLowerLayerServiceConf::SetRaNumberOfInstances (uint8_t index, uint8_t numberOfInstances)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << (uint32_t) numberOfInstances);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetNumberOfInstances (numberOfInstances);
}

double
SatLowerLayerServiceConf::GetRaAverageNormalizedOfferedLoadThreshold (uint8_t index) const
{
  NS_LOG_FUNCTION (this << (uint32_t) index);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  return m_raServiceEntries[index].GetAverageNormalizedOfferedLoadThreshold ();
}

void
SatLowerLayerServiceConf::SetRaAverageNormalizedOfferedLoadThreshold (uint8_t index, double averageNormalizedOfferedLoadThreshold)
{
  NS_LOG_FUNCTION (this << (uint32_t) index << averageNormalizedOfferedLoadThreshold);

  if (index >= m_maxRaServiceEntries)
    {
      NS_FATAL_ERROR ("Service index out of range!!!");
    }

  m_raServiceEntries[index].SetAverageNormalizedOfferedLoadThreshold (averageNormalizedOfferedLoadThreshold);
}

} // namespace ns3
