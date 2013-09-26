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

#ifndef SATELLITE_CARRIER_CONF_H
#define SATELLITE_CARRIER_CONF_H

#include <vector>
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements configuration for carriers
 */

class SatCarrierConf : public SimpleRefCount<SatCarrierConf>
{
public:

  /**
   * Default constructor for SatCarrierConf
   */
  SatCarrierConf ();

  /**
   * Constructor for SatCarrierConf.
   *
   * \param frequency_hz      Center frequency of the carrier in Hertz
   * \param bandwidth_hz      Bandwidth of the carrier in Hertz
   */
  SatCarrierConf ( double frequency_hz, double bandwidth_hz );

  /**
   * Destructor for SatCarrierConf
   */
  ~SatCarrierConf ();

  /**
   * Get center frequency of the carrier.
   *
   * \return The center frequency of frame in Hertz.
   */
  inline double GetFrequency_hz() { return m_frequency_hz; }

  /**
   * Get bandwidth of the carrier.
   *
   * \return The bandwidth of frame in Hertz.
   */
  inline double GetBandwidth_hz() { return m_bandwidth_hz; }

private:
  double m_frequency_hz;   // center frequency
  double m_bandwidth_hz;

};

} // namespace ns3

#endif // SATELLITE_CARRIER_CONF_H
