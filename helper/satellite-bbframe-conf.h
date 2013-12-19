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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_BBFRAME_CONF_H
#define SATELLITE_BBFRAME_CONF_H

#include <map>
#include "ns3/object.h"
#include "ns3/satellite-enums.h"

namespace ns3 {


/**
 * \ingroup satellite
 * \brief This class implements the BBFrame configuration for DVB-S2.
 */

class SatBbFrameConf : public Object
{
public:
  /**
   * Default constructor
   */
  SatBbFrameConf ();

  /**
   * Destructor for SatBbFrameConf
   */
  virtual ~SatBbFrameConf ();

  static TypeId GetTypeId (void);

  /**
   * Get the BBFrame length in Time
   * \param modcod Used MODCOD in the BBFrame
   * \param frameType Used BBFrame type (short OR normal)
   * \param symbolRate The symbol rate of the scheduled carrier
   * \return The BBFrame length in Time
   */
  Time GetBbFrameLength (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType, double symbolRate) const;

  /**
   * Get the dummy frame length in Time
   * \param symbol rate of the scheduled carrier
   * \return The dummy BBFrame length in Time
   */
  Time GetDummyBbFrameLength (double symbolRate) const;

  /**
   * Get the BBFrame higher layer payload in bytes
   * \param modcod Used MODCOD in the BBFrame
   * \param frameType Used BBFrame type (short OR normal)
   * \return The maximum payload in bytes
   */
  uint32_t GetBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const;


private:

  /**
   * Convert the SatModCod_t enum to modulated bits
   * \param modcod Used modcod in SatModCod_t enum
   * \return modulated bits in uint32_t
   */
  uint32_t GetModulatedBits (SatEnums::SatModcod_t modcod) const;

  /**
   * Convert the SatModCod_t enum to modulated bits
   * \param modcod Used modcod in SatModCod_t enum
   * \return coding rate in double
   */
  double GetCodingRate (SatEnums::SatModcod_t modcod) const;

  /**
   * The size of the (data) slot in symbols
   */
  uint32_t m_symbolsPerSlot;

  /**
   * The size of the pilot block in symbols
   */
  uint32_t m_pilotBlockInSymbols;

  /**
   * Interval of pilot block in slots
   */
  uint32_t m_pilotBlockIntervalInSlots;

  /**
   * The PL header size in slots
   */
  uint32_t m_plHeaderInSlots;

  /**
   * Dummy BBFrame length in slots
   */
  uint32_t m_dummyFrameInSlots;

  std::map<uint32_t, uint32_t> m_payloadsShortFrame;
  std::map<uint32_t, uint32_t> m_payloadsNormalFrame;
};

} // namespace ns3

#endif // SATELLITE_BBFRAME_CONF_H
