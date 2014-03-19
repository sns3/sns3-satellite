/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 */

#ifndef SATELLITE_ADDRESS_TAG_H
#define SATELLITE_ADDRESS_TAG_H

#include <ns3/tag.h>
#include <ns3/address.h>

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements a tag that carries the MAC address of the sender
 *        of the packet.
 */
class SatAddressTag : public Tag
{
public:
  /// Creates a SatAddressTag instance with an invalid source address.
  SatAddressTag ();

  /**
   * \brief Creates a SatAddressTag instance with the given source address.
   * \param addr the source address.
   */
  SatAddressTag (Address addr);

  /**
   * \param addr the source address.
   */
  void SetSourceAddress (Address addr);

  /**
   * \return the source address.
   */
  Address GetSourceAddress () const;

  // Inherited from ObjectBase base class.
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  // Inherited from Tag base class.
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (TagBuffer buf) const;
  virtual void Deserialize (TagBuffer buf);
  virtual void Print (std::ostream &os) const;

private:
  Address m_sourceAddress;  ///< The source address.

};


} // namespace ns3


#endif /* SATELLITE_ADDRESS_TAG_H */
