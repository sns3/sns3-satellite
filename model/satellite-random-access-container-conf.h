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
#include <map>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access configuration
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
  SatRandomAccessConf (Ptr<SatLowerLayerServiceConf> llsConf);

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
   *
   * \return
   */
  Ptr<SatRandomAccessAllocationChannel> GetAllocationChannelConfiguration (uint32_t allocationChannel);

  /**
   *
   * \return
   */
  uint32_t GetSlottedAlohaControlRandomizationInterval () { return slottedAlohaControlRandomizationInterval;}

  /**
   *
   * \param controlRandomizationInterval
   */
  void SetSlottedAlohaControlRandomizationInterval (uint32_t controlRandomizationInterval) { slottedAlohaControlRandomizationInterval = controlRandomizationInterval;}

  /**
   *
   * \return
   */
  uint32_t GetNumOfAllocationChannels () { return m_allocationChannelCount; }

  /**
   *
   */
  void DoSlottedAlohaVariableSanityCheck ();

private:

  /**
   *
   */
  std::map<uint32_t,Ptr<SatRandomAccessAllocationChannel> > m_allocationChannelConf;

  /**
   * \brief
   * in milliseconds
   */
  uint32_t slottedAlohaControlRandomizationInterval;

  /**
   *
   */
  uint32_t m_allocationChannelCount;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_CONF_H */
