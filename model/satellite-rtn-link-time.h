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

#ifndef SATELLITE_RTN_LINK_TIME_H_
#define SATELLITE_RTN_LINK_TIME_H_

#include "ns3/satellite-superframe-sequence.h"

namespace ns3 {

/**
 * \ingroup satellite
 * SatRtnLinkTime is a singleton class initialized with superframe sequence. The
 * class holds information related to return link superframe timing related to both
 * NCC and UT scheduling.
*/

class SatRtnLinkTime : public SimpleRefCount<SatRtnLinkTime>
{
public:
  /**
   * Default constructor
   */
  SatRtnLinkTime ();

  /**
   * Destructor for SatRtnLinkTime
   */
  virtual ~SatRtnLinkTime ();

  /**
   * \brief Initialize with the superframe sequence
   * \param seq Superframe sequence
   */
  void Initialize (Ptr<SatSuperframeSeq> seq);

  /**
   * \brief Get superframe duration of a superframe sequence
   * \param superFrameSeqId Superframe sequence id
   * \return Superframe duration in Time
   */
  Time GetSuperFrameDuration (uint8_t superFrameSeqId) const;

  /**
   * \brief Get currently running superframe count
   * \param superFrameSeqId Superframe sequence id
   * \return Superframe count
   */
  uint32_t GetCurrentSuperFrameCount (uint8_t superFrameSeqId) const;

  /**
   * \brief Get currently running superframe start time
   * \param superFrameSeqId Superframe sequence id
   * \return Superframe start time
   */
  Time GetCurrentSuperFrameStartTime (uint8_t superFrameSeqId) const;

  /**
   * \brief Get next superframe count
   * \param superFrameSeqId Superframe sequence id
   * \return Superframe count
   */
  uint32_t GetNextSuperFrameCount (uint8_t superFrameSeqId) const;

  /**
   * \brief Get next superframe start time
   * \param superFrameSeqId Superframe sequence id
   * \return Superframe start time
   */
  Time GetNextSuperFrameStartTime (uint8_t superFrameSeqId) const;

  /**
   * \brief Get superframe transmission time for a UT with a certain timing advance
   * \param superFrameSeqId Superframe sequence d
   * \param superFrameCount Superframe count
   * \param timingAdvance A propagation delay between sender and receiver
   * \return Superframe transmission time
   */
  Time GetSuperFrameTxTime (uint8_t superFrameSeqId, uint32_t superFrameCount, Time timingAdvance) const;

  /**
   * \brief Get superframe count for current superframe with a certain timing advance
   * \param superFrameSeqId Superframe sequence d
   * \param timingAdvance A propagation delay between sender and receiver
   * \return uint32_t Superframe count
   */
  uint32_t GetCurrentSuperFrameCount (uint8_t superFrameSeqId, Time timingAdvance) const;

  /**
   * \brief Get superframe count for next superframe with a certain timing advance
   * \param superFrameSeqId Superframe sequence d
   * \param timingAdvance A propagation delay between sender and receiver
   * \return Superframe count
   */
  uint32_t GetNextSuperFrameCount (uint8_t superFrameSeqId, Time timingAdvance) const;

  /**
   * \brief Get superframe transmission time for current superframe with a certain timing advance
   * \param superFrameSeqId Superframe sequence d
   * \param timingAdvance A propagation delay between sender and receiver
   * \return Superframe transmission time
   */
  Time GetCurrentSuperFrameTxTime (uint8_t superFrameSeqId, Time timingAdvance) const;

  /**
   * \brief Get superframe transmission time for next superframe with a certain timing advance
   * \param superFrameSeqId Superframe sequence d
   * \param timingAdvance A propagation delay between sender and receiver
   * \return Superframe transmission time
   */
  Time GetNextSuperFrameTxTime (uint8_t superFrameSeqId, Time timingAdvance) const;

private:
  Ptr<SatSuperframeSeq> m_superframeSeq;
};

} // namespace ns3


#endif /* SATELLITE_RTN_LINK_TIME_H_ */
