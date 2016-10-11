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

#ifndef SATELLITE_ARQ_HEADER_H
#define SATELLITE_ARQ_HEADER_H


#include <vector>
#include "ns3/header.h"


namespace ns3 {


/**
 * \ingroup satellite
 *
 * \brief ARQ header implementation contains the sequence number related
 * to the packet in question. Sequence number is identified with one byte, thus
 * it may range between 0 - 255.
 */

class SatArqHeader : public Header
{
public:
  /**
   * Constructor
   */
  SatArqHeader ();

  /**
   * Destructor for SatArqHeader
   */
  ~SatArqHeader ();

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get serialized size of SatArqHeader
   * \return Serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serializes information to buffer from this instance of SatArqHeader
   * \param start Buffer in which the information is serialized
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * Deserializes information from buffer to this instance of SatArqHeader
   * \param start Buffer from which the information is deserialized
   * \return Serialized size of the buffer
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Print time stamp of this instance of SatArqHeader
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * \brief Get sequence number
   * \return Sequence number
   */
  uint8_t GetSeqNo () const;

  /**
   * \brief Set sequence number
   * \param seqNo Sequence number
   */
  void SetSeqNo (uint8_t seqNo);

private:
  /**
   * Sequence number represented by one Byte
   */
  uint8_t m_seqNo;

};


}; // namespace ns3

#endif // SATELLITE_ARQ_HEADER_H
