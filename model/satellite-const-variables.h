/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#ifndef SATELLITE_CONST_VARIABLES_H
#define SATELLITE_CONST_VARIABLES_H

#include <cstdint>

namespace ns3
{

/**
 * \ingroup satellite
 *
 * \brief SatConstVariables is used for const variable definitions in satellite module.
 */
namespace SatConstVariables
{

/**
 * \brief Constant definition for the speed of light in m/s
 */
constexpr double SPEED_OF_LIGHT = 299792458.0;

/**
 * \brief Number of bits in a byte
 */
constexpr uint32_t BITS_PER_BYTE = 8;

/**
 * \brief Number of bytes consisting a kilobyte
 */
constexpr uint32_t BYTES_IN_KBYTE = 1024;

/**
 * \brief Number of bits consisting a kilobit
 */
constexpr uint32_t BITS_IN_KBIT = 1000;

/**
 * \brief Boltzmann Constant
 */
constexpr double BOLTZMANN_CONSTANT = 1.3806488e-23;

/**
 * \brief Used superframe sequence in the RTN link.
 */
constexpr uint8_t SUPERFRAME_SEQUENCE = 0;

/**
 * \brief Maximum value for time slot ID as specified in ETSI EN 301 542-2, chapter 7.5.1.3.
 */
constexpr uint16_t MAXIMUM_TIME_SLOT_ID = 2047;

/**
 * \brief Maximum number of beams per satellite
 */
constexpr uint32_t MAX_BEAMS_PER_SATELLITE = 1000;

/**
 * \brief Maximum number of satellites in constellation
 */
constexpr uint32_t MAX_SATELLITES = 10000;

} // namespace SatConstVariables

} // namespace ns3

#endif /* SATELLITE_CONST_VARIABLES_H */
