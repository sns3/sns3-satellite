/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_MAC_TAG_H
#define SATELLITE_MAC_TAG_H

#include "ns3/mac48-address.h"
#include "ns3/tag.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements a tag that carries the satellite MAC specific
 * information, such as source and destination MAC address.
 */
class SatMacTag : public Tag
{
public:

  /**
   * Default constructor.
   */
  SatMacTag ();

  /**
   * Destructor for SatMacTag
   */
  ~SatMacTag ();

  /**
   * \brief Set destination MAC address
   * \param dest Destination MAC address
   */
  void SetDestAddress (Mac48Address dest);

  /**
   * \brief Get destination MAC address
   * \return Destination MAC address
   */
  Mac48Address GetDestAddress (void) const;

  /**
   * \brief Set source MAC address
   * \param source Source MAC address
   */
  void SetSourceAddress (Mac48Address source);

  /**
   * \brief Get source MAC address
   * \return Source MAC address
   */
  Mac48Address GetSourceAddress (void) const;

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
   * Get serialized size of SatMacTag
   * \return Serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serializes information to buffer from this instance of SatMacTag
   * \param i Buffer in which the information is serialized
   */
  virtual void Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatMacTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void Deserialize (TagBuffer i);

  /**
   * Print time stamp of this instance of SatMacTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

private:
  static const uint32_t ADDRESS_LENGHT = 6;

  Mac48Address   m_destAddress;
  Mac48Address   m_sourceAddress;
};

/**
 * \ingroup satellite
 * \brief SatFlowIdTag implements a tag which carries the flow identifier
 * of a packet.
 */
class SatFlowIdTag : public Tag
{
public:

  /**
   * Default constructor.
   */
  SatFlowIdTag ();

  /**
   * Destructor for SatFlowIdTag
   */
  ~SatFlowIdTag ();

  /**
   * \brief Set flow id
   * \param flowId Flow identifier
   */
  void SetFlowId (uint8_t flowId);

  /**
   * \brief Get flow identifier
   * \return Flow identifier
   */
  uint8_t GetFlowId () const;

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
   * Get serialized size of SatFlowIdTag
   * \return Serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serializes information to buffer from this instance of SatFlowIdTag
   * \param i Buffer in which the information is serialized
   */
  virtual void Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatFlowIdTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void Deserialize (TagBuffer i);

  /**
   * Print time stamp of this instance of SatFlowIdTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_flowId;
};


} // namespace ns3

#endif /* SATELLITE_MAC_TAG_H */
