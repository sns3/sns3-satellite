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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */


#ifndef SATELLITE_FADING_EXTERNAL_INPUT_TRACE_CONTAINER_H_
#define SATELLITE_FADING_EXTERNAL_INPUT_TRACE_CONTAINER_H_

#include <map>
#include "ns3/object.h"
#include "satellite-fading-external-input-trace.h"
#include "satellite-channel.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Satellite fading external input trace container
 */
class SatFadingExternalInputTraceContainer : public Object
{
public:

  typedef std::pair<Ptr<SatFadingExternalInputTrace>, Ptr<SatFadingExternalInputTrace> >  ChannelTracePair_t;

  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  SatFadingExternalInputTraceContainer ();

  /**
   * \brief Destructor
   */
  ~SatFadingExternalInputTraceContainer ();

  /**
   * Get method for getting a proper fading trace
   * \return Channel fading trace for a certain node and channel.
   */
  Ptr<SatFadingExternalInputTrace> GetFadingTrace (uint32_t nodeId, SatEnums::ChannelType_t channelType);

  /**
   * \brief A method to test that the fading traces are according to
   * assumptions.
   * \param numOfUts number of UTs to test
   * \param numOfGws number of GWs to test
   * \return boolean value indicating success or failure
   */
  bool TestFadingTraces (uint32_t numOfUts, uint32_t numOfGws);

private:

  /**
   * Create new UT fading trace
   */
  void CreateUtFadingTrace (uint32_t utId);

  /**
   * Create new GW fading trace
   */
  void CreateGwFadingTrace (uint32_t gwId);

  /**
   * Container of the UT fading traces
   */
  std::map< uint32_t, ChannelTracePair_t> m_utFadingMap;

  /**
   * Container of the GW fading traces
   */
  std::map< uint32_t, ChannelTracePair_t> m_gwFadingMap;
};

} // namespace ns3


#endif /* SATELLITE_FADING_EXTERNAL_INPUT_TRACE_CONTAINER_H_ */
