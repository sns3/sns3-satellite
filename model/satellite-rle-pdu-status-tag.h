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

#ifndef SATELLITE_RLE_PDU_STATUS_TAG_H
#define SATELLITE_RLE_PDU_STATUS_TAG_H

#include "ns3/tag.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatRlePduStatusTag is used to tag Payload adapted
 * PDUs in fragmentation.
 */
class SatRlePduStatusTag : public Tag
{
public:
  SatRlePduStatusTag ();

  /**
   * Set PPDU status
   * \return status PpduStatus_t
   */
  void SetStatus (uint8_t status);

  /**
   * Get PPDU status
   * \return uint8_t PPDU status
   */
  uint8_t GetStatus (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * Enums for Payload adapted PDU fragmentation
   */
  typedef enum { FULL_PPDU          = 0,
                 START_PPDU         = 1,
                 CONTINUATION_PPDU  = 2,
                 END_PPDU           = 3,
                 LAST_ELEMENT       = 4 } PpduStatus_t;

private:
  uint8_t m_ppduStatus;
};


}; // namespace ns3

#endif // SATELLITE_ENCAPSULATION_STATUS_TAG_H
