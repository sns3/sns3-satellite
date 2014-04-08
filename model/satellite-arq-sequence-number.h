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

#ifndef SATELLITE_ARQ_SEQUENCE_NUMBER_H_
#define SATELLITE_ARQ_SEQUENCE_NUMBER_H_

#include <map>
#include "ns3/simple-ref-count.h"

/**
 * \ingroup satellite
 *
 * \brief SatArqSequenceNumber
 */
namespace ns3 {

class SatArqSequenceNumber : public SimpleRefCount<SatArqSequenceNumber>
{
public:

  SatArqSequenceNumber ();
  SatArqSequenceNumber (uint8_t windowSize);

  virtual ~SatArqSequenceNumber () {};

  /**
   * Check whether there are free (released) sequence numbers
   *  \return bool Boolean to indicate whether there are seq no available
   */
  bool SeqNoAvailable () const;

  /**
   * Returns the next free sequence number
   * \return uint32_t Next free sequence number
   */
  uint8_t NextSequenceNumber ();

  /**
   * Release a sequence number if either ACK is received or
   * maximum retransmissions have been reached.
   * \param seqNo Sequence number to be released
   */
  void Release (uint8_t seqNo);

private:

  /**
   * Clean ups the sequence number map
   */
  void CleanUp ();

  std::map<uint32_t, bool> m_seqNoMap;
  uint32_t m_currSeqNo;
  uint32_t m_windowSize;
  uint32_t m_maxSn;

};


} // namespace

#endif /* SATELLITE_ARQ_SEQUENCE_NUMBER_H_ */
