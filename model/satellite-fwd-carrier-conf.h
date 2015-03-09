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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_FWD_CARRIER_CONF_H
#define SATELLITE_FWD_CARRIER_CONF_H

#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements configuration for forward link carrier .
 */

class SatFwdCarrierConf : public SimpleRefCount<SatFwdCarrierConf>
{
public:
  /**
   * Default constructor for SatFwdCarrierConf
   */
  SatFwdCarrierConf ();

  /**
   * Constructor for SatFwdCarrierConf
   *
   * \param bandwidthInHz     Allocated bandwidth of Forward Link Carrier in hertz
   * \param rollOff           Roll-off factor
   * \param spacing           Spacing factor
   */
  SatFwdCarrierConf (double bandwidthInHz, double rollOff, double spacing);

  /**
   * Destructor for SatFwdCarrierConf
   */
  ~SatFwdCarrierConf ();

  /**
   * Get bandwidth of Forward Link Carrier.
   *
   * \return The bandwidth of Forward Link Carrier in hertz.
   */
  inline double GetAllocatedBandwidthInHz () const
  {
    return m_allocatedBandwidthInHz;
  }

  /**
   * Get occupied bandwidth of Forward Link Carrier.
   *
   * \return The occupied bandwidth of Forward Link Carrier in hertz.
   */
  inline double GetOccupiedBandwidthInHz () const
  {
    return m_occupiedBandwidthInHz;
  }

  /**
   * Get occupied bandwidth of Forward Link Carrier.
   *
   * \return The occupied bandwidth of Forward Link Carrier in hertz.
   */
  inline double GetEffectiveBandwidthInHz () const
  {
    return m_effectiveBandwidthInHz;
  }

private:
  double m_allocatedBandwidthInHz;
  double m_occupiedBandwidthInHz;
  double m_effectiveBandwidthInHz;
};

} // namespace ns3

#endif // SATELLITE_FWD_CARRIER_CONF_H
