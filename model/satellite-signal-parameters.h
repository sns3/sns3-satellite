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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_SIGNAL_PARAMETERS_H
#define SATELLITE_SIGNAL_PARAMETERS_H

#include <ns3/ptr.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/object.h>

namespace ns3 {

class SatPhyTx;

/**
* \ingroup satellite
* The transmission-reception related parameters needed e.g. for
* received signal strength calculation.
*/
class SatSignalParameters : public Object
{
public:
  
  /**
  * default constructor
  */
  SatSignalParameters ();
  
  /**
    * copy constructor
    */
  SatSignalParameters (const SatSignalParameters& p);

  Ptr<SatSignalParameters> Copy ();
  static TypeId GetTypeId (void);

  /**
  * The packet being transmitted with this signal
  */
  Ptr<Packet> m_packet;

  /**
  * The beam for the packet transmission
  */
  uint16_t m_beamId;

  /**
   * The duration of the packet transmission.
   *
   */
  Time m_duration;

  /**
   * The SatPhyTx instance that is making the transmission
   */
  Ptr<SatPhyTx> m_phyTx;

};


}  // namespace ns3


#endif /* SATELLITE_SIGNAL_PARAMETERS_H */
