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
 * Author: Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_GROUND_STATION_ADDRESS_TAG_H
#define SATELLITE_GROUND_STATION_ADDRESS_TAG_H

#include <ns3/tag.h>
#include <ns3/mac48-address.h>


namespace ns3 {

/**
 * \ingroup satellite
 * \brief Tag to store ground station destination address. Use for routing in constellations
 */
class SatGroundStationAddressTag : public Tag
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId  GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId  GetInstanceTypeId (void) const;

  /**
   * Default constructor.
   */
  SatGroundStationAddressTag ();

  /**
   * Constructor with initialization parameters.
   * \param groundStationAddress
   */
  SatGroundStationAddressTag (Mac48Address groundStationAddress);

  /**
   * Serializes information to buffer from this instance of SatGroundStationAddressTag
   * \param i Buffer in which the information is serialized
   */
  virtual void  Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatGroundStationAddressTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void  Deserialize (TagBuffer i);

  /**
   * Get serialized size of SatGroundStationAddressTag
   * \return Serialized size in bytes
   */
  virtual uint32_t  GetSerializedSize () const;

  /**
   * Print time stamp of this instance of SatGroundStationAddressTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Get the ground station MAC address
   * \return ground station MAC address
   */
  Mac48Address GetGroundStationAddress (void) const;

  /**
   * Set the ground station MAC address
   * \param groundStationAddress ground station MAC address
   */
  void SetGroundStationAddress (Mac48Address groundStationAddress);

private:
  static const uint32_t ADDRESS_LENGHT = 6;

  Mac48Address m_groundStationAddress;
};

} //namespace ns3

#endif /* SATELLITE_GROUND_STATION_ADDRESS_TAG_H */
