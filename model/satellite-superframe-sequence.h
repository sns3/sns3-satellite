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

#ifndef SATELLITE_SUPERFRAME_SEQUENCE_H
#define SATELLITE_SUPERFRAME_SEQUENCE_H

#include <vector>
#include <map>
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/satellite-control-message.h"
#include "satellite-frame-conf.h"
#include "satellite-wave-form-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements super frame sequence.
 *  It consists information of the super frames in sequence.
 *
 */
class SatSuperframeSeq : public Object
{
public:

  /**
   * Define type SatSuperframeConfList
   */
  typedef std::vector<Ptr<SatSuperframeConf> > SatSuperframeConfList;

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeSeq ();

  /**
   * Destructor for SatSuperframeSeq.
   */
  ~SatSuperframeSeq ();

  /**
   * \brief Add waveform configuration class instance to this superframe sequence
   * \param wfConf Waveform conf to add
   */
  void AddWaveformConf (Ptr<SatWaveformConf> wfConf);

  /**
   * \brief Get waveform configuration
   * \return SatWaveformConf Waveform configuration class instance
   */
  Ptr<SatWaveformConf> GetWaveformConf () const;

  /**
   * \brief Add super frame (configuration) to super frame sequence
   * \param conf Super frame configuration to add. (id of the sequence is order number of addition starting from 0)
   */
  void AddSuperframe (Ptr<SatSuperframeConf> conf);

  /**
   * \brief Get superframe conf of the sequence.
   *
   * \param seqId    Id of the super frame sequence requested.
   * \return The requested super frame conf of the sequence.
   */
  Ptr<SatSuperframeConf> GetSuperframeConf  (uint8_t seqId ) const;

  /**
   * \brief Get global carrier id. Converts super frame specific id to global carrier id.
   *
   * \param superframeId       Id of the super frame requested.
   * \param frameId            Id of the frame inside super frame requested.
   * \param frameCarrierId     Id of the carrier inside frame requested.
   *
   * \return The requested carrier id of the super frame.
   */
  uint32_t GetCarrierId ( uint8_t superframeId, uint8_t frameId, uint16_t frameCarrierId ) const;

  /**
   * \brief Get carrier count of the super frame sequence.
   *
   * \return The super frame sequence carrier count.
   */
  uint32_t GetCarrierCount () const;

  /**
   * \brief Get carrier count in the super frame.
   *
   * \param seqId Sequence id of the super frame which carrier count is requested.
   *
   * \return The super frame carrier count.
   */
  uint32_t GetCarrierCount ( uint8_t seqId ) const;

  /**
   * \brief Get duration of the super frame.
   *
   * \param seqId Sequence id of the super frame which duration is requested.
   *
   * \return The super frame duration.
   */
  Time GetDuration ( uint8_t seqId ) const;

  /**
   * \brief Get the center frequency of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside super frame which center frequency is requested.
   *
   * \return The center frequency of the requested carrier.
   */
  double GetCarrierFrequencyHz (uint32_t carrierId) const;

  /**
   * \brief Get the bandwidth of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside super frame which bandwidth is requested.
   * \param bandwidthType Type of bandwidth requested.
   *
   * \return The bandwidth of the requested carrier.
   */
  double GetCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const;

  /**
   * \brief Get target duration for sequence.
   * \return target duration for sequence
   */
  inline Time GetTargetDuration () const
  {
    return m_targetDuration;
  }

private:
  typedef std::map<uint32_t, Ptr<SatControlMsgContainer> > TbtpMap_t;

  /**
    * Super frame sequences.
    *
    * Table includes super frame configurations for the return link.
    * Item index of the list means super frame sequence (SFS).
    */
  SatSuperframeConfList m_superframe;

  /**
   * Waveform configurations
   */
  Ptr<SatWaveformConf> m_wfConf;

  /**
   * Target duration time for sequence.
   */
  Time m_targetDuration;
};

} // namespace ns3

#endif // SATELLITE_SUPERFRAME_SEQUENCE_H
