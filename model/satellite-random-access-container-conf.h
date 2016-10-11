/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#ifndef SATELLITE_RANDOM_ACCESS_CONF_H
#define SATELLITE_RANDOM_ACCESS_CONF_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-random-access-allocation-channel.h"
#include "satellite-lower-layer-service.h"
#include "ns3/satellite-superframe-sequence.h"
#include <map>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access configuration. This class holds
 * the non-allocation channel specific random access configuration
 * in addition to the configuration objects specific for each
 * allocation channel.
 */
class SatRandomAccessConf : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatRandomAccessConf ();

  /**
   * \brief Constructor
   */
  SatRandomAccessConf (Ptr<SatLowerLayerServiceConf> llsConf, Ptr<SatSuperframeSeq> m_superframeSeq);

  /**
   * \brief Destructor
   */
  virtual ~SatRandomAccessConf ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for returning the allocation channel specific RA configuration
   * \param allocationChannel allocation channel ID
   * \return allocation channel configuration
   */
  Ptr<SatRandomAccessAllocationChannel> GetAllocationChannelConfiguration (uint32_t allocationChannel);

  /**
   * \brief Function for getting the Slotted ALOHA control randomization interval
   * \return control randomization interval
   */
  uint32_t GetSlottedAlohaControlRandomizationIntervalInMilliSeconds ()
  {
    return m_slottedAlohaControlRandomizationIntervalInMilliSeconds;
  }

  /**
   * \brief Function for setting the Slotted ALOHA control randomization interval
   * \param controlRandomizationIntervalInMilliSeconds control randomization interval in milliseconds
   */
  void SetSlottedAlohaControlRandomizationIntervalInMilliSeconds (uint32_t controlRandomizationIntervalInMilliSeconds)
  {
    m_slottedAlohaControlRandomizationIntervalInMilliSeconds = controlRandomizationIntervalInMilliSeconds;
  }

  /**
   * \brief Function for getting the number of available allocation channels
   * \return number of allocation channels
   */
  uint32_t GetNumOfAllocationChannels ()
  {
    return m_allocationChannelCount;
  }

  /**
   * \brief Function for getting the CRDSA signaling overhead in bytes
   * \return CRDSA signaling overhead in bytes
   */
  uint32_t GetCrdsaSignalingOverheadInBytes ()
  {
    return m_crdsaSignalingOverheadInBytes;
  }

  /**
   * \brief Function for getting the Slotted ALOHA signaling overhead in bytes
   * \return Slotted ALOHA signaling overhead in bytes
   */
  uint32_t GetSlottedAlohaSignalingOverheadInBytes ()
  {
    return m_slottedAlohaSignalingOverheadInBytes;
  }

  /**
   * \brief Function for checking the Slotted ALOHA variable sanity
   */
  void DoSlottedAlohaVariableSanityCheck ();

private:
  /**
   * \brief Map containing the allocation channel configurations
   */
  std::map<uint32_t,Ptr<SatRandomAccessAllocationChannel> > m_allocationChannelConf;

  /**
   * \brief Slotted ALOHA control randomization interval in milliseconds
   *
   */
  uint32_t m_slottedAlohaControlRandomizationIntervalInMilliSeconds;

  /**
   * \brief Number of available allocation channels
   */
  uint32_t m_allocationChannelCount;

  /**
   * \brief Defines the size of the CRDSA signaling overhead in bytes
   */
  uint32_t m_crdsaSignalingOverheadInBytes;

  /**
   * \brief Defines the size of the Slotted ALOHA signaling overhead in bytes
   */
  uint32_t m_slottedAlohaSignalingOverheadInBytes;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_CONF_H */
