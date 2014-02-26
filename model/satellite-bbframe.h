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


#ifndef SATELLITE_BBFRAME_H_
#define SATELLITE_BBFRAME_H_

#include <vector>
#include "ns3/simple-ref-count.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/satellite-bbframe-conf.h"
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief BB Frame class.
 * Holds information of a BB Frame for forward link scheduling.
 *
 */
class SatBbFrame : public SimpleRefCount<SatBbFrame>
{
public:

  typedef std::vector<Ptr<Packet > > SatBbFrameData;

  /**
   * Default constructor. Constructs BB frame with default ModCod and default frame type.
   */
  SatBbFrame ();

  /**
   * Constructor to create BB frame according to given type and MODCOD.
   * \param modCod Used ModCod
   * \param type Type if the frame
   * \param conf Pointer to BBFrame configuration
   */
  SatBbFrame (SatEnums::SatModcod_t modCod, SatEnums::SatBbFrameType_t type, Ptr<SatBbFrameConf> conf);

  /**
   * Destructor
   */
  virtual ~SatBbFrame ();

  /**
   * Get the data in the BB Frame info as container of the packet pointers.
   * \return Container having data as packet pointers.
   */
  const SatBbFrameData& GetTransmitData ();

  /**
   * Add payload (packet) to transmit buffer of this BB Frame info
   *
   * \param packet  Pointer to packet wanted to add to transmit buffer
   * \return Free bytes left in transmit buffer
   */
  uint32_t AddPayload (Ptr<Packet> packet);

  /**
   * Check whether the buffer contains control messages (PDUs)
   * \return boolean
   */
  bool ContainsControlPdu () const;

  /**
   * Get space left in BB frame transmit buffer in bytes.
   * \return free bytes in transmit buffer
   */
  uint32_t GetSpaceLeftInBytes () const ;

  /**
   * Get the maximum size of the BB Frame transmit buffer in bytes.
   * \return the maximum size of the BB Frame transmit buffer
   */
  uint32_t GetMaxSpaceInBytes () const ;

  /**
   * Get duration of the frame transmission.
   * \return duration of the frame transmission
   */
  inline Time GetDuration () const {return m_duration;}

  /**
   * Get type of the frame.
   * \return Type of the frame
   */
  inline SatEnums::SatBbFrameType_t GetFrameType () const {return m_frameType;}

  /**
   * Get type of the frame.
   * \return Type of the frame
   */
  inline SatEnums::SatModcod_t GetModcod () const {return m_modCod;}

private:

  SatEnums::SatModcod_t m_modCod;
  uint32_t m_spaceInBytes;
  uint32_t m_maxSpaceInBytes;
  bool m_containsControlPdu;
  SatBbFrameData framePayload;
  Time m_duration;
  SatEnums::SatBbFrameType_t m_frameType;
};


} // namespace ns3


#endif /* SATELLITE_BBFRAME_H_ */
