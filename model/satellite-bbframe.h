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

namespace ns3 {

/**
 * \ingroup satellite
 * \brief BB Frame class.
 * Holds information of BB Frames for forward link scheduling.
 *
 */
class SatBbFrame :  public SimpleRefCount<SatBbFrame>
{
public:

  /**
   *
   */
  typedef enum
  {
    SHORT_FRAME, //!< SHORT_FRAME
    NORMAL_FRAME,//!< NORMAL_FRAME
    DUMMY_FRAME  //!< DUMMY_FRAME
  } FrameType_t;

  typedef std::vector<Ptr<Packet > > SatBbFrameData;
  static const uint32_t m_shortBbFrameLengthInBytes = 2025;   // 16200 bits without ModCod
  static const uint32_t m_normalBbFrameLengthInBytes =  8100;   // 64800 bits without ModCod

  /**
   * Default constructor. Constructs BB frame with default ModCod and default frame type.
   */
  SatBbFrame ();

  /**
   * Constructor to create BB frame according to given type and MODCOD.
   * \param modCod Used ModCod
   * \param type Type if the frame
   */
  SatBbFrame (uint32_t modCod, FrameType_t type);

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
   * Add data to transmit buffer of this BB Frame info
   *
   * \param packet        Pointer to packet wanted to add transmit buffer
   * \param controlData   Is given data treated as control data
   * \return Free bytes left in transmit buffer
   */
  uint32_t AddTransmitData (Ptr<Packet> packet, bool controlData);

  /**
   * Check whether the buffer contains control messages
   * \return boolean
   */
  bool ContainsControlData () const;

  /**
   * Get free bytes in transmit buffer.
   * \return free bytes in transmit buffer
   */
  uint32_t GetBytesLeft () const ;

  //TODO: Needed to calculate according to MODCOD
  /**
   * Get duration of the frame transmission.
   * \return duration of the frame transmission
   */
  inline Time GetDuration () const {return m_duration;}


private:

  uint32_t m_modCod;
  uint32_t m_freeBytes;
  uint32_t m_TotalBytes;
  bool m_containsControlData;
  SatBbFrameData frameData;
  Time m_duration;
};


} // namespace ns3


#endif /* SATELLITE_BBFRAME_H_ */
