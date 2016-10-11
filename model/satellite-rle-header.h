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

#ifndef SATELLITE_RLE_HEADER_H
#define SATELLITE_RLE_HEADER_H


#include <vector>
#include "ns3/header.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatPPduHeader implementation. RLE header is added to each
 * of the transmitted RLE packets at the UT in RTN link direction. The header
 * is detached and interpreted at the RLE receiver at the GW. Based on the
 * content the receiver can do the defragmentation of the higher layer packet.
 * The size of the RLE header is determined by the Serialize and GetSerializedSize
 * methods. The RLE header has the proper content and size according to
 * specification.
 *
 * \see ETSI TS 103 179 V1.1.1 Satellite Earth Stations and Systems (SES);
 * Return Link Encapsulation (RLE) protocol
 */
class SatPPduHeader : public Header
{
public:
  /**
   * Constructor
   */
  SatPPduHeader ();

  /**
   * Destructor for SatPPduHeader
   */
  ~SatPPduHeader ();

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
   * Get serialized size of SatPPduHeader
   * \return Serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serializes information to buffer from this instance of SatPPduHeader
   * \param start Buffer in which the information is serialized
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * Deserializes information from buffer to this instance of SatPPduHeader
   * \param start Buffer from which the information is deserialized
   * \return Serialized size of the buffer
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Print time stamp of this instance of SatPPduHeader
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * \brief Get start indicator of PPDU header
   * \return Start indicator
   */
  uint8_t GetStartIndicator () const;

  /**
   * \brief Get end indicator of PPDU header
   * \return End indicator
   */
  uint8_t GetEndIndicator () const;

  /**
   * \brief Get PPDU fragment length in bytes
   * \return PPDU length in bytes
   */
  uint16_t GetPPduLength () const;

  /**
   * \brief Get PPDU fragment id
   * \return Fragment id
   */
  uint8_t GetFragmentId () const;

  /**
   * \brief Get total length of higher layer PDU. Set in
   * START_PPDU status type.
   * \return Total length of HL PDU
   */
  uint16_t GetTotalLength () const;

  /**
   * \brief Set start indicator to PPDU header
   */
  void SetStartIndicator ();

  /**
   * \brief Set end indicator to PPDU header
   */
  void SetEndIndicator ();

  /**
   * \brief Set PPDU fragment length to PPDU header
   * \param bytes PPDU length in bytes
   */
  void SetPPduLength (uint16_t bytes);

  /**
   * \brief Set fragment id to PPDU header
   * \param id Fragment id
   */
  void SetFragmentId (uint8_t id);

  /**
   * Set total length of higher layer PDU. Set in
   * START_PPDU status type.
   * \param bytes Total HL packet size
   */
  void SetTotalLength (uint16_t bytes);

  /**
   * \brief Get the maximum RLE header size
   * \param type Header type
   * \return Header size
   */
  uint32_t GetHeaderSizeInBytes (uint8_t type) const;

  /**
   * \brief Get maximum RLE header size
   * \return Maximum header size
   */
  uint32_t GetMaxHeaderSizeInBytes () const;

private:
  uint8_t m_startIndicator;
  uint8_t m_endIndicator;
  uint16_t m_ppduLengthInBytes;
  uint8_t m_fragmentId;
  uint16_t m_totalLengthInBytes;

  /**
   * Constant variables for determining the header sizes
   * of different RLE PPDU fragments.
   */
  const uint32_t m_fullPpduHeaderSize;
  const uint32_t m_startPpduHeaderSize;
  const uint32_t m_endPpduHeaderSize;
  const uint32_t m_continuationPpduHeaderSize;

};


}; // namespace ns3

#endif // SATELLITE_RLE_HEADER_H
