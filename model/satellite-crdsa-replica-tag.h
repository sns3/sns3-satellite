/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */

#ifndef SATELLITE_CRDSA_REPLICA_TAG_H
#define SATELLITE_CRDSA_REPLICA_TAG_H

#include "ns3/tag.h"
#include <vector>

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements a tag that carries information
 * about the slot IDs of CRDSA packet replicas. This information
 * is used to determine the CRDSA replicas within the CRDSA frame.
 * The tag is added to each created CRDSA packet on the
 * transmitter side and read from the CRDSA packet on the
 * receiver side.
 */
class SatCrdsaReplicaTag : public Tag
{
public:

  /**
   * Default constructor.
   */
  SatCrdsaReplicaTag ();

  /**
   * Destructor for SatCrdsaReplicaTag
   */
  ~SatCrdsaReplicaTag ();

  void AddSlotId (uint16_t slotId);
  std::vector<uint16_t> GetSlotIds (void);

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
   * Get serialized size of SatCrdsaReplicaTag
   * \return Serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serializes information to buffer from this instance of SatCrdsaReplicaTag
   * \param buffer Buffer in which the information is serialized
   */
  virtual void Serialize (TagBuffer buffer) const;

  /**
   * Deserializes information from buffer to this instance of SatCrdsaReplicaTag
   * \param buffer Buffer from which the information is deserialized
   */
  virtual void Deserialize (TagBuffer buffer);

  /**
   * Print time stamp of this instance of SatCrdsaReplicaTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

private:
  std::vector<uint16_t> m_slotIds;
  uint8_t m_numOfIds;
};

} // namespace ns3

#endif /* SATELLITE_CRDSA_REPLICA_TAG_H */
