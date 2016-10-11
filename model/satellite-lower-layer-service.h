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

#ifndef SATELLITE_LOWER_LAYER_SERIVICE_H
#define SATELLITE_LOWER_LAYER_SERIVICE_H

#include <vector>
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * The SatLowerLayerService class holds information of a satellite lower layer service DA entry.
 *
 * This class is only used by SatLowerLayerServiceConf class.
 */
class SatLowerLayerServiceDaEntry
{
  // grant access for SatLowerLayerServiceConf to access private members
  friend class SatLowerLayerServiceConf;

public:
  SatLowerLayerServiceDaEntry ();

  /**
   * Destructor for SatLowerLayerServiceDaEntry
   */
  ~SatLowerLayerServiceDaEntry ();

private:
  bool                      m_constantAssignmentProvided;
  bool                      m_rbdcAllowed;
  bool                      m_volumeAllowed;
  Ptr<RandomVariableStream> m_constantServiceRateStream;
  uint16_t                  m_maximumServiceRateKbps;
  uint16_t                  m_minimumServiceRateKbps;
  uint16_t                  m_maximumBacklogInKbytes;

  /**
   * Get state, if constant assignment is provided.
   *
   * \return Is constant assignment provided [true or false]
   */
  inline bool GetConstantAssignmentProvided () const
  {
    return m_constantAssignmentProvided;
  }

  /**
   * Set state, if constant assignment is provided.
   *
   * \param constAssignmentProvided Constant assignment provided state [true or false]
   */
  inline void SetConstantAssignmentProvided (bool constAssignmentProvided)
  {
    m_constantAssignmentProvided = constAssignmentProvided;
  }

  /**
   * Get state, if RBDC is allowed.
   *
   * \return Is RBDC allowed [true or false]
   */
  inline bool GetRbdcAllowed () const
  {
    return m_rbdcAllowed;
  }

  /**
   * Set state, if RBDC is allowed.
   *
   * \param bdcAllowed RBDC allowed state [true or false]
   */
  inline void SetRbdcAllowed (bool bdcAllowed)
  {
    m_rbdcAllowed = bdcAllowed;
  }

  /**
   * Get state, if volume is allowed.
   *
   * \return Is volume allowed [true or false]
   */
  inline bool GetVolumeAllowed () const
  {
    return m_volumeAllowed;
  }

  /**
   * Set state, if volume is allowed.
   *
   * \param volumeAllowed Volume allowed state [true or false]
   */
  inline void SetVolumeAllowed (bool volumeAllowed)
  {
    m_volumeAllowed = volumeAllowed;
  }

  /**
   * Get constant service rate.
   *
   * \return Constant service rate [KBps]
   */
  inline uint16_t GetConstantServiceRateInKbps () const
  {
    return m_constantServiceRateStream->GetInteger ();
  }

  /**
   * Get constant service rate stream.
   *
   * \return Constant service rate stream.
   */
  inline Ptr<RandomVariableStream> GetConstantServiceRateStream () const
  {
    return m_constantServiceRateStream;
  }

  /**
   * Set constant service rate stream.
   *
   * \param constantServiceRateKbps Constant service rate [KBps]
   */
  inline void SetConstantServiceRateStream (Ptr<RandomVariableStream> constantServiceRateStream)
  {
    m_constantServiceRateStream = constantServiceRateStream;
  }

  /**
   * Get maximum service rate.
   *
   * \return Maximum service rate [KBps]
   */
  inline uint16_t GetMaximumServiceRateInKbps () const
  {
    return m_maximumServiceRateKbps;
  }

  /**
   * Set maximum service rate.
   *
   * \param maximumServiceRateKbps Maximum service rate [KBps]
   */
  inline void SetMaximumServiceRateInKbps (uint16_t maximumServiceRateKbps)
  {
    m_maximumServiceRateKbps = maximumServiceRateKbps;
  }

  /**
   * Get minimum service rate.
   *
   * \return Minimum service rate [KBps]
   */
  inline uint16_t GetMinimumServiceRateInKbps () const
  {
    return m_minimumServiceRateKbps;
  }

  /**
   * Set minimum service rate.
   *
   * \param minimumServiceRateKbps Minimum service rate [KBps]
   */
  inline void SetMinimumServiceRateInKbps (uint16_t minimumServiceRateKbps)
  {
    m_minimumServiceRateKbps = minimumServiceRateKbps;
  }

  /**
   * Get maximum backlog size.
   *
   * \return Maximum backlog size [KBytes]
   */
  inline uint16_t GetMaximumBacklogInKbytes () const
  {
    return m_maximumBacklogInKbytes;
  }

  /**
   * Set maximum backlog size.
   *
   * \param maximumBacklogInBytes Maximum backlog size [KBytes]
   */
  inline void SetMaximumBacklogInKbytes (uint16_t maximumBacklogInKbytes)
  {
    m_maximumBacklogInKbytes = maximumBacklogInKbytes;
  }
};

/**
 * \ingroup satellite
 *
 * The SatLowerLayerService class holds information of a satellite lower layer service RA entry.
 */
class SatLowerLayerServiceRaEntry
{
  // grant access for SatLowerLayerServiceConf to access private members
  friend class SatLowerLayerServiceConf;

public:
  SatLowerLayerServiceRaEntry ();

  /**
   * Destructor for SatLowerLayerServiceRaEntry
   */
  ~SatLowerLayerServiceRaEntry ();

private:
  uint8_t  m_maxUniquePayloadPerBlock;
  uint8_t  m_maxConsecutiveBlockAccessed;
  uint8_t  m_minimumIdleBlock;
  uint16_t m_backOffTimeInMilliSeconds;
  uint16_t m_highLoadBackOffTimeInMilliSeconds;
  uint16_t m_backOffProbability;
  uint16_t m_highLoadBackOffProbability;
  uint8_t m_numberOfInstances;
  double m_averageNormalizedOfferedLoadThreshold;

  /**
   * Get maximum unique payload per block.
   *
   * \return Maximum unique payload per block
   */
  inline uint8_t GetMaximumUniquePayloadPerBlock () const
  {
    return m_maxUniquePayloadPerBlock;
  }

  /**
   * Set maximum unique payload per block.
   *
   * \param maxUniquePayloadPerBlock Maximum unique payload per block
   */
  inline void SetMaximumUniquePayloadPerBlock (uint8_t maxUniquePayloadPerBlock)
  {
    m_maxUniquePayloadPerBlock = maxUniquePayloadPerBlock;
  }

  /**
   * Get maximum consecutive block accessed.
   *
   * \return Maximum consecutive block accessed
   */
  inline uint8_t GetMaximumConsecutiveBlockAccessed () const
  {
    return m_maxConsecutiveBlockAccessed;
  }

  /**
   * Set maximum consecutive block accessed.
   *
   * \param maxConsecutiveBlockAccessed Maximum consecutive block accessed
   */
  inline void SetMaximumConsecutiveBlockAccessed (uint8_t maxConsecutiveBlockAccessed)
  {
    m_maxConsecutiveBlockAccessed = maxConsecutiveBlockAccessed;
  }

  /**
   * Get minimum idle block.
   *
   * \return Minimum idle block
   */
  inline uint8_t GetMinimumIdleBlock () const
  {
    return m_minimumIdleBlock;
  }

  /**
   * Set minimum idle block.
   *
   * \param minimumIdleBlock Minimum idle block
   */
  inline void SetMinimumIdleBlock (uint8_t minimumIdleBlock)
  {
    m_minimumIdleBlock = minimumIdleBlock;
  }

  /**
   * Get back off time in milliseconds.
   *
   * \return Back off time
   */
  inline uint16_t GetBackOffTimeInMilliSeconds () const
  {
    return m_backOffTimeInMilliSeconds;
  }

  /**
   * Set back off time in milliseconds.
   *
   * \param backOffTimeInMilliSeconds Back off time
   */
  inline void SetBackOffTimeInMilliSeconds (uint16_t backOffTimeInMilliSeconds)
  {
    m_backOffTimeInMilliSeconds = backOffTimeInMilliSeconds;
  }

  /**
   * Get high load back off time in milliseconds.
   *
   * \return Back off time
   */
  inline uint16_t GetHighLoadBackOffTimeInMilliSeconds () const
  {
    return m_highLoadBackOffTimeInMilliSeconds;
  }

  /**
   * Set high load back off time in milliseconds.
   *
   * \param backOffTimeInMilliSeconds Back off time
   */
  inline void SetHighLoadBackOffTimeInMilliSeconds (uint16_t backOffTimeInMilliSeconds)
  {
    m_highLoadBackOffTimeInMilliSeconds = backOffTimeInMilliSeconds;
  }

  /**
   * Get back off probability.
   *
   * \return Back off probability
   */
  inline uint16_t GetBackOffProbability () const
  {
    return m_backOffProbability;
  }

  /**
   * Set back off probability.
   *
   * \param backOffProbability Back off probability
   */
  inline void SetBackOffProbability (uint16_t backOffProbability)
  {
    m_backOffProbability = backOffProbability;
  }

  /**
   * Get high load back off probability.
   *
   * \return High load back off probability
   */
  inline uint16_t GetHighLoadBackOffProbability () const
  {
    return m_highLoadBackOffProbability;
  }

  /**
   * Set high load back off probability.
   *
   * \param highLoadBackOffProbability High load back off probability
   */
  inline void SetHighLoadBackOffProbability (uint16_t highLoadBackOffProbability)
  {
    m_highLoadBackOffProbability = highLoadBackOffProbability;
  }

  /**
   * Get number of instances.
   *
   * \return Number of instances
   */
  inline uint8_t GetNumberOfInstances () const
  {
    return m_numberOfInstances;
  }

  /**
   * Set number of instances.
   *
   * \param numberOfInstances Number of instances
   */
  inline void SetNumberOfInstances (uint8_t numberOfInstances)
  {
    m_numberOfInstances = numberOfInstances;
  }

  /**
   * Get average normalized offered load threshold.
   *
   * \return Average normalized offered load threshold
   */
  inline double GetAverageNormalizedOfferedLoadThreshold () const
  {
    return m_averageNormalizedOfferedLoadThreshold;
  }

  /**
   * Set average normalized offered load threshold.
   *
   * \param averageNormalizedOfferedLoadThreshold Average normalized offered load threshold
   */
  inline void SetAverageNormalizedOfferedLoadThreshold (double averageNormalizedOfferedLoadThreshold)
  {
    m_averageNormalizedOfferedLoadThreshold = averageNormalizedOfferedLoadThreshold;
  }
};

/**
 * \ingroup satellite
 *
 * The SatLowerLayerServiceConf class holds information of all configures lower layer service entries.
 *
 * This class is only used by SatLowerLayerServiceConf class.
 */
class SatLowerLayerServiceConf : public Object
{
public:
  static const uint8_t m_minDaServiceEntries = 2;
  static const uint8_t m_minRaServiceEntries = 1;

  static const uint8_t m_maxDaServiceEntries = 4;
  static const uint8_t m_maxRaServiceEntries = 1;

  /**
   * Constructor for SatLowerLayerServiceConf.
   */
  SatLowerLayerServiceConf ();

  /**
   * Destructor for SatLowerLayerServiceConf.
   */
  ~SatLowerLayerServiceConf ();


  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get count of configured RA services.
   *
   * \return count of configured RA services.
   */
  inline uint8_t GetRaServiceCount () const
  {
    return m_raServiceEntryCount;
  }

  /**
   * Get count of configured DA services.
   *
   * \return count of configured DA services.
   */
  inline uint8_t GetDaServiceCount () const
  {
    return m_daServiceEntryCount;
  }

  /**
   * Get configured dynamic rate persistence.
   *
   * \return Dynamic rate persistence
   */
  uint8_t GetDynamicRatePersistence () const
  {
    return m_dynamicRatePersistence;
  }

  /**
   * Get configured volume backlog persistence.
   *
   * \return Volume backlog persistence
   */
  uint8_t GetVolumeBacklogPersistence () const
  {
    return m_volumeBacklogPersistence;
  }

  /**
   * Get configured default control randomization interval.
   *
   * \return Default control randomization interval
   */
  Time GetDefaultControlRandomizationInterval () const
  {
    return m_defaultControlRandomizationInterval;
  }

  /**
   * Get state, if constant assignment is provided for a DA service.
   *
   * \param index Index of the service
   * \return Is constant assignment provided [true or false]
   */
  bool GetDaConstantAssignmentProvided (uint8_t index) const;

  /**
   * Get state, if RBDC is allowed for a DA service.
   *
   * \param index Index of the service
   * \return Is RBDC allowed [true or false]
   */
  bool GetDaRbdcAllowed (uint8_t index) const;

  /**
   * Get state, if volume is allowed for a DA service.
   *
   * \param index Index of the service
   * \return Is volume allowed [true or false]
   */
  bool GetDaVolumeAllowed (uint8_t index) const;

  /**
   * Get constant service rate for a DA service.
   *
   * \param index Index of the service
   * \return Constant service rate [KBps]
   */
  uint16_t GetDaConstantServiceRateInKbps (uint8_t index) const;

  /**
   * Get constant service rate stream for a DA service.
   *
   * \param index Index of the service
   * \return Constant service rate stream [KBps]
   */
  Ptr<RandomVariableStream> GetDaConstantServiceRateStream (uint8_t index) const;

  /**
   * Get maximum service rate for a DA service.
   *
   * \param index Index of the service
   * \return Maximum service rate [KBps]
   */
  uint16_t GetDaMaximumServiceRateInKbps (uint8_t index) const;

  /**
   * Get minimum service rate for a DA service.
   *
   * \param index Index of the service
   * \return Minimum service rate [KBps]
   */
  uint16_t GetDaMinimumServiceRateInKbps (uint8_t index) const;

  /**
   * Get maximum backlog size for a DA service.
   *
   * \param index Index of the service
   * \return Maximum backlog size [KBytes]
   */
  uint16_t GetDaMaximumBacklogInKbytes (uint8_t index) const;

  /**
   * Get maximum unique payload per block for a RA service.
   *
   * \param index Index of the service
   * \return Maximum unique payload per block
   */
  uint8_t GetRaMaximumUniquePayloadPerBlock (uint8_t index) const;

  /**
   * Get maximum consecutive block accessed  for a RA service.
   *
   * \param index Index of the service
   * \return Maximum consecutive block accessed
   */
  uint8_t GetRaMaximumConsecutiveBlockAccessed (uint8_t index) const;

  /**
   * Get minimum idle block for a RA service.
   *
   * \param index Index of the service
   * \return Minimum idle block
   */
  uint8_t GetRaMinimumIdleBlock (uint8_t index) const;

  /**
   * Get back off time in milliseconds.
   *
   * \param index Index of the service
   * \return Back off time
   */
  uint16_t GetRaBackOffTimeInMilliSeconds (uint8_t index) const;

  /**
   * Get high load back off time in milliseconds.
   *
   * \param index Index of the service
   * \return Back off time
   */
  uint16_t GetRaHighLoadBackOffTimeInMilliSeconds (uint8_t index) const;

  /**
   * Get back off probability.
   *
   * \param index Index of the service
   * \return Back off probability
   */
  uint16_t GetRaBackOffProbability (uint8_t index) const;

  /**
   * Get high load back off probability.
   *
   * \param index Index of the service
   * \return High load back off probability
   */
  uint16_t GetRaHighLoadBackOffProbability (uint8_t index) const;

  /**
   * Get number of instances.
   *
   * \param index Index of the service
   * \return Number of instances
   */
  uint8_t GetRaNumberOfInstances (uint8_t index) const;

  /**
   * Get average normalized offeredLoad Threshold.
   *
   * \param index Index of the service
   * \return Average normalized offered load threshold
   */
  double GetRaAverageNormalizedOfferedLoadThreshold (uint8_t index) const;

private:
  uint8_t                                  m_dynamicRatePersistence;
  uint8_t                                  m_volumeBacklogPersistence;
  Time                                     m_defaultControlRandomizationInterval;
  uint8_t                                  m_daServiceEntryCount;
  SatLowerLayerServiceDaEntry              m_daServiceEntries[m_maxDaServiceEntries];
  uint8_t                                  m_raServiceEntryCount;
  SatLowerLayerServiceRaEntry              m_raServiceEntries[m_maxRaServiceEntries];

  /**
   * Template method to convert number to string
   * \param number number to convert as string
   * \return number as string
   */
  template <class T>
  static std::string GetNumberAsString (T number)
  {
    std::stringstream ss;   // create a string stream
    ss << (double) number;  // add number to the stream as double always to show number correctly in outputs

    return ss.str ();
  }

  /**
   * Method to convert DA service index to service name.
   *
   * \param index index to convert as service name
   * \return service name
   */
  static std::string GetIndexAsDaServiceName (uint8_t index);

  /**
   * Method to convert RA service index to service name.
   *
   * \param index index to convert as service name
   * \return service name
   */
  static std::string GetIndexAsRaServiceName (uint8_t index);

  /**
   * Set state, if constant assignment is provided for a DA service.
   *
   * \param index Index of the service
   * \param constAssignmentProvided Constant assignment provided state [true or false]
   */
  void SetDaConstantAssignmentProvided (uint8_t index, bool constAssignmentProvided);

  /**
   * Set state, if RBDC is allowed for a DA service.
   *
   * \param index Index of the service
   * \param bdcAllowed RBDC allowed state [true or false]
   */
  void SetDaRbdcAllowed (uint8_t index, bool bdcAllowed);

  /**
   * Set state, if volume is allowed for a DA service.
   *
   * \param index Index of the service
   * \param volumeAllowed Volume allowed state [true or false]
   */
  void SetDaVolumeAllowed (uint8_t index, bool volumeAllowed);

  /**
   * Set constant service rate stream for a DA service.
   *
   * \param index Index of the service
   * \param constantServiceRateStream Constant service rate stream [KBps]
   */
  void SetDaConstantServiceRateStream (uint8_t index, Ptr<RandomVariableStream> constantServiceRateStream);

  /**
   * Set maximum service rate for a DA service.
   *
   * \param index Index of the service
   * \param maximumServiceRateKbps Maximum service rate [KBps]
   */
  void SetDaMaximumServiceRateInKbps (uint8_t index, uint16_t maximumServiceRateKbps);

  /**
   * Set minimum service rate for a DA service.
   *
   * \param index Index of the service
   * \param minimumServiceRateKbps Minimum service rate [KBps]
   */
  void SetDaMinimumServiceRateInKbps (uint8_t index, uint16_t minimumServiceRateKbps);

  /**
   * Set maximum backlog size for a DA service.
   *
   * \param index Index of the service
   * \param maximumBacklogInKbytes Maximum backlog size [KBytes]
   */
  void SetDaMaximumBacklogInKbytes (uint8_t index, uint16_t maximumBacklogInKbytes);

  /**
   * Set maximum unique payload per block for a RA service.
   *
   * \param index Index of the service
   * \param maxUniquePayloadPerBlock Maximum unique payload per block
   */
  void SetRaMaximumUniquePayloadPerBlock (uint8_t index, uint8_t maxUniquePayloadPerBlock);

  /**
   * Set maximum consecutive block accessed for a RA service.
   *
   * \param index Index of the service
   * \param maxConsecutiveBlockAccessed Maximum consecutive block accessed
   */
  void SetRaMaximumConsecutiveBlockAccessed (uint8_t index, uint8_t maxConsecutiveBlockAccessed);

  /**
   * Set minimum idle block for a RA service.
   *
   * \param index Index of the service
   * \param minimumIdleBlock Minimum idle block
   */
  void SetRaMinimumIdleBlock (uint8_t index, uint8_t minimumIdleBlock);

  /**
   * Set back off time in milliseconds.
   *
   * \param index Index of the service
   * \param backOffTimeInMilliSeconds Back off time
   */
  void SetRaBackOffTimeInMilliSeconds (uint8_t index, uint16_t backOffTimeInMilliSeconds);

  /**
   * Set high load back off time in milliseconds.
   *
   * \param index Index of the service
   * \param backOffTimeInMilliSeconds Back off time
   */
  void SetRaHighLoadBackOffTimeInMilliSeconds (uint8_t index, uint16_t backOffTimeInMilliSeconds);

  /**
   * Set back off probability.
   *
   * \param index Index of the service
   * \param backOffProbability Back off probability
   */
  void SetRaBackOffProbability (uint8_t index, uint16_t backOffProbability);

  /**
   * Set high load back off probability.
   *
   * \param index Index of the service
   * \param highLoadBackOffProbability High load back off probability
   */
  void SetRaHighLoadBackOffProbability (uint8_t index, uint16_t highLoadBackOffProbability);

  /**
   * Set number of instances.
   *
   * \param index Index of the service
   * \param numberOfInstances Number of instances
   */
  void SetRaNumberOfInstances (uint8_t index, uint8_t numberOfInstances);

  /**
   * Set average normalized offeredLoad Threshold.
   *
   * \param index Index of the service
   * \param Average normalized offered load threshold
   */
  void SetRaAverageNormalizedOfferedLoadThreshold (uint8_t index, double averageNormalizedOfferedLoadThreshold);

/**
 * SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE macro helps to define DA service entry
 * attribute access method.
 *
 *  Methods defined e.g. when index is 0.
 *
 *  - SetDaServ0ConstantAssignmentProvided, see @SatLowerLayerServiceEntry::SetConstantAssignmentProvided
 *  - GetDaServ0ConstantAssignmentProvided, see @SatLowerLayerServiceEntry::GetConstantAssignmentProvided
 *  - SetDaServ0RbdcAllowed, see @SatLowerLayerServiceEntry::SetRbdcAllowed
 *  - GetDaServ0RbdcAllowed, see @SatLowerLayerServiceEntry::GetRbdcAllowed
 *  - SetDaServ0VolumeAllowed, see @SatLowerLayerServiceEntry::SetVolumeAllowed
 *  - GetDaServ0VolumeAllowed, see @SatLowerLayerServiceEntry::GetVolumeAllowed
 *  - SetDaServ0ConstantServiceRateStream, see @SatLowerLayerServiceEntry::SetonstantServiceRateStream
 *  - GetDaServ0ConstantServiceRateStream, see @SatLowerLayerServiceEntry::GetonstantServiceRateStream
 *  - SetDaServ0MaximumServiceRateInKbps, see @SatLowerLayerServiceEntry::SetMaximumServiceRateInKbps
 *  - GetDaServ0MaximumServiceRateInKbps, see @SatLowerLayerServiceEntry::GetMaximumServiceRateInKbps
 *  - SetDaServ0MinimumServiceRateInKbps, see @SatLowerLayerServiceEntry::SetMinimumServiceRateInKbps
 *  - GetDaServ0MinimumServiceRateInKbps, see @SatLowerLayerServiceEntry::GetMinimumServiceRateInKbps
 *  - SetDaServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::SetMaximumBacklogSizeInKbytes
 *  - GetDaServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::GetMaximumBacklogSizeInKbytes
 *
 * \param index Index of the service which attribute access methods are defined
 */
#define SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
  inline void SetDaServ ## index ## ConstantAssignmentProvided (bool value)  \
  { return SetDaConstantAssignmentProvided (index, value); \
  } \
  inline bool GetDaServ ## index ## ConstantAssignmentProvided () const  \
  { return GetDaConstantAssignmentProvided (index); } \
  inline void SetDaServ ## index ## RbdcAllowed (bool value)  \
  { return SetDaRbdcAllowed (index, value); } \
  inline bool GetDaServ ## index ## RbdcAllowed () const  \
  { return GetDaRbdcAllowed (index); } \
  inline void SetDaServ ## index ## VolumeAllowed (bool value)  \
  { return SetDaVolumeAllowed (index, value); } \
  inline bool GetDaServ ## index ## VolumeAllowed () const  \
  { return GetDaVolumeAllowed (index); } \
  inline void SetDaServ ## index ## ConstantServiceRateStream (Ptr<RandomVariableStream> value)  \
  { return SetDaConstantServiceRateStream (index, value); } \
  inline Ptr<RandomVariableStream> GetDaServ ## index ## ConstantServiceRateStream () const  \
  { return GetDaConstantServiceRateStream (index); } \
  inline void SetDaServ ## index ## MaximumServiceRateInKbps (uint16_t value)  \
  { return SetDaMaximumServiceRateInKbps (index, value); } \
  inline uint16_t GetDaServ ## index ## MaximumServiceRateInKbps () const  \
  { return GetDaMaximumServiceRateInKbps (index); } \
  inline void SetDaServ ## index ## MinimumServiceRateInKbps (uint16_t value)  \
  { return SetDaMinimumServiceRateInKbps (index, value); } \
  inline uint16_t GetDaServ ## index ## MinimumServiceRateInKbps () const  \
  { return GetDaMinimumServiceRateInKbps (index); } \
  inline void SetDaServ ## index ## MaximumBacklogInKbytes (uint16_t value)  \
  { return SetDaMaximumBacklogInKbytes (index, value); } \
  inline uint8_t GetDaServ ## index ## MaximumBacklogInKbytes () const  \
  { return GetDaMaximumBacklogInKbytes (index); }

/**
 * SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE macro helps to define RA service entry
 * attribute access method.
 *
 *  Methods defined e.g. when index is 0.
 *
 *  - SetRaServ0MaximumUniquePayloadPerBlock, see @SatLowerLayerServiceRaEntry::SetMaximumUniquePayloadPerBlock
 *  - GetRaServ0MaximumUniquePayloadPerBlock, see @SatLowerLayerServiceRaEntry::GetMaximumUniquePayloadPerBlock
 *  - SetRaServ0MaximumConsecutiveBlockAccessed, see @SatLowerLayerServiceRaEntry::SetMaximumConsecutiveBlockAccessed
 *  - GetRaServ0MaximumConsecutiveBlockAccessed, see @SatLowerLayerServiceRaEntry::GetMaximumConsecutiveBlockAccessed
 *  - SetRaServ0MinimumIdleBlock, see @SatLowerLayerServiceRaEntry::SetMinimumIdleBlock
 *  - GetRaServ0VMinimumIdleBlock, see @SatLowerLayerServiceRaEntry::GetMinimumIdleBlock
 *  - SetRaServ0BackOffTimeInMilliSeconds, see @SatLowerLayerServiceRaEntry::SetBackOffTimeInMilliSeconds
 *  - GetRaServ0VBackOffTimeInMilliSeconds, see @SatLowerLayerServiceRaEntry::GetBackOffTimeInMilliSeconds
 *  - SetRaServ0HighLoadBackOffTimeInMilliSeconds, see @SatLowerLayerServiceRaEntry::SetHighLoadBackOffTimeInMilliSeconds
 *  - GetRaServ0VHighLoadBackOffTimeInMilliSeconds, see @SatLowerLayerServiceRaEntry::GetHighLoadBackOffTimeInMilliSeconds
 *  - SetRaServ0BackOffProbability, see @SatLowerLayerServiceRaEntry::SetBackOffProbability
 *  - GetRaServ0VBackOffProbability, see @SatLowerLayerServiceRaEntry::GetBackOffProbability
 *  - SetRaServ0HighLoadBackOffProbability, see @SatLowerLayerServiceRaEntry::SetHighLoadBackOffProbability
 *  - GetRaServ0VHighLoadBackOffProbability, see @SatLowerLayerServiceRaEntry::GetHighLoadBackOffProbability
 *  - SetRaServ0NumberOfInstances, see @SatLowerLayerServiceRaEntry::SetNumberOfInstances
 *  - GetRaServ0VNumberOfInstances, see @SatLowerLayerServiceRaEntry::GetNumberOfInstances
 *  - SetRaServ0AverageNormalizedOfferedLoadThreshold, see @SatLowerLayerServiceRaEntry::SetAverageNormalizedOfferedLoadThreshold
 *  - GetRaServ0VAverageNormalizedOfferedLoadThreshold, see @SatLowerLayerServiceRaEntry::GetAverageNormalizedOfferedLoadThreshold
 *
 * \param index Index of the service which attribute access methods are defined
 */
#define SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
  inline void SetRaServ ## index ## MaximumUniquePayloadPerBlock (uint8_t value)  \
  { return SetRaMaximumUniquePayloadPerBlock (index, value); } \
  inline uint8_t GetRaServ ## index ## MaximumUniquePayloadPerBlock () const  \
  { return GetRaMaximumUniquePayloadPerBlock (index); } \
  inline void SetRaServ ## index ## MaximumConsecutiveBlockAccessed (uint8_t value)  \
  { return SetRaMaximumConsecutiveBlockAccessed (index, value); } \
  inline uint8_t GetRaServ ## index ## MaximumConsecutiveBlockAccessed () const  \
  { return GetRaMaximumConsecutiveBlockAccessed (index); } \
  inline void SetRaServ ## index ## MinimumIdleBlock (uint8_t value)  \
  { return SetRaMinimumIdleBlock (index, value); } \
  inline uint8_t GetRaServ ## index ## MinimumIdleBlock () const  \
  { return GetRaMinimumIdleBlock (index); } \
  inline void SetRaServ ## index ## BackOffTimeInMilliSeconds (uint16_t value)  \
  { return SetRaBackOffTimeInMilliSeconds (index, value); } \
  inline uint16_t GetRaServ ## index ## BackOffTimeInMilliSeconds () const  \
  { return GetRaBackOffTimeInMilliSeconds (index); } \
  inline void SetRaServ ## index ## HighLoadBackOffTimeInMilliSeconds (uint16_t value)  \
  { return SetRaHighLoadBackOffTimeInMilliSeconds (index, value); } \
  inline uint16_t GetRaServ ## index ## HighLoadBackOffTimeInMilliSeconds () const  \
  { return GetRaHighLoadBackOffTimeInMilliSeconds (index); } \
  inline void SetRaServ ## index ## BackOffProbability (uint16_t value)  \
  { return SetRaBackOffProbability (index, value); } \
  inline uint16_t GetRaServ ## index ## BackOffProbability () const  \
  { return GetRaBackOffProbability (index); } \
  inline void SetRaServ ## index ## HighLoadBackOffProbability (uint16_t value)  \
  { return SetRaHighLoadBackOffProbability (index, value); } \
  inline uint16_t GetRaServ ## index ## HighLoadBackOffProbability () const  \
  { return GetRaHighLoadBackOffProbability (index); } \
  inline void SetRaServ ## index ## NumberOfInstances (uint8_t value)  \
  { return SetRaNumberOfInstances (index, value); } \
  inline uint8_t GetRaServ ## index ## NumberOfInstances () const  \
  { return GetRaNumberOfInstances (index); } \
  inline void SetRaServ ## index ## AverageNormalizedOfferedLoadThreshold (double value)  \
  { return SetRaAverageNormalizedOfferedLoadThreshold (index, value); } \
  inline double GetRaServ ## index ## AverageNormalizedOfferedLoadThreshold () const  \
  { return GetRaAverageNormalizedOfferedLoadThreshold (index); }

  SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE (0);
  SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE (1);
  SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE (2);
  SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE (3);

  SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE (0);
  //SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(1);
  //SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(2);
  //SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(3);
};

} // namespace ns3

#endif /* SATELLITE_LOWER_LAYER_SERIVICE_H */
