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
 */
class SatLowerLayerServiceDaEntry : public Object
{
public:
  SatLowerLayerServiceDaEntry () ;
  ~SatLowerLayerServiceDaEntry ();

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get state, if constant assignment is provided.
   *
   * \return Is constant assignment provided [true or false]
   */
  inline bool GetConstantAssignmentProvided () const {return m_constantAssignmentProvided;}

  /**
   * Set state, if constant assignment is provided.
   *
   * \param constAssignmentProvided Constant assignment provided state [true or false]
   */
  inline void SetConstantAssignmentProvided (bool constAssignmentProvided) { m_constantAssignmentProvided = constAssignmentProvided;}

  /**
   * Get state, if RBDC is allowed.
   *
   * \return Is RBDC allowed [true or false]
   */
  inline bool GetRbdcAllowed () const {return m_rbdcAllowed;}

  /**
   * Set state, if RBDC is allowed.
   *
   * \param bdcAllowed RBDC allowed state [true or false]
   */
  inline void SetRbdcAllowed (bool bdcAllowed) { m_rbdcAllowed = bdcAllowed;}

  /**
   * Get state, if volume is allowed.
   *
   * \return Is volume allowed [true or false]
   */
  inline bool GetVolumeAllowed () const {return m_volumeAllowed;}

  /**
   * Set state, if volume is allowed.
   *
   * \param volumeAllowed Volume allowed state [true or false]
   */
  inline void SetVolumeAllowed (bool volumeAllowed) { m_volumeAllowed = volumeAllowed;}

  /**
   * Get constant service rate.
   *
   * \return Constant service rate [kbps]
   */
  inline uint32_t GetConstantServiceRateInKbps () const {return m_constantServiceRateStream->GetInteger();}

  /**
   * Get constant service rate stream.
   *
   * \return Constant service rate stream.
   */
  inline Ptr<RandomVariableStream> GetConstantServiceRateStream () const { return m_constantServiceRateStream;}

  /**
   * Set constant service rate stream.
   *
   * \param constantServiceRateKbps Constant service rate [kbps]
   */
  inline void SetConstantServiceRateStream (Ptr<RandomVariableStream> constantServiceRateStream) { m_constantServiceRateStream = constantServiceRateStream;}

  /**
   * Get maximum service rate.
   *
   * \return Maximum service rate [kbps]
   */
  inline uint32_t GetMaximumServiceRateInKbps () const {return m_maximumServiceRateKbps;}

  /**
   * Set maximum service rate.
   *
   * \param maximumServiceRateKbps Maximum service rate [kbps]
   */
  inline void SetMaximumServiceRateInKbps (uint32_t maximumServiceRateKbps) { m_maximumServiceRateKbps = maximumServiceRateKbps;}

  /**
   * Get minimum service rate.
   *
   * \return Minimum service rate [kbps]
   */
  inline uint32_t GetMinimumServiceRateInKbps () const {return m_minimumServiceRateKbps;}

  /**
   * Set minimum service rate.
   *
   * \param minimumServiceRateKbps Minimum service rate [kbps]
   */
  inline void SetMinimumServiceRateInKbps (uint32_t minimumServiceRateKbps) { m_minimumServiceRateKbps = minimumServiceRateKbps;}

  /**
   * Get maximum backlog size.
   *
   * \return Maximum backlog size [bytes]
   */
  inline uint32_t GetMaximumBacklogInBytes () const {return m_maximumBacklogInBytes;}

  /**
   * Set maximum backlog size.
   *
   * \param maximumBacklogInBytes Maximum backlog size [bytes]
   */
  inline void SetMaximumBacklogInBytes (uint32_t maximumBacklogInBytes) { m_maximumBacklogInBytes = maximumBacklogInBytes;}

private:
  bool                      m_constantAssignmentProvided;
  bool                      m_rbdcAllowed;
  bool                      m_volumeAllowed;
  Ptr<RandomVariableStream> m_constantServiceRateStream;
  double                    m_maximumServiceRateKbps;
  double                    m_minimumServiceRateKbps;
  uint32_t                  m_maximumBacklogInBytes;
};

/**
 * \ingroup satellite
 *
 * The SatLowerLayerService class holds information of a satellite lower layer service RA entry.
 */
class SatLowerLayerServiceRaEntry : public Object
{
public:
  SatLowerLayerServiceRaEntry () ;
  ~SatLowerLayerServiceRaEntry ();

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get maximum unique payload per block.
   *
   * \return Maximum unique payload per block
   */
  inline uint32_t GetMaximumUniquePayloadPerBlock () const {return m_maxUniquePayloadPerBlock;}

  /**
   * Set maximum unique payload per block.
   *
   * \param maxUniquePayloadPerBlock Maximum unique payload per block
   */
  inline void SetMaximumUniquePayloadPerBlock (uint32_t maxUniquePayloadPerBlock) { m_maxUniquePayloadPerBlock = maxUniquePayloadPerBlock;}

  /**
   * Get maximum consecutive block accessed.
   *
   * \return Maximum consecutive block accessed
   */
  inline uint32_t GetMaximumConsecutiveBlockAccessedInBytes () const {return m_maxConsecutiveBlockAccessed;}

  /**
   * Set maximum consecutive block accessed.
   *
   * \param maxConsecutiveBlockAccessed Maximum consecutive block accessed
   */
  inline void SetMaximumConsecutiveBlockAccessed (uint32_t maxConsecutiveBlockAccessed) { m_maxConsecutiveBlockAccessed = maxConsecutiveBlockAccessed;}

  /**
   * Get minimum idle block.
   *
   * \return Minimum idle block
   */
  inline uint32_t GetMinimumIdleBlock () const {return m_minimumIdleBlock;}

  /**
   * Set minimum idle block.
   *
   * \param minimumIdleBlock Minimum idle block
   */
  inline void SetMinimumIdleBlock (uint32_t minimumIdleBlock) { m_minimumIdleBlock = minimumIdleBlock;}

private:
  uint32_t  m_maxUniquePayloadPerBlock;
  uint32_t  m_maxConsecutiveBlockAccessed;
  uint32_t  m_minimumIdleBlock;
};

/**
 * \ingroup satellite
 *
 * The SatLowerLayerServiceConf class holds information of all configures lower layer service entries.
 */
class SatLowerLayerServiceConf : public Object
{
public:
  static const uint32_t m_maxDaServiceEntries = 4;
  static const uint32_t m_maxRaServiceEntries = 4;

  /**
   * Constructor for SatLowerLayerServiceConf.
   */
  SatLowerLayerServiceConf ();

  /**
   * Destructor for SatLowerLayerServiceConf.
   */
  ~SatLowerLayerServiceConf ();

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Template method to convert number to string
   * \param number number to convert as string
   * \return number as string
   */
  template <class T>
  static std::string GetNumberAsString (T number)
  {
    std::stringstream ss;   //create a string stream
    ss << number;           //add number to the stream

    return ss.str();
  }

  /**
   * Method to convert DA service index to service name.
   *
   * \param index index to convert as service name
   * \return service name
   */
  static std::string GetIndexAsDaServiceName(uint32_t index);

  /**
   * Method to convert RA service index to service name.
   *
   * \param index index to convert as service name
   * \return service name
   */
  static std::string GetIndexAsRaServiceName(uint32_t index);

  /**
   * Get count of configured RA services.
   *
   * \return count of configured RA services.
   */
  inline uint32_t GetRaServiceCount () const { return m_raServiceEntryCount;}

  /**
   * Get count of configured DA services.
   *
   * \return count of configured DA services.
   */
  inline uint32_t GetDaServiceCount () const { return m_daServiceEntryCount;}

  /**
   * Get configured dynamic rate persistence.
   *
   * \return Dynamic rate persistence
   */
  uint32_t GetDynamicRatePersistence () const { return m_dynamicRatePersistence; }

  /**
   * Get configured volume backlog persistence.
   *
   * \return Volume backlog persistence
   */
  uint32_t GetVolumeBacklogPersistence () const { return m_volumeBacklogPersistence; }

  /**
   * Get configured default control randomization interval.
   *
   * \return Default control randomization interval
   */
  Time GetDefaultControlRandomizationInterval () const { return m_defaultControlRandomizationInterval; }

  /**
   * Get state, if constant assignment is provided for a DA service.
   *
   * \param index Index of the service
   * \return Is constant assignment provided [true or false]
   */
  bool GetDaConstantAssignmentProvided (uint32_t index) const;

  /**
   * Set state, if constant assignment is provided for a DA service.
   *
   * \param index Index of the service
   * \param constAssignmentProvided Constant assignment provided state [true or false]
   */
  void SetDaConstantAssignmentProvided (uint32_t index, bool constAssignmentProvided);

  /**
   * Get state, if RBDC is allowed for a DA service.
   *
   * \param index Index of the service
   * \return Is RBDC allowed [true or false]
   */
  bool GetDaRbdcAllowed (uint32_t index) const;

  /**
   * Set state, if RBDC is allowed for a DA service.
   *
   * \param index Index of the service
   * \param bdcAllowed RBDC allowed state [true or false]
   */
  void SetDaRbdcAllowed (uint32_t index, bool bdcAllowed);

  /**
   * Get state, if volume is allowed for a DA service.
   *
   * \param index Index of the service
   * \return Is volume allowed [true or false]
   */
  bool GetDaVolumeAllowed (uint32_t index) const;

  /**
   * Set state, if volume is allowed for a DA service.
   *
   * \param index Index of the service
   * \param volumeAllowed Volume allowed state [true or false]
   */
  void SetDaVolumeAllowed (uint32_t index, bool volumeAllowed);

  /**
   * Get constant service rate for a DA service.
   *
   * \param index Index of the service
   * \return Constant service rate [kbps]
   */
  uint32_t GetDaConstantServiceRateInKbps (uint32_t index) const;

  /**
   * Get constant service rate stream for a DA service.
   *
   * \param index Index of the service
   * \return Constant service rate stream [kbps]
   */
  Ptr<RandomVariableStream> GetDaConstantServiceRateStream (uint32_t index) const;

  /**
   * Set constant service rate stream for a DA service.
   *
   * \param index Index of the service
   * \param constantServiceRateStream Constant service rate stream [kbps]
   */
  void SetDaConstantServiceRateStream (uint32_t index, Ptr<RandomVariableStream> constantServiceRateStream);

  /**
   * Get maximum service rate for a DA service.
   *
   * \param index Index of the service
   * \return Maximum service rate [kbps]
   */
  uint32_t GetDaMaximumServiceRateInKbps (uint32_t index) const;

  /**
   * Set maximum service rate for a DA service.
   *
   * \param index Index of the service
   * \param maximumServiceRateKbps Maximum service rate [kbps]
   */
  void SetDaMaximumServiceRateInKbps (uint32_t index, uint32_t maximumServiceRateKbps);

  /**
   * Get minimum service rate for a DA service.
   *
   * \param index Index of the service
   * \return Minimum service rate [kbps]
   */
  uint32_t GetDaMinimumServiceRateInKbps (uint32_t index) const;

  /**
   * Set minimum service rate for a DA service.
   *
   * \param index Index of the service
   * \param minimumServiceRateKbps Minimum service rate [kbps]
   */
  void SetDaMinimumServiceRateInKbps (uint32_t index, uint32_t minimumServiceRateKbps);

  /**
   * Get maximum backlog size for a DA service.
   *
   * \param index Index of the service
   * \return Maximum backlog size [bytes]
   */
  uint32_t GetDaMaximumBacklogInBytes (uint32_t index) const;

  /**
   * Set maximum backlog size for a DA service.
   *
   * \param index Index of the service
   * \param maximumBacklogInBytes Maximum backlog size [bytes]
   */
  void SetDaMaximumBacklogInBytes (uint32_t index, uint32_t maximumBacklogInBytes);

  /**
   * Get maximum unique payload per block for a RA service.
   *
   * \param index Index of the service
   * \return Maximum unique payload per block [bytes]
   */
  uint32_t GetRaMaximumUniquePayloadPerBlock (uint32_t index) const;

  /**
   * Set maximum unique payload per block for a RA service.
   *
   * \param index Index of the service
   * \param maxUniquePayloadPerBlock Maximum unique payload per block
   */
  void SetRaMaximumUniquePayloadPerBlock (uint32_t index, uint32_t maxUniquePayloadPerBlock);

  /**
   * Get maximum consecutive block accessed  for a RA service.
   *
   * \param index Index of the service
   * \return Maximum consecutive block accessed
   */
  uint32_t GetRaMaximumConsecutiveBlockAccessed (uint32_t index) const;

  /**
   * Set maximum consecutive block accessed for a RA service.
   *
   * \param index Index of the service
   * \param maxConsecutiveBlockAccessed Maximum consecutive block accessed [bytes]
   */
  void SetRaMaximumConsecutiveBlockAccessed (uint32_t index, uint32_t maxConsecutiveBlockAccessed);

  /**
   * Get minimum idle block for a RA service.
   *
   * \param index Index of the service
   * \return Minimum idle block
   */
  uint32_t GetRaMinimumIdleBlock (uint32_t index) const;

  /**
   * Set minimum idle block for a RA service.
   *
   * \param index Index of the service
   * \param minimumIdleBlock Minimum idle block
   */
  void SetRaMinimumIdleBlock (uint32_t index, uint32_t minimumIdleBlock);

  private:
    uint32_t                                  m_dynamicRatePersistence;
    uint32_t                                  m_volumeBacklogPersistence;
    Time                                      m_defaultControlRandomizationInterval;
    uint32_t                                  m_daServiceEntryCount;
    SatLowerLayerServiceDaEntry               m_daServiceEntries[m_maxDaServiceEntries];
    uint32_t                                  m_raServiceEntryCount;
    SatLowerLayerServiceRaEntry               m_raServiceEntries[m_maxRaServiceEntries];
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
 *  - SetDaServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::SetMaximumBacklogSizeInBytes
 *  - GetDaServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::GetMaximumBacklogSizeInBytes
 *
 * \param index Index of the service which attribute access methods are defined
 */
#define SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
  inline void SetDaServ ## index ## ConstantAssignmentProvided (bool value)  \
    { return SetDaConstantAssignmentProvided (index, value); } \
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
  inline void SetDaServ ## index ## MaximumServiceRateInKbps (double value)  \
    { return SetDaMaximumServiceRateInKbps (index, value); } \
  inline double GetDaServ ## index ## MaximumServiceRateInKbps () const  \
    { return GetDaMaximumServiceRateInKbps (index); } \
  inline void SetDaServ ## index ## MinimumServiceRateInKbps (double value)  \
    { return SetDaMinimumServiceRateInKbps (index, value); } \
  inline double GetDaServ ## index ## MinimumServiceRateInKbps () const  \
    { return GetDaMinimumServiceRateInKbps (index); } \
  inline void SetDaServ ## index ## MaximumBacklogInBytes (uint32_t value)  \
    { return SetDaMaximumBacklogInBytes (index, value); } \
  inline uint32_t GetDaServ ## index ## MaximumBacklogInBytes () const  \
    { return GetDaMaximumBacklogInBytes (index); }

/**
 * SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE macro helps to define DA service entry
 * attribute access method.
 *
 *  Methods defined e.g. when index is 0.
 *
 *  - SetRaServ0MaximumUniquePayloadPerBlockInBytes, see @SatLowerLayerServiceRaEntry::SetMaximumUniquePayloadPerBlockInBytes
 *  - GetRaServ0MaximumUniquePayloadPerBlockInBytes, see @SatLowerLayerServiceRaEntry::GetMaximumUniquePayloadPerBlockInBytes
 *  - SetRaServ0MaximumConsecutiveBlockAccessedInBytes, see @SatLowerLayerServiceRaEntry::SetMaximumConsecutiveBlockAccessedInBytes
 *  - GetRaServ0MaximumConsecutiveBlockAccessedInBytes, see @SatLowerLayerServiceRaEntry::GetMaximumConsecutiveBlockAccessedInBytes
 *  - SetRaServ0MinimumIdleBlockInBytes, see @SatLowerLayerServiceRaEntry::SetMinimumIdleBlockInBytes
 *  - GetRaServ0VMinimumIdleBlockInBytes, see @SatLowerLayerServiceRaEntry::GetMinimumIdleBlockInBytes
 *
 * \param index Index of the service which attribute access methods are defined
 */
#define SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
  inline void SetRaServ ## index ## MaximumUniquePayloadPerBlock (uint32_t value)  \
    { return SetRaMaximumUniquePayloadPerBlock (index, value); } \
  inline double GetRaServ ## index ## MaximumUniquePayloadPerBlock () const  \
    { return GetRaMaximumUniquePayloadPerBlock (index); } \
  inline void SetRaServ ## index ## MaximumConsecutiveBlockAccessed (double value)  \
    { return SetRaMaximumConsecutiveBlockAccessed (index, value); } \
  inline double GetRaServ ## index ## MaximumConsecutiveBlockAccessed () const  \
    { return GetRaMaximumConsecutiveBlockAccessed (index); } \
  inline void SetRaServ ## index ## MinimumIdleBlock (uint32_t value)  \
    { return SetRaMinimumIdleBlock (index, value); } \
  inline uint32_t GetRaServ ## index ## MinimumIdleBlock () const  \
    { return GetRaMinimumIdleBlock (index); }

    SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(0);
    SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(1);
    SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(2);
    SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(3);

    SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(0);
    SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(1);
    SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(2);
    SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(3);
};

} // namespace ns3

#endif /* SATELLITE_LOWER_LAYER_SERIVICE_H */
