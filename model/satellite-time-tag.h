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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_TIME_TAG_H
#define SATELLITE_TIME_TAG_H

#include "ns3/packet.h"
#include "ns3/nstime.h"


namespace ns3 {

class Tag;

/**
 * \ingroup satellite
 * \brief Time tag used to identify the time when packet is
 * enqued.
 *
 */
class SatTimeTag : public Tag
{
public:
  static TypeId  GetTypeId (void);
  virtual TypeId  GetInstanceTypeId (void) const;

  SatTimeTag ();
  SatTimeTag (Time senderTimestamp);

  virtual void  Serialize (TagBuffer i) const;
  virtual void  Deserialize (TagBuffer i);
  virtual uint32_t  GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  Time  GetSenderTimestamp (void) const
  {
    return m_senderTimestamp;
  }

  void  SetSenderTimestamp (Time senderTimestamp)
  {
    this->m_senderTimestamp = senderTimestamp;
  }

private:
  Time m_senderTimestamp;

};

} //namespace ns3

#endif /* SATELLITE_TIME_TAG_H */
