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

#ifndef SATELLITE_ENCAP_PDU_STATUS_TAG_H
#define SATELLITE_ENCAP_PDU_STATUS_TAG_H

#include "ns3/tag.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatEncapPduStatusTag is used temporarily to tag packets with the
 * fragmentation status in the encapsulation and fragmentation process.
 * The same generic tag is used in both RLE (SatReturnLinkEncapsulator) and
 * GSE (SatGenericStreamEncapsulator). There are four possible status
 * enumerators:
 * - FULL_PDU: a complete higher layer PDU which has not been fragmented yet
 * - START_PDU: a first piece of a fragmented higher layer PDU
 * - CONTINUATION_PDU: a middle piece of a fragmented higher layer PDU (not first nor last)
 * - END_PDU: a last piece of a fragmented higher layer PDU
 */
class SatEncapPduStatusTag : public Tag
{
public:

  /**
   * Default constructor.
   */
  SatEncapPduStatusTag ();

  /**
   * \brief Set PDU status
   * \param status Status of a PDU
   */
  void SetStatus (uint8_t status);

  /**
   * \brief Get PDU status
   * \return PDU status
   */
  uint8_t GetStatus (void) const;

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
   * Get serialized size of SatEncapPduStatusTag
   * \return Serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serializes information to buffer from this instance of SatEncapPduStatusTag
   * \param i Buffer in which the information is serialized
   */
  virtual void Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatEncapPduStatusTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void Deserialize (TagBuffer i);

  /**
   * Print time stamp of this instance of SatEncapPduStatusTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Fragmentation enums
   */
  typedef enum {
    FULL_PDU          = 0,
    START_PDU         = 1,
    CONTINUATION_PDU  = 2,
    END_PDU           = 3,
    LAST_ELEMENT       = 4
  } PduStatus_t;

private:
  uint8_t m_pduStatus;
};


}; // namespace ns3

#endif // SATELLITE_ENCAP_PDU_STATUS_TAG_H
