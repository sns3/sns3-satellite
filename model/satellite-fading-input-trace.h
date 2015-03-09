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
#ifndef SATELLITE_FADING_INPUT_TRACE_H
#define SATELLITE_FADING_INPUT_TRACE_H

#include "satellite-base-fading.h"
#include "ns3/satellite-fading-input-trace-container.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for fading input trace. The class implements the fading
 * interface and provides access to the container of input fading files.
 */
class SatFadingInputTrace : public SatBaseFading
{
public:
  /**
   * \brief Constructor
   */
  SatFadingInputTrace ();

  /**
   * \brief Constructor
   */
  SatFadingInputTrace (Ptr<SatFadingInputTraceContainer> satFadingInputTraceContainer);

  /**
   * \brief Destructor
   */
  ~SatFadingInputTrace ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for getting the fading value
   * \param macAddress MAC address
   * \param channelType channel type
   * \return fading value
   */
  double DoGetFading (Address macAddress, SatEnums::ChannelType_t channelType);

private:
  /**
   * \brief Pointer to input trace container
   */
  Ptr<SatFadingInputTraceContainer> m_satFadingInputTraceContainer;
};

} // namespace ns3

#endif /* SATELLITE_FADING_INPUT_TRACE_H */
