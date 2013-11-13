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
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "satellite-frame-conf.h"
#include "satellite-wave-form-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements superframe sequence.
 *  It consists information of the superframes in sequence.
 *
 */
class SatSuperframeSeq : public Object
{
public:
  typedef std::vector<Ptr<SatSuperframeConf> > SatSuperframeConfList;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeSeq ();

  /**
   * Constructor for SatSuperframeSeq.
   *
   * \param confs Superframes in sequence. (In acsending order according to frequency).
   */
  SatSuperframeSeq ( SatSuperframeConfList * confs );

  /**
   * Destructor for SatSuperframeSeq.
   */
  ~SatSuperframeSeq ();

  /**
   * Add waveform configuration class instance to this superframe sequence
   * \param wfConf Waveform conf to add
   */
  void AddWaveformConf (Ptr<SatWaveformConf> wfConf);

  /**
   * Get waveform configuration
   * \return SatWaveformConf Waveform configuration class instance
   */
  Ptr<SatWaveformConf> GetWaveformConf () const;

  /**
   * Add superframe (configuration) to superframe sequence
   * \param conf Superframe conf to add. (id of the sequence is order number of addition starting from 0)
   */
  void AddSuperframe (Ptr<SatSuperframeConf> conf);

  /**
   * Get superframe conf of the sequence.
   *
   * \param seqId    Id of the superframe sequence requested.
   * \return The requested superframe conf of the sequence.
   */
  Ptr<SatSuperframeConf> GetSuperframeConf  (uint8_t seqId ) const;

  /**
   * Get global carrier id. Converts superframe specific id to global carrier id.
   *
   * \param superframeId       Id of the superframe requested.
   * \param frameId            Id of the frame inside supeframe requested.
   * \param frameCarrierId     Id of the carrier inside frame requested.
   *
   * \return The requested carrier id of the super frame.
   */
  uint32_t GetCarrierId( uint8_t superframeId, uint8_t frameId, uint16_t frameCarrierId ) const;

  /**
   * Get carrier count of the super frame sequence.
   *
   * \return The super frame sequence carrier count.
   */
  uint32_t GetCarrierCount () const;

  /**
   * Get carrier count in the super frame.
   *
   * \param seqId Sequence id of the superframe which carrier count is requested.
   *
   * \return The super frame carrier count.
   */
  uint32_t GetCarrierCount ( uint8_t seqId ) const;

  /**
   * Get duration of the super frame.
   *
   * \param seqId Sequence id of the superframe which duration is requested.
   *
   * \return The superframe duration in seconds.
   */
  double GetDuration_s ( uint8_t seqId ) const;

  /**
   * Get the center frequency of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside superframe which center frequency is requested.
   *
   * \return The center frequency of the requested carrier.
   */
  double GetCarrierFrequency_hz (uint32_t carrierId) const;

  /**
   * Get the bandwidth of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside superframe which bandwidth is requested.
   *
   * \return The bandwidth of the requested carrier.
   */
  double GetCarrierBandwidth_hz (uint32_t carrierId) const;

private:
  /**
    * Superframe sequencies.
    *
    * Table includes superframe configurations for the return link.
    * Item index of the list means superframe sequency (SFS).
    */
  SatSuperframeConfList m_superframe;

  /**
   * Waveform configurations
   */
  Ptr<SatWaveformConf> m_wfConf;

};

} // namespace ns3

#endif // SATELLITE_SUPERFRAME_SEQUENCE_H
