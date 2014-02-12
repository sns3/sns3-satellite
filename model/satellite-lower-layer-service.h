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

#ifndef SATELLITE_LOWER_LAYER_SERIVICE_H
#define SATELLITE_LOWER_LAYER_SERIVICE_H

#include <vector>
#include "ns3/object.h"
#include "ns3/nstime.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * The SatLowerLayerService class holds information of a satellite lower layer service entry.
 */
class SatLowerLayerServiceEntry : public Object
{
public:
  SatLowerLayerServiceEntry () ;
  ~SatLowerLayerServiceEntry ();

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
  inline double GetConstantServiceRateInKbps () const {return m_constantServiceRateKbps;}

  /**
   * Set constant service rate.
   *
   * \param constantServiceRateKbps Constant service rate [kbps]
   */
  inline void SetConstantServiceRateInKbps (double constantServiceRateKbps) { m_constantServiceRateKbps = constantServiceRateKbps;}

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
  bool      m_constantAssignmentProvided;
  bool      m_rbdcAllowed;
  bool      m_volumeAllowed;
  double    m_constantServiceRateKbps;
  double    m_maximumServiceRateKbps;
  double    m_minimumServiceRateKbps;
  uint32_t  m_maximumBacklogInBytes;
};

/**
 * \ingroup satellite
 *
 * The SatLowerLayerServiceConf class holds information of all configures lower layer service entries.
 */
class SatLowerLayerServiceConf : public Object
{
public:
  static const uint32_t m_maxServices = 4;

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
   * Method to convert number to string
   * \param number number to convert as string
   * \return number as string
   */
  static std::string GetNumberAsString (uint32_t number);

  /**
   * Method to convert service index to service name.
   *
   * \param index index to convert as service name
   * \return service name
   */
  static std::string GetIndexAsServiceName(uint32_t index);

  /**
   * Get state, if constant assignment is provided for a service.
   *
   * \param index Index of the service
   * \return Is constant assignment provided [true or false]
   */
  bool GetConstantAssignmentProvided (uint32_t index) const;

  /**
   * Set state, if constant assignment is provided for a service.
   *
   * \param index Index of the service
   * \param constAssignmentProvided Constant assignment provided state [true or false]
   */
  void SetConstantAssignmentProvided (uint32_t index, bool constAssignmentProvided);

  /**
   * Get state, if RBDC is allowed for a service.
   *
   * \param index Index of the service
   * \return Is RBDC allowed [true or false]
   */
  bool GetRbdcAllowed (uint32_t index) const;

  /**
   * Set state, if RBDC is allowed for a service.
   *
   * \param index Index of the service
   * \param bdcAllowed RBDC allowed state [true or false]
   */
  void SetRbdcAllowed (uint32_t index, bool bdcAllowed);

  /**
   * Get state, if volume is allowed for a service.
   *
   * \param index Index of the service
   * \return Is volume allowed [true or false]
   */
  bool GetVolumeAllowed (uint32_t index) const;

  /**
   * Set state, if volume is allowed for a service.
   *
   * \param index Index of the service
   * \param volumeAllowed Volume allowed state [true or false]
   */
  void SetVolumeAllowed (uint32_t index, bool volumeAllowed);

  /**
   * Get constant service rate for a service.
   *
   * \param index Index of the service
   * \return Constant service rate [kbps]
   */
  double GetConstantServiceRateInKbps (uint32_t index) const;

  /**
   * Set constant service rate for a service.
   *
   * \param index Index of the service
   * \param constantServiceRateKbps Constant service rate [kbps]
   */
  void SetConstantServiceRateInKbps (uint32_t index, double constantServiceRateKbps);

  /**
   * Get maximum service rate for a service.
   *
   * \param index Index of the service
   * \return Maximum service rate [kbps]
   */
  double GetMaximumServiceRateInKbps (uint32_t index) const;

  /**
   * Set maximum service rate for a service.
   *
   * \param index Index of the service
   * \param maximumServiceRateKbps Maximum service rate [kbps]
   */
  void SetMaximumServiceRateInKbps (uint32_t index, double maximumServiceRateKbps);

  /**
   * Get minimum service rate.
   *
   * \param index Index of the service
   * \return Minimum service rate [kbps]
   */
  double GetMinimumServiceRateInKbps (uint32_t index) const;

  /**
   * Set minimum service rate for a service.
   *
   * \param index Index of the service
   * \param minimumServiceRateKbps Minimum service rate [kbps]
   */
  void SetMinimumServiceRateInKbps (uint32_t index, double minimumServiceRateKbps);

  /**
   * Get maximum backlog size.
   *
   * \param index Index of the service
   * \return Maximum backlog size [bytes]
   */
  uint32_t GetMaximumBacklogInBytes (uint32_t index) const;

  /**
   * Set maximum backlog size for a service.
   *
   * \param index Index of the service
   * \param maximumBacklogInBytes Maximum backlog size [bytes]
   */
  void SetMaximumBacklogInBytes (uint32_t index, uint32_t maximumBacklogInBytes);

  private:
    uint32_t                                m_dynamicRatePersistence;
    uint32_t                                m_volumeBacklogPersistence;
    Time                                    m_defaultControlRandomizationInterval;
    uint32_t                                m_serviceCount;
    std::vector<SatLowerLayerServiceEntry>  m_serviceContainer;
    SatLowerLayerServiceEntry               m_service[m_maxServices];
/**
 * SAT_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE macro helps to define service entry
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
 *  - SetServ0ConstantServiceRateInKbps, see @SatLowerLayerServiceEntry::SetonstantServiceRateInKbps
 *  - GetServ0ConstantServiceRateInKbps, see @SatLowerLayerServiceEntry::GetonstantServiceRateInKbps
 *  - SetServ0MaximumServiceRateInKbps, see @SatLowerLayerServiceEntry::SetMaximumServiceRateInKbps
 *  - GetServ0MaximumServiceRateInKbps, see @SatLowerLayerServiceEntry::GetMaximumServiceRateInKbps
 *  - SetServ0MinimumServiceRateInKbps, see @SatLowerLayerServiceEntry::SetMinimumServiceRateInKbps
 *  - GetServ0MinimumServiceRateInKbps, see @SatLowerLayerServiceEntry::GetMinimumServiceRateInKbps
 *  - SetServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::SetMaximumBacklogSizeInBytes
 *  - GetServ0MaximumBacklogSizeInBytes, see @SatLowerLayerServiceEntry::GetMaximumBacklogSizeInBytes
 *
 * \param index Index of the service which attribute access methods are defined
 */
#define SAT_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(index)                      \
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
    inline void SetServ ## index ## ConstantServiceRateInKbps (double value)  \
      { return SetConstantServiceRateInKbps (index, value); } \
    inline double GetServ ## index ## ConstantServiceRateInKbps () const  \
      { return GetConstantServiceRateInKbps (index); } \
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

    SAT_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(0);
    SAT_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(1);
    SAT_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(2);
    SAT_SERVICE_ATTRIBUTE_ACCESSOR_DEFINE(3);
};

} // namespace ns3

#endif /* SATELLITE_LOWER_LAYER_SERIVICE_H */
