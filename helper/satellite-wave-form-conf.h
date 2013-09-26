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

#ifndef SATELLITE_WAVE_FORM_CONF_H
#define SATELLITE_WAVE_FORM_CONF_H

#include <vector>
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements configuration for Bandwidth Time Units (BTUs)
 */

class SatWaveFormConf : public SimpleRefCount<SatWaveFormConf>
{
public:

  /**
   * Default constructor for SatWaveFormConf
   */
  SatWaveFormConf ();

  /**
   * Constructor for SatWaveFormConf
   *
   * \param codingRate            Coding rate of the wave form
   * \param payloadLength_byte    Payload length of the wave form in bytes
   * \param burstLength_sym       Burst lenght of the wave form in symbols
   */
  SatWaveFormConf (double codingRate, uint32_t payloadLength_byte, uint32_t burstLength_sym);

  /**
   * Destructor for SatWaveFormConf
   */
  ~SatWaveFormConf ();

  /**
   * Get coding rate of the wave form.
   *
   * \return The coding rate of the wave form.
   */
  inline double GetCodingRate() { return m_codingRate; }

  /**
   * Get paylod length of the wave form.
   *
   * \return The paylod length of the wave form in bytes.
   */
  inline double GetPayloadLength() { return m_payloadLength_byte; }

  /**
   * Get burst length of the wave form.
   *
   * \return The burst length of the wave form in symbols.
   */
  inline double GetBurstLength() { return m_burstLength_sym; }

private:
  double m_codingRate;
  uint32_t m_payloadLength_byte;
  uint32_t m_burstLength_sym;
};

} // namespace ns3

#endif // SATELLITE_WAVE_FORM_CONF_H
