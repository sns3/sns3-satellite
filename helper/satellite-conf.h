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
#include "ns3/satellite-superframe-sequence.h"
#include "ns3/satellite-fwd-carrier-conf.h"

namespace ns3 {

/**
 * \brief A configuration class for the GEO satellite reference system
 *
 */
class SatConf : public Object
{
public:

  /**
   * Default constructor.
   */
  SatConf ();
  virtual ~SatConf ()
  {
  }

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  inline Ptr<SatSuperframeSeq> GetSuperframeSeq ()
  {
    return m_superframeSeq;
  }

  /**
   * Initialize the configuration
   * \param rtnConf RTN link configuration file name
   * \param fwdConf FWD link configuration file name
   * \param gwPos GW position file name
   * \param satPos Satellie position file name
   * \param wfConf Waveform configuration file name
   */
  void Initialize (std::string rtnConf,
                   std::string fwdConf,
                   std::string gwPos,
                   std::string satPos,
                   std::string wfConf);

  /**
   * Try to open a file from a given path
   * \param filePathName
   */
  std::ifstream* OpenFile (std::string filePathName) const;

  /**
   * Load satellite configuration from a file
   * \param filePathName
   * \return Container of the configuration data
   */
  std::vector <std::vector <uint32_t> > LoadSatConf (std::string filePathName) const;

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
  std::vector <uint32_t> GetBeamConfiguration (uint32_t beamId, SatEnums::SatLinkDir_t dir) const;

  /**
   * \return The number of the carriers in return link.
   */
  uint32_t GetRtnLinkCarrierCount () const;

  /**
   * Get count of the GWs (positions).
   *
   * \return GW count
   */
  uint32_t GetGwCount () const;

  /**
   * Get count of the UTs (positions).
   *
   * \return UT count
   */
  uint32_t GetUtCount () const;


  /**
   * Get the position of the GW for a given GW id
   *
   * \param gwId id of the GW
   *
   * \return Requested GW's position.
   */
  GeoCoordinate GetGwPosition (uint32_t gwId) const;

  /**
   * Get the position of the GW for a given UT id
   *
   * \param utId id of the UT (from SatIdMapper)
   *
   * \return Requested UT's position.
   */
  GeoCoordinate GetUtPosition (uint32_t utId) const;

  /**
   * Get the position of the Geo Satellite
   *
   * \return Geo satellite position.
   */
  GeoCoordinate GetGeoSatPosition () const;

  /**
   * Convert carrier id, sequency id and frequency id to real frequency value.
   *
   * \param chType    Type of channel.
   * \param freqId    Id of the frequency.
   * \param carrierId Id of the carrier.
   * \return real frequency value
   */
  double GetCarrierFrequencyHz ( SatEnums::ChannelType_t chType, uint32_t freqId, uint32_t carrierId );

  /**
   * Convert carrier id and sequence id to to bandwidth value.
   *
   * \param chType    Type of channel.
   * \param carrierId Id of the carrier.
   * \param bandwidthType Type of the bandwidth.
   * \return bandwidth value
   */
  double GetCarrierBandwidthHz ( SatEnums::ChannelType_t chType, uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType );

  /**
   * \return The number of the carriers in return link.
   */
  uint32_t GetFwdLinkCarrierCount () const;

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
  typedef std::vector <GeoCoordinate> PositionContainer_t;

  /*
   *  Columns:
   *  1. Beam id
   *  2. User frequency id
   *  3. GW id
   *  4. Feeder frequency id
   */
  std::vector <std::vector <uint32_t> > m_rtnConf;
  std::vector <std::vector <uint32_t> > m_fwdConf;

  /**
   * Beam count.
   */
  uint32_t m_beamCount;

  /**
   * Geodetic positions of the GWs
   */
  PositionContainer_t m_gwPositions;

  /**
   * Geodetic positions of the UTs
   */
  PositionContainer_t m_utPositions;

  /**
   * Geodetic positions of the Geo Satellite
   */
  PositionContainer_t m_geoSatPosition;

  /**
   * File to use when loading UT specific position (for user defined positions)
   */
  std::string  m_utPositionInputFileName;

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
  std::vector<Ptr<SatFwdCarrierConf> >  m_forwardLinkCarrierConf;

  /**
   *  Base frequency of forward feeder link.
   */
  double m_fwdFeederLinkFreqHz;

  /**
   *  Bandwidth of forward feeder link.
   */
  double m_fwdFeederLinkBandwidthHz;

  /**
   *  Base frequency of forward user link.
   */
  double m_fwdUserLinkFreqHz;

  /**
   *  Bandwidth of forward user link.
   */
  double m_fwdUserLinkBandwidthHz;

  /**
   *  Base frequency of return feeder link.
   */
  double m_rtnFeederLinkFreqHz;

  /**
   *  Bandwidth of return feeder link.
   */
  double m_rtnFeederLinkBandwidthHz;

  /**
   *  Base frequency of return user link.
   */
  double m_rtnUserLinkFreqHz;

  /**
   *  Bandwidth of return user link.
   */
  double m_rtnUserLinkBandwidthHz;

  /**
   *  The number of the channels in different satellite
   *  links: forward user, return user, forward feeder,
   *  return feeder.
   */
  uint32_t m_fwdUserLinkChannelCount;
  uint32_t m_rtnUserLinkChannelCount;
  uint32_t m_fwdFeederLinkChannelCount;
  uint32_t m_rtnFeederLinkChannelCount;

  /**
   *  The super frame configuration used for sequence 0.
   */
  SatSuperframeConf::SuperFrameConfiguration_t m_SuperFrameConfForSeq0;

  /**
   * The configured allocated bandwidth for forward link carriers.
   */
  double m_fwdCarrierAllocatedBandwidthHz;

  /**
   * The configured carrier roll-off factor for forward link carriers.
   */
  double m_fwdCarrierRollOffFactor;

  /**
   * The configured carrier spacing factor for forward link carriers.
   */
  double m_fwdCarrierSpacingFactor;

  /**
   * Get bandwidth of the forward link carrier.
   *
   * \param carrierId Id of the carrier.
   * \param bandwidthType Type of the bandwidth.
   * \return Requested carrier bandwidth.
   */
  double GetFwdLinkCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const;

  /**
   * Configures itself with default values. Creates configuration storages as needed.
   * \param wfConf path+filename of the waveform configuration
   */
  void Configure (std::string wfConf);

  /**
   * \param carrierId  Global carrier ID across all superframe sequences.
   * \param seqId      Pointer to variable to store seqId of the super frame sequence
   *                   where global id belongs to
   *
   * \return Carrier id relative to the superframe
   */
  uint32_t GetSuperframeCarrierId (uint32_t carrierId, uint32_t * seqId);

  /**
   * \param seqId      Sequence of the superframe.
   * \param carrierId  Carrier ID inside requested superframe.
   *
   * \return Global carrier id  across all superframe sequences.
   */
  uint32_t GetGlobalCarrierId (uint32_t seqId, uint32_t carrierId );

  /**
   * Load node positions from a file
   * \param filePathName
   * \param container Container reference to store found positions
   */
  void LoadPositions (std::string filePathName, PositionContainer_t& container);

};


} // namespace ns3


#endif /* SAT_CONF_H */
