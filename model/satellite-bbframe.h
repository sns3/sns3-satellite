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
#include "ns3/traced-callback.h"
#include "ns3/satellite-bbframe-conf.h"
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatBbFrame class implements functionality for BB frames.
 * Holds information of a BB Frame for forward link scheduling.
 * It also provides methods to query information of the frame and methods
 * to modify frames like shrink, extend and merge.
 *
 */
class SatBbFrame : public SimpleRefCount<SatBbFrame>
{
public:

  /**
   * Define type SatBbFramePayload_t
   */
  typedef std::vector<Ptr<Packet > > SatBbFramePayload_t;

  /**
   * Default constructor. Constructs BB frame with default ModCod and default frame type.
   */
  SatBbFrame ();

  /**
   * Constructor to create BB frame according to given type and MODCOD, type and BB frame configuration.
   *
   * \param modCod Used ModCod
   * \param type Type of the frame
   * \param conf Pointer to BBFrame configuration
   */
  SatBbFrame (SatEnums::SatModcod_t modCod, SatEnums::SatBbFrameType_t type, Ptr<SatBbFrameConf> conf);

  /**
   * Destructor fro BB frame.
   */
  virtual ~SatBbFrame ();

  /**
   * Get the data in the BB Frame info as container of the packet pointers.
   * \return Container having data as packet pointers.
   */
  const SatBbFramePayload_t& GetPayload ();

  /**
   * Add payload (packet) to transmit buffer of this BB Frame info
   *
   * \param packet  Pointer to packet wanted to add to transmit buffer
   * \return Space left bytes in transmit buffer after addition
   */
  uint32_t AddPayload (Ptr<Packet> packet);

  /**
   * Get space left in BB frame transmit buffer in bytes.
   * \return space left in bytes in transmit buffer
   */
  uint32_t GetSpaceLeftInBytes () const;

  /**
   * Get space used in BB frame transmit buffer in bytes.
   * \return space used in bytes in transmit buffer
   */
  uint32_t GetSpaceUsedInBytes () const;

  /**
   * Get the maximum size of the BB Frame transmit buffer in bytes.
   * \return the maximum size of the BB Frame transmit buffer in bytes
   */
  uint32_t GetMaxSpaceInBytes () const;

  /**
   * Get the occupancy of the of the BB Frame.
   * \return the occupancy of the BB Frame 0 - 1.
   */
  double GetOccupancy () const;

  /**
   * Get spectra efficiency of the frame.
   *
   * \param carrierBandwidthInHz Carrier bandwidth in hertz.
   * \return
   */
  double GetSpectralEfficiency (double carrierBandwidthInHz) const;

  /**
   * Checks occupancy of the frame if given frame would been merged with this frame.
   *
   * \param mergedFrame Another frame wanted to merge with this frame.
   * \return O, if frames cannot be merged, occupancy of the merged frame in otherwise.
   */
  double GetOccupancyIfMerged (Ptr<SatBbFrame> mergedFrame) const;

  /**
   * Merge given frame with this frame.
   *
   * \param mergedFrame Another frame to be merged with this frame.
   * \param mergeTraceCb Logging trace source for BB frame optimization.
   * \return true if merging done, false otherwise.
   */
  bool MergeWithFrame (Ptr<SatBbFrame> mergedFrame, TracedCallback<Ptr<SatBbFrame>, Ptr<SatBbFrame> > mergeTraceCb);

  /**
   * Shrink BB frame to the shortest type possible according to
   * current load in the frame.
   * \param conf Pointer to BB frame configuration
   * \return Decrease of the frame duration after shrinking it
   */
  Time Shrink (Ptr<SatBbFrameConf> conf);

  /**
   * Extent BB frame to the longest type.
   * \param conf Pointer to BB frame configuration
   * \return Increase of the frame duration after extending it
   */
  Time Extend (Ptr<SatBbFrameConf> conf);

  /**
   * Get duration of the frame transmission.
   * \return duration of the frame transmission
   */
  inline Time GetDuration () const
  {
    return m_duration;
  }

  /**
   * Get type of the frame.
   * \return Type of the frame
   */
  inline SatEnums::SatBbFrameType_t GetFrameType () const
  {
    return m_frameType;
  }

  /**
   * Get type of the frame.
   * \return Type of the frame
   */
  inline SatEnums::SatModcod_t GetModcod () const
  {
    return m_modCod;
  }

  /**
   * Get header size of the frame.
   * \return Header size in bytes.
   */
  inline uint32_t GetFrameHeaderSize () const
  {
    return m_headerSizeInBytes;
  }

  /**
   * Callback signature for Ptr<SatBbFrame>.
   * \param bbFrame The BB frame.
   */
  typedef void (*BbFrameCallback)(Ptr<SatBbFrame> bbFrame);

  /**
   * Callback signature for merging of two instances of SatBbFrame.
   * \param to The frame merged to.
   * \param to The frame merged from.
   */
  typedef void (*BbFrameMergeCallback)
    (Ptr<SatBbFrame> to, Ptr<SatBbFrame> from);

private:
  SatEnums::SatModcod_t m_modCod;
  uint32_t m_freeSpaceInBytes;
  uint32_t m_maxSpaceInBytes;
  uint32_t m_headerSizeInBytes;
  SatBbFramePayload_t m_framePayload;
  Time m_duration;
  SatEnums::SatBbFrameType_t m_frameType;
};

} // namespace ns3


#endif /* SATELLITE_BBFRAME_H_ */
