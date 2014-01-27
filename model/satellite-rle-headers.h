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

#ifndef SATELLITE_RLE_HEADERS_H
#define SATELLITE_RLE_HEADERS_H


#include <vector>
#include "ns3/header.h"

/**
 * \ingroup satellite
 *
 * \brief SatPPduHeader (payload adapted PDU) and SatFPduHeader
 * (frame PDU) implementations.
 */
namespace ns3 {

class SatPPduHeader : public Header
{
public:

  /**
   * Constructor
   */
  SatPPduHeader ();
  ~SatPPduHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  /**
   * Get start indicator of PPDU header
   * \return uint8_t start indicator
   */
  uint8_t GetStartIndicator () const;

  /**
   * Get end indicator of PPDU header
   * \return uint8_t end indicator
   */
  uint8_t GetEndIndicator () const;

  /**
   * Get PPDU fragment length in bytes
   * \return uint32_t PPDU length
   */
  uint16_t GetPPduLength () const;

  /**
   * Get PPDU fragment id
   * \return uint32_t fragment id
   */
  uint8_t GetFragmentId () const;

  /**
   * Get total length of higher layer PDU. Set in
   * START_PPDU status type.
   * \return uint32_t total length of HL PDU
   */
  uint16_t GetTotalLength () const;

  /**
   * Set start indicator to PPDU header
   */
  void SetStartIndicator ();

  /**
   * Set end indicator to PPDU header
   */
  void SetEndIndicator ();

  /**
   * Set PPDU fragment length to PPDU header
   * \param bytes PPDU length in bytes
   */
  void SetPPduLength (uint16_t bytes);

  /**
   * Set fragment id to PPDU header
   * \param id Fragment id
   */
  void SetFragmentId (uint8_t id);

  /**
   * Set total length of higher layer PDU. Set in
   * START_PPDU status type.
   */
  void SetTotalLength (uint16_t bytes);

  /**
   * Get the maximum RLE header size
   * \return uint32_t header size
   */
  uint32_t GetHeaderSizeInBytes (uint8_t type) const;

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

class SatFPduHeader : public Header
{
public:

  /**
   * Constructor
   */
  SatFPduHeader ();
  ~SatFPduHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  /**
   * Push PPDU length to header
   * \param size PPDU length in bytes
   */
  void PushPPduLength (uint32_t size);

  /**
   * Get number of PPDUs inside FPDU
   * \return uint8_t number of PPDUs
   */
  uint8_t GetNumPPdus () const;

  /**
   * Get PPDU length of certain index
   * \param index PPDU index
   * \return uint32_t PPDU length in bytes
   */
  uint32_t GetPPduLength (uint32_t index) const;

private:

  uint8_t m_numPPdus;
  std::vector<uint32_t> m_ppduSizesInBytes;
};

}; // namespace ns3

#endif // SATELLITE_RLE_HEADERS_H
