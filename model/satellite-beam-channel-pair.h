/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */


#ifndef SATELLITE_BEAM_CHANNEL_PAIR_H
#define SATELLITE_BEAM_CHANNEL_PAIR_H

#include <map>

#include <ns3/object.h>

#include "satellite-channel.h"
#include "satellite-typedefs.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * Container of Satellite channels pairs. As a Satellite channel is mapped into
 * a frequency band and act as data transmitter in a single direction, a pair of
 * Satellite channels represent a bidirectionnal link between either an UT and
 * the Satellite or a GW and the Satellite.
 *
 * The main task of the SatChannelPair is to store such bidirectionnal link
 * information on a per-beam basis.
 *
 */

class SatChannelPair : public Object
{
public:
  typedef std::pair<Ptr<SatChannel>, Ptr<SatChannel> >  ChannelPair_t;    //forward = first, return  = second

  /**
   * Default constructor.
   */
  SatChannelPair ();

  /**
   * Destructor for SatChannel
   */
  virtual ~SatChannelPair ();

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Retrieve the channel pair associated to a beam
   * \param beamId the ID of the beam
   * \return a pair of SatChannel that has been associated to this beam
   */
  ChannelPair_t GetChannelPair (uint32_t beamId) const;
  Ptr<SatChannel> GetForwardChannel (uint32_t frequencyId) const;
  Ptr<SatChannel> GetReturnChannel (uint32_t frequencyId) const;

  /**
   * \brief Test if a channel pair has been stored for a given color
   * \param frequencyId the ID of the color
   * \return whether or not this color already stores a pair of SatChannel
   */
  // bool HasChannelPair (uint32_t frequencyId);
  bool HasFwdChannel (uint32_t frequencyId) const;
  bool HasRtnChannel (uint32_t frequencyId) const;

  /**
   * \brief Associate a new beam to a given color
   * \param beamId the ID of the beam
   * \param frequencyId the ID of the color
   */
  // void UpdateBeamsForFrequency (uint32_t beamdId, uint32_t frequencyId);
  void UpdateBeamsForFrequency (uint32_t beamdId, uint32_t fwdFrequencyId, uint32_t rtnFrequencyId);

  /**
   * \brief Store a pair of SatChannel for the given color
   * and associate the given beam to said color
   * \param beamId the ID of the beam
   * \param frequencyId the ID of the color
   * \param channels the SatChannel pair to store
   */
  // void StoreChannelPair (uint32_t beamId, uint32_t frequencyId, ChannelPair_t channels);
  void StoreChannelPair (uint32_t beamId,
                         uint32_t fwdFrequencyId,
                         Ptr<SatChannel> fwdChannel,
                         uint32_t rtnFrequencyId,
                         Ptr<SatChannel> rtnChannel);

private:
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > m_frequencies;   // map from beam ID to frequency ID
  std::map<uint32_t, Ptr<SatChannel> > m_fwdChannels;
  std::map<uint32_t, Ptr<SatChannel> > m_rtnChannels;
};

}

#endif /* SATELLITE_BEAM_CHANNEL_PAIR_H */
