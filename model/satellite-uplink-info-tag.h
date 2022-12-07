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
 * Author: Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_UPLINK_INFO_TAG_H
#define SATELLITE_UPLINK_INFO_TAG_H

#include <ns3/tag.h>
#include <ns3/nstime.h>


namespace ns3 {

/**
 * \ingroup satellite
 * \brief Tag to store uplink phy info that need to be forwarded to the downlink.
 */
class SatUplinkInfoTag : public Tag
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId  GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId  GetInstanceTypeId (void) const;

  /**
   * Default constructor.
   */
  SatUplinkInfoTag ();

  /**
   * Constructor with initialization parameters.
   * \param satelliteReceptionTime
   * \param sinr
   * \param additionalInterference additional interference to add to compute final SINR
   * \param satId Satellite ID of the UT
   * \param beamId Beam ID of the UT
   * \param isControl This is a control packet
   */
  SatUplinkInfoTag (Time satelliteReceptionTime, double sinr, double additionalInterference, uint32_t satId, uint32_t beamId, bool isControl);

  /**
   * Serializes information to buffer from this instance of SatUplinkInfoTag
   * \param i Buffer in which the information is serialized
   */
  virtual void  Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatUplinkInfoTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void  Deserialize (TagBuffer i);

  /**
   * Get serialized size of SatUplinkInfoTag
   * \return Serialized size in bytes
   */
  virtual uint32_t  GetSerializedSize () const;

  /**
   * Print time stamp of this instance of SatUplinkInfoTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Get satellite reception time of packet
   * \return Satellite reception time
   */
  Time GetSatelliteReceptionTime (void) const;

  /**
   * Set satellite reception time of packet
   * \param satelliteReceptionTime Satellite reception time
   */
  void SetSatelliteReceptionTime (Time satelliteReceptionTime);

  /**
   * Get uplink SINR
   * \return Uplink SINR
   */
  double GetSinr (void) const;

  /**
   * Set uplink SINR
   * \param sinr Uplink SINR
   * \param additionalInterference additional interference to add to compute final SINR
   */
  void SetSinr (double sinr, double additionalInterference);

  /**
   * Get additional interference to add to compute final SINR
   * \return The interference to add
   */
  double GetAdditionalInterference (void) const;

  /**
   * Tell if SINR already computed
   * \return true if already computed
   */
  bool IsSinrComputed (void) const;

  /**
   * Get the UT sat ID
   * \return Sat ID
   */
  uint32_t GetSatId (void) const;

  /**
   * Set the UT sat ID
   * \param satId Sat ID
   */
  void SetSatId (uint32_t satId);

  /**
   * Get the UT beam ID
   * \return Beam ID
   */
  uint32_t GetBeamId (void) const;

  /**
   * Set the UT beam ID
   * \param beamId Beam ID
   */
  void SetBeamId (uint32_t beamId);

  /**
   * Get if packet is a control packet
   * \return True if this is a control packet
   */
  bool IsControl (void) const;

  /**
   * Set if packet is a control packet
   * \param isControl True if this is a control packet
   */
  void SetIsControl (bool isControl);

private:
  Time m_satelliteReceptionTime;              // Reception time of packet in satellite
  double m_sinr;                              // SINR computed on uplink
  double m_additionalInterference;            // Other interference to add to compute final SINR
  bool m_sinrComputed;                        // Flag to tell if SINR has been already computed
  uint32_t m_satId;                           // Sat ID where the UT is located
  uint32_t m_beamId;                          // Beam ID where the UT is located
  bool m_isControl;                           // Uplink burst is control and has been sent using WF02

};

} //namespace ns3

#endif /* SATELLITE_UPLINK_INFO_TAG_H */
