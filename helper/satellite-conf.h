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

#ifndef SAT_CONF_H
#define SAT_CONF_H

#include <vector>
#include <set>
#include <fstream>
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/geo-coordinate.h"
#include "ns3/satellite-channel.h"
#include "satellite-superframe-sequence.h"

namespace ns3 {

/**
 * \brief A configuration class for the GEO satellite reference system
 *
 */
class SatConf : public Object
{
public:
  typedef enum
  {
    STATIC_CONFIG_0,
    STATIC_CONFIG_1,
    STATIC_CONFIG_2,
    STATIC_CONFIG_3,
  } StaticFrameConfiguration_t;

  SatConf ();
  virtual ~SatConf () {}

  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  inline Ptr<SatSuperframeSeq> GetSuperframeSeq () {return m_superframeSeq;}

  /**
   * Initialize the configuration
   * \param path
   * \param satConf
   * \param gwPos
   * \param satPos
   * \param wfConf
   */
  void Initialize (std::string path, std::string satConf, std::string gwPos, std::string satPos, std::string wfConf);

  /**
   * Try to open a file from a given path
   * \param filePathName
   */
  std::ifstream* OpenFile (std::string filePathName);

  /**
   * Load satellite configuration from a file
   * \param filePathName
   */
  void LoadSatConf (std::string filePathName);

  /**
   * Load GW positions from a file
   * \param filePathName
   */
  void LoadGwPos (std::string filePathName);

  /**
   * Load satellite position from a file
   * \param filePathName
   */
  void LoadGeoSatPos (std::string filePathName);

  /**
   * Get count of the beams (configurations).
   *
   * \return beam count
   */
   uint32_t GetBeamCount () const;

  /**
   * Get the configuration vector for a given satellite beam id
   *
   * \param beamId id of the beam
   * \return the configuration vector for a given satellite beam id
   */
   std::vector <uint32_t> GetBeamConfiguration (uint32_t beamId) const;

   /**
    * \return The number of the carriers in return link.
    */
   uint32_t GetRtnLinkCarrierCount () const;


   /**
    * \return The number of the carriers in return link.
    */
   uint32_t GetFwdLinkCarrierCount () const;

   /**
    * Get bandwidth of the forward link carrier.
    *
    * \param carrierId Id of the carrier.
    * \param bandwidthType Type of the bandwidth.
    * \return Requested carrier frequency.
    */
   double GetFwdLinkCarrierFrequencyHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const;

   /**
    * Get count of the GWs (positions).
    *
    * \return GW count
    */
   uint32_t GetGwCount () const;

   /**
    * Get the position of the GW for a given GW id
    *
    * \param gwId id of the GW
    *
    * \return Requested GW's position.
    */
   GeoCoordinate GetGwPosition (uint32_t gwId) const;

   /**
    * Get the position of the Geo Satellite
    *
    * \return Geo satellite position.
    */
   GeoCoordinate GetGeoSatPosition () const;

   /**
    * Get the number of the user frequencies.
    *
    * \return The number of the user frequencies.
    */
   uint32_t GetUserFreqCount() const;

   /**
    * Get the number of the feeder frequencies.
    *
    * \return The number of the feeder frequencies.
    */
   uint32_t GetFeederFreqCount() const;

   /**
    * Convert carrier id, sequency id and frequency id to real frequency value.
    *
    * \param chType    Type of channel.
    * \param freqId    Id of the frequency.
    * \param carrierId Id of the carrier.
    * \return real frequency value
    */
   double GetCarrierFrequency( SatEnums::ChannelType_t chType, uint32_t freqId, uint32_t carrierId );

   /**
    * Convert carrier id and sequence id to to bandwidth value.
    *
    * \param chType    Type of channel.
    * \param carrierId Id of the carrier.
    * \param bandwidhtType Type of the bandwidth.
    * \return bandwidth value
    */
   double GetCarrierBandwidth( SatEnums::ChannelType_t chType, uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidhtType );

   /**
    * Definition for beam ID index (column) in m_conf
    */
   static const uint32_t BEAM_ID_INDEX = 0;

   /**
    * Definition for user frequency ID index (column) in m_conf
    */
   static const uint32_t U_FREQ_ID_INDEX = 1;

   /**
    * Definition for GW ID index (column) in m_conf
    */
   static const uint32_t GW_ID_INDEX = 2;

   /**
    * Definition for feeder frequency ID index (column) in m_conf
    */
   static const uint32_t F_FREQ_ID_INDEX = 3;
   static const uint32_t BEAM_ELEM_COUNT = 4;

private:

   /**
    * Configures itself with default values. Creates configuration storages as needed.
    * \param wfConf path+filename of the waveform configuration
    */
   void Configure(std::string wfConf);

   /**
    * \param carrierId  Global carrier ID across all seperframe sequncies.
    * \param seqId      Pointer to variable to store seqId of the super frame sequence
    *                   where globl id belongs to
    *
    * \return Carrier id relative to the superframe
    */
   uint32_t GetSuperframeCarrierId (uint32_t carrierId, uint32_t * seqId);

   /**
    * \param seqId      Sequence of the superframe.
    * \param carrierId  Carrier ID inseide requested superframe.
    *
    * \return Global carrier id  across all seperframe sequncies.
    */
    uint32_t GetGlobalCarrierId (uint32_t seqId, uint32_t carrierId );

   /*
    *  Columns:
    *  1. Beam id
    *  2. User frequency id
    *  3. GW id
    *  4. Feeder frequency id
    */
   std::vector <std::vector <uint32_t> > m_conf;

   /**
    * Beam count.
    */
   uint32_t m_beamCount;

   /**
    * Geodetic positions of the GWs
    */
   std::vector <GeoCoordinate> m_gwPositions;

   /**
    * GW count.
    */
   uint32_t m_gwCount;

   /**
    * Geodetic positions of the Geo Satellite
    */
   GeoCoordinate m_geoSatPosition;

   /**
    * Superframe sequence configuration
    *
    * Table includes superframe configurations for return link.
    * Item index of the list means superframe sequency (SFS).
    */
   Ptr<SatSuperframeSeq> m_superframeSeq;

   /**
    * Forward link carrier configuration.
    *
    * Item index of the list means carrier configuration sequence.
    * Currently only one sequence used and only one carrier inside carrier conf.
    */
   std::vector<Ptr<SatBtuConf> >  m_forwardLinkCarrierConf;

   /**
    *  Base frequency of forward feeder link.
    */
   double m_fwdFeederLinkFreq_hz;

   /**
    *  Bandwidth of forward feeder link.
    */
   double m_fwdFeederLinkBandwidth_hz;

   /**
    *  Base frequency of forward user link.
    */
   double m_fwdUserLinkFreq_hz;

   /**
    *  Bandwidth of forward user link.
    */
   double m_fwdUserLinkBandwidth_hz;

   /**
    *  Base frequency of return feeder link.
    */
   double m_rtnFeederLinkFreq_hz;

   /**
    *  Bandwidth of return feeder link.
    */
   double m_rtnFeederLinkBandwidth_hz;

   /**
    *  Base frequency of return user link.
    */
   double m_rtnUserLinkFreq_hz;

   /**
    *  Bandwidth of return user link.
    */
   double m_rtnUserLinkBandwidth_hz;

   /**
    *  The number of the channels in user link.
    */
   uint32_t m_userLinkChannelCount;

   /**
    *  The number of the channels in user link.
    */
   uint32_t m_feederLinkChannelCount;

   /**
    * The configured static frame configuration to be used.
    */
   StaticFrameConfiguration_t m_staticFrameConfig;

   /**
    * The configured superframe target duration for static configurations.
    */
   double m_frameConfTargetDuration;

   /**
    * The configured allocated carrier bandwidth for static configurations.
    */
   double m_frameConfAllocatedBandwidth;

   /**
    * The configured carrier roll-off factor for static configurations.
    */
   double m_frameConfRollOffFactor;

   /**
    * The configured carrier spacing factor for static configurations.
    */
   double m_frameConfSpacingFactor;

   /**
    * The configured allocated bandwidth for forward link carriers.
    */
   double m_fwdCarrierAllocatedBandwidth;

   /**
    * The configured carrier roll-off factor for forward link carriers.
    */
   double m_fwdCarrierRollOffFactor;

   /**
    * The configured carrier spacing factor for forward link carriers.
    */
   double m_fwdCarrierSpacingFactor;
};


} // namespace ns3


#endif /* SAT_CONF_H */
