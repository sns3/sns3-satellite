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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_CONST_VARIABLES_H
#define SATELLITE_CONST_VARIABLES_H


namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatConstVariables is used for const variable definitions in satellite module.
 * This class is not planned to be instantiated or inherited.
 */

class SatConstVariables
{
public:
  /**
   * \brief Constant definition for the speed of light in m/s
   */
  static constexpr double SPEED_OF_LIGHT = 299792458.0;

  /**
   * \brief Number of bits in a byte
   */
  static constexpr uint32_t BITS_PER_BYTE = 8;

  /**
   * \brief Number of bytes consisting a kilobyte
   */
  static constexpr uint32_t BYTES_IN_KBYTE = 1024;

  /**
   * \brief Number of bits consisting a kilobit
   */
  static constexpr uint32_t BITS_IN_KBIT = 1000;

  /**
   * \brief Boltzmann Constant
   */
  static constexpr double BOLTZMANN_CONSTANT = 1.3806488e-23;

  /**
   * \brief Used superframe sequence in the RTN link.
   */
  static constexpr uint8_t SUPERFRAME_SEQUENCE = 0;

  /**
   * \brief CRDSA allocation channel.
   */
  static constexpr uint8_t CRDSA_ALLOCATION_CHANNEL = 0;

  /**
   * \brief Slotted ALOHA allocation channel.
   */
  static constexpr uint8_t SLOTTED_ALOHA_ALLOCATION_CHANNEL = 0;

  /**
   * \brief Maximum value for time slot ID as specified in ETSI EN 301 542-2, chapter 7.5.1.3.
   */
  static constexpr uint16_t MAXIMUM_TIME_SLOT_ID = 2047;

private:
  /**
   * Destructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatConstVariables () = 0;
};

} // namespace ns3

#endif /* SATELLITE_CONST_VARIABLES_H */
