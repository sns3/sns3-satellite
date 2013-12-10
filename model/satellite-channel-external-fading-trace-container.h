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


#ifndef SATELLITE_CHANNEL_EXTERNAL_FADING_TRACE_CONTAINER_H_
#define SATELLITE_CHANNEL_EXTERNAL_FADING_TRACE_CONTAINER_H_

#include <map>
#include "ns3/object.h"
#include "satellite-channel-external-fading-trace.h"
#include "satellite-channel.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Satellite channel external fading trace container
 */
class SatChannelExternalFadingTraceContainer : public Object
{
public:

  typedef std::pair<Ptr<SatChannelExternalFadingTrace>, Ptr<SatChannelExternalFadingTrace> >  ChannelTracePair_t;

  static TypeId GetTypeId (void);

  SatChannelExternalFadingTraceContainer ();
  SatChannelExternalFadingTraceContainer (uint32_t numUts, uint32_t numGws);

  ~SatChannelExternalFadingTraceContainer ();

  /**
   * Get method for getting a proper fading trace
   * \return Channel fading trace for a certain node and channel.
   */
  Ptr<SatChannelExternalFadingTrace> GetFadingTrace (uint32_t nodeId, SatEnums::ChannelType_t channelType) const;

  /**
   * A method to test that the fading traces are according to
   * assumptions.
   * \return boolean value indicating success or failure
   */
  bool TestFadingTraces () const;

private:

  /**
   * Create the UT fading traces for a certain number of UTs (defined by
   * the creator of this class).
   */
  void CreateUtFadingTraces (uint32_t numUts);

  /**
   * Create the GW fading traces for a certain number of GWs (defined by
   * the creator of this class).
   */
  void CreateGwFadingTraces (uint32_t numGws);

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


#endif /* SATELLITE_CHANNEL_EXTERNAL_FADING_TRACE_CONTAINER_H_ */
