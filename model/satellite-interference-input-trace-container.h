/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
#ifndef SATELLITE_INTERFERENCE_INPUT_TRACE_CONTAINER_H
#define SATELLITE_INTERFERENCE_INPUT_TRACE_CONTAINER_H

#include "satellite-base-trace-container.h"
#include "ns3/satellite-input-fstream-double-container.h"
#include "satellite-enums.h"
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for interference trace container
 */
class SatInterferenceInputTraceContainer : public SatBaseTraceContainer
{
public:

  static const uint32_t DEFAULT_RX_POWER_DENSITY_INDEX = 1;

  static const uint32_t DEFAULT_INTF_DENSITY_INDEX = 2;

  static const uint32_t DEFAULT_NUMBER_OF_COLUMNS = 3;

  typedef std::pair<Address,SatEnums::ChannelType_t> key_t;

  typedef std::map <key_t, Ptr<SatInputFileStreamDoubleContainer> > container_t;

  /**
   * \brief Constructor
   */
  SatInterferenceInputTraceContainer ();

  /**
   * \brief Destructor
   */
  virtual ~SatInterferenceInputTraceContainer ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   *  \brief Do needed dispose actions.
   */
  void DoDispose ();

  void AddNode (std::pair<Address,SatEnums::ChannelType_t> key);

  Ptr<SatInputFileStreamDoubleContainer> FindNode (key_t key);

  double GetInterferenceDensity (key_t key);

  double GetRxPowerDensity (key_t key);

private:

  void Reset ();

  /**
   *
   */
  container_t m_container;

  /**
   *
   */
  uint32_t m_index;
};

} // namespace ns3

#endif /* SATELLITE_INTERFERENCE_INPUT_TRACE_CONTAINER_H */
