/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef LORA_BEAM_TAG_H
#define LORA_BEAM_TAG_H

#include <ns3/tag.h>

namespace ns3 {

/**
 * Tag used to save various data about a packet, like its Spreading Factor and
 * data about interference.
 */
class LoraBeamTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create a LoraBeamTag with a given beam ID.
   *
   * \param beamId The beam ID associated to this packet
   */
  LoraBeamTag (uint8_t beamId = 0);

  virtual ~LoraBeamTag ();

  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual uint32_t GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  /**
   * Read which beam ID this packet was transmitted with.
   *
   * \return This tag's packet's beam ID.
   */
  uint8_t GetBeamId () const;

  /**
   * Set which beamId this packet was transmitted with.
   *
   * \param beamId The beam ID.
   */
  void SetBeamId (uint8_t beamId);

private:
  uint8_t m_beamId; //!< The beam ID used by the packet.
};
} // namespace ns3

#endif //LORA_BEAM_TAG_H
