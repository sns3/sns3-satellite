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

  SatMacTag ();
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

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
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

  SatFlowIdTag ();
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

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_flowId;
};


} // namespace ns3

#endif /* SATELLITE_MAC_TAG_H */
