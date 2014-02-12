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
  inline double GetConstantServiceRateInKbps () const {return m_constantServiceRateStream->GetValue ();}

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
  inline double GetMaximumServiceRateInKbps () const {return m_maximumServiceRateKbps;}

  /**
   * Set maximum service rate.
   *
   * \param maximumServiceRateKbps Maximum service rate [kbps]
   */
  inline void SetMaximumServiceRateInKbps (double maximumServiceRateKbps) { m_maximumServiceRateKbps = maximumServiceRateKbps;}

  /**
   * Get minimum service rate.
   *
   * \return Minimum service rate [kbps]
   */
  inline double GetMinimumServiceRateInKbps () const {return m_minimumServiceRateKbps;}

  /**
   * Set minimum service rate.
   *
   * \param minimumServiceRateKbps Minimum service rate [kbps]
   */
  inline void SetMinimumServiceRateInKbps (double minimumServiceRateKbps) { m_minimumServiceRateKbps = minimumServiceRateKbps;}

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
   * \return Maximum unique payload per block [bytes]
   */
  inline uint32_t GetMaximumUniquePayloadPerBlockInBytes () const {return m_maxUniquePayloadPerBlockInBytes;}

  /**
   * Set maximum unique payload per block.
   *
   * \param maxUniquePayloadPerBlockInBytes Maximum unique payload per block [bytes]
   */
  inline void SetMaximumUniquePayloadPerBlockInBytes (uint32_t maxUniquePayloadPerBlockInBytes) { m_maxUniquePayloadPerBlockInBytes = maxUniquePayloadPerBlockInBytes;}

  /**
   * Get maximum consecutive block accessed.
   *
   * \return Maximum consecutive block accessed [bytes]
   */
  inline uint32_t GetMaximumConsecutiveBlockAccessedInBytes () const {return m_maxConsecutiveBlockAccessedInBytes;}

  /**
   * Set maximum consecutive block accessed.
   *
   * \param maxConsecutiveBlockAccessedInBytes Maximum consecutive block accessed [bytes]
   */
  inline void SetMaximumConsecutiveBlockAccessedInBytes (uint32_t maxConsecutiveBlockAccessedInBytes) { m_maxConsecutiveBlockAccessedInBytes = maxConsecutiveBlockAccessedInBytes;}

  /**
   * Get minimum idle block.
   *
   * \return Minimum idle block [bytes]
   */
  inline uint32_t GetMinimumIdleBlockInBytes () const {return m_minimumIdleBlockInBytes;}

  /**
   * Set minimum idle block.
   *
   * \param minimumIdleBlockInBytes Minimum idle block [bytes]
   */
  inline void SetMinimumIdleBlockInBytes (uint32_t minimumIdleBlockInBytes) { m_minimumIdleBlockInBytes = minimumIdleBlockInBytes;}

private:
  uint32_t  m_maxUniquePayloadPerBlockInBytes;
  uint32_t  m_maxConsecutiveBlockAccessedInBytes;
  uint32_t  m_minimumIdleBlockInBytes;
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
   * Method to convert service index to service name.
   *
   * \param index index to convert as service name
   * \return service name
   */
  static std::string GetIndexAsServiceName(uint32_t index);

  /**
   * Get state, if constant assignment is provided for a DA service.
   *
   * \param index Index of the service
   * \return Is constant assignment provided [true or false]
   */
  bool GetConstantAssignmentProvided (uint32_t index) const;

  /**
   * Set state, if constant assignment is provided for a DA service.
   *
   * \param index Index of the service
   * \param constAssignmentProvided Constant assignment provided state [true or false]
   */
  void SetConstantAssignmentProvided (uint32_t index, bool constAssignmentProvided);

  /**
   * Get state, if RBDC is allowed for a DA service.
   *
   * \param index Index of the service
   * \return Is RBDC allowed [true or false]
   */
  bool GetRbdcAllowed (uint32_t index) const;

  /**
   * Set state, if RBDC is allowed for a DA service.
   *
   * \param index Index of the service
   * \param bdcAllowed RBDC allowed state [true or false]
   */
  void SetRbdcAllowed (uint32_t index, bool bdcAllowed);

  /**
   * Get state, if volume is allowed for a DA service.
   *
   * \param index Index of the service
   * \return Is volume allowed [true or false]
   */
  bool GetVolumeAllowed (uint32_t index) const;

  /**
   * Set state, if volume is allowed for a DA service.
   *
   * \param index Index of the service
   * \param volumeAllowed Volume allowed state [true or false]
   */
  void SetVolumeAllowed (uint32_t index, bool volumeAllowed);

  /**
   * Get constant service rate stream for a DA service.
   *
   * \param index Index of the service
   * \return Constant service rate stream [kbps]
   */
  Ptr<RandomVariableStream> GetConstantServiceRateStream (uint32_t index) const;

  /**
   * Set constant service rate stream for a DA service.
   *
   * \param index Index of the service
   * \param constantServiceRateStream Constant service rate stream [kbps]
   */
  void SetConstantServiceRateStream (uint32_t index, Ptr<RandomVariableStream> constantServiceRateStream);

  /**
   * Get maximum service rate for a DA service.
   *
   * \param index Index of the service
   * \return Maximum service rate [kbps]
   */
  double GetMaximumServiceRateInKbps (uint32_t index) const;

  /**
   * Set maximum service rate for a DA service.
   *
   * \param index Index of the service
   * \param maximumServiceRateKbps Maximum service rate [kbps]
   */
  void SetMaximumServiceRateInKbps (uint32_t index, double maximumServiceRateKbps);

  /**
   * Get minimum service rate for a DA service.
   *
   * \param index Index of the service
   * \return Minimum service rate [kbps]
   */
  double GetMinimumServiceRateInKbps (uint32_t index) const;

  /**
   * Set minimum service rate for a DA service.
   *
   * \param index Index of the service
   * \param minimumServiceRateKbps Minimum service rate [kbps]
   */
  void SetMinimumServiceRateInKbps (uint32_t index, double minimumServiceRateKbps);

  /**
   * Get maximum backlog size for a DA service.
   *
   * \param index Index of the service
   * \return Maximum backlog size [bytes]
   */
  uint32_t GetMaximumBacklogInBytes (uint32_t index) const;

  /**
   * Set maximum backlog size for a DA service.
   *
   * \param index Index of the service
   * \param maximumBacklogInBytes Maximum backlog size [bytes]
   */
  void SetMaximumBacklogInBytes (uint32_t index, uint32_t maximumBacklogInBytes);

  /**
   * Get maximum unique payload per block for a RA service.
   *
   * \param index Index of the service
   * \return Maximum unique payload per block [bytes]
   */
  uint32_t GetMaximumUniquePayloadPerBlockInBytes (uint32_t index) const;

  /**
   * Set maximum unique payload per block for a RA service.
   *
   * \param index Index of the service
   * \param maxUniquePayloadPerBlockInBytes Maximum unique payload per block [bytes]
   */
  void SetMaximumUniquePayloadPerBlockInBytes (uint32_t index, uint32_t maxUniquePayloadPerBlockInBytes);

  /**
   * Get maximum consecutive block accessed  for a RA service.
   *
   * \param index Index of the service
   * \return Maximum consecutive block accessed [bytes]
   */
  uint32_t GetMaximumConsecutiveBlockAccessedInBytes (uint32_t index) const;

  /**
   * Set maximum consecutive block accessed for a RA service.
   *
   * \param index Index of the service
   * \param maxConsecutiveBlockAccessedInBytes Maximum consecutive block accessed [bytes]
   */
  void SetMaximumConsecutiveBlockAccessedInBytes (uint32_t index, uint32_t maxConsecutiveBlockAccessedInBytes);

  /**
   * Get minimum idle block for a RA service.
   *
   * \param index Index of the service
   * \return Minimum idle block [bytes]
   */
  uint32_t GetMinimumIdleBlockInBytes (uint32_t index) const;

  /**
   * Set minimum idle block for a RA service.
   *
   * \param index Index of the service
   * \param minimumIdleBlockInBytes Minimum idle block [bytes]
   */
  void SetMinimumIdleBlockInBytes (uint32_t index, uint32_t minimumIdleBlockInBytes);

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
 *  - SetServ0ConstantAssignmentProvided, see @SatLowerLayerServiceEntry::SetConstantAssignmentProvided
 *  - GetServ0ConstantAssignmentProvided, see @SatLowerLayerServiceEntry::GetConstantAssignmentProvided
 *  - SetServ0RbdcAllowed, see @SatLowerLayerServiceEntry::SetRbdcAllowed
 *  - GetServ0RbdcAllowed, see @SatLowerLayerServiceEntry::GetRbdcAllowed
 *  - SetServ0VolumeAllowed, see @SatLowerLayerServiceEntry::SetVolumeAllowed
 *  - GetServ0VolumeAllowed, see @SatLowerLayerServiceEntry::GetVolumeAllowed
 *  - SetServ0ConstantServiceRateStream, see @SatLowerLayerServiceEntry::SetonstantServiceRateStream
 *  - GetServ0ConstantServiceRateStream, see @SatLowerLayerServiceEntry::GetonstantServiceRateStream
 *  - SetServ0MaximumServiceRateInKbps, see @SatLowerLayerServiceEntry::SetMaximumServiceRateInKbps
 *  - GetServ0MaximumServiceRateInKbps, see @SatLowerLayerServiceEntry::GetMaximumServiceRateInKbps
 *  - SetServ0MinimumServiceRateInKbps, see @SatLowerLayerServiceEntry::SetMinimumServiceRateInKbps
 *  - GetServ0MinimumServiceRateInKbps, see @SatLowerLayerServiceEntry::GetMinimumServiceRateInKbps
 *  - SetServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::SetMaximumBacklogSizeInBytes
 *  - GetServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::GetMaximumBacklogSizeInBytes
 *
 * \param index Index of the service which attribute access methods are defined
 */
#define SAT_DA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
  inline void SetServ ## index ## ConstantAssignmentProvided (bool value)  \
    { return SetConstantAssignmentProvided (index, value); } \
  inline bool GetServ ## index ## ConstantAssignmentProvided () const  \
    { return GetConstantAssignmentProvided (index); } \
  inline void SetServ ## index ## RbdcAllowed (bool value)  \
    { return SetRbdcAllowed (index, value); } \
  inline bool GetServ ## index ## RbdcAllowed () const  \
    { return GetRbdcAllowed (index); } \
  inline void SetServ ## index ## VolumeAllowed (bool value)  \
    { return SetVolumeAllowed (index, value); } \
  inline bool GetServ ## index ## VolumeAllowed () const  \
    { return GetVolumeAllowed (index); } \
  inline void SetServ ## index ## ConstantServiceRateStream (Ptr<RandomVariableStream> value)  \
    { return SetConstantServiceRateStream (index, value); } \
  inline Ptr<RandomVariableStream> GetServ ## index ## ConstantServiceRateStream () const  \
    { return GetConstantServiceRateStream (index); } \
  inline void SetServ ## index ## MaximumServiceRateInKbps (double value)  \
    { return SetMaximumServiceRateInKbps (index, value); } \
  inline double GetServ ## index ## MaximumServiceRateInKbps () const  \
    { return GetMaximumServiceRateInKbps (index); } \
  inline void SetServ ## index ## MinimumServiceRateInKbps (double value)  \
    { return SetMinimumServiceRateInKbps (index, value); } \
  inline double GetServ ## index ## MinimumServiceRateInKbps () const  \
    { return GetMinimumServiceRateInKbps (index); } \
  inline void SetServ ## index ## MaximumBacklogInBytes (uint32_t value)  \
    { return SetMaximumBacklogInBytes (index, value); } \
  inline uint32_t GetServ ## index ## MaximumBacklogInBytes () const  \
    { return GetMaximumBacklogInBytes (index); }

/**
 * SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE macro helps to define DA service entry
 * attribute access method.
 *
 *  Methods defined e.g. when index is 0.
 *
 *  - SetServ0MaximumUniquePayloadPerBlockInBytes, see @SatLowerLayerServiceRaEntry::SetMaximumUniquePayloadPerBlockInBytes
 *  - GetServ0MaximumUniquePayloadPerBlockInBytes, see @SatLowerLayerServiceRaEntry::GetMaximumUniquePayloadPerBlockInBytes
 *  - SetServ0MaximumConsecutiveBlockAccessedInBytes, see @SatLowerLayerServiceRaEntry::SetMaximumConsecutiveBlockAccessedInBytes
 *  - GetServ0MaximumConsecutiveBlockAccessedInBytes, see @SatLowerLayerServiceRaEntry::GetMaximumConsecutiveBlockAccessedInBytes
 *  - SetServ0MinimumIdleBlockInBytes, see @SatLowerLayerServiceRaEntry::SetMinimumIdleBlockInBytes
 *  - GetServ0VMinimumIdleBlockInBytes, see @SatLowerLayerServiceRaEntry::GetMinimumIdleBlockInBytes
 *
 * \param index Index of the service which attribute access methods are defined
 */
#define SAT_RA_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
  inline void SetServ ## index ## MaximumUniquePayloadPerBlockInBytes (double value)  \
    { return SetMaximumUniquePayloadPerBlockInBytes (index, value); } \
  inline double GetServ ## index ## MaximumUniquePayloadPerBlockInBytes () const  \
    { return GetMaximumUniquePayloadPerBlockInBytes (index); } \
  inline void SetServ ## index ## MaximumConsecutiveBlockAccessedInBytes (double value)  \
    { return SetMaximumConsecutiveBlockAccessedInBytes (index, value); } \
  inline double GetServ ## index ## MaximumConsecutiveBlockAccessedInBytes () const  \
    { return GetMaximumConsecutiveBlockAccessedInBytes (index); } \
  inline void SetServ ## index ## MinimumIdleBlockInBytes (uint32_t value)  \
    { return SetMinimumIdleBlockInBytes (index, value); } \
  inline uint32_t GetServ ## index ## MinimumIdleBlockInBytes () const  \
    { return GetMinimumIdleBlockInBytes (index); }

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
