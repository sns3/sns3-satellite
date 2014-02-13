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

#ifndef SATELLITE_MAC_TAG_H
#define SATELLITE_MAC_TAG_H

#include "ns3/address.h"
#include "ns3/tag.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements a tag that carries the satellite MAC specific
 * information currently unit-ID (mac address)
 */
class SatMacTag : public Tag
{
public:

  SatMacTag ();
  ~SatMacTag ();
  void SetDestAddress (Address dest);
  Address GetDestAddress (void) const;

  void SetSourceAddress (Address source);
  Address GetSourceAddress (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  Address   m_destAddress;
  Address   m_sourceAddress;
};

class SatRcIndexTag : public Tag
{
public:

  SatRcIndexTag ();
  ~SatRcIndexTag ();
  void SetRcIndex (uint8_t rcIndex);
  uint8_t GetRcIndex () const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_rcIndex;
};


} // namespace ns3

#endif /* SATELLITE_MAC_TAG_H */
