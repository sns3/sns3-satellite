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

#include <ns3/tag.h>
#include <ns3/nstime.h>


namespace ns3 {

/*
 * There are 4 classes defined here: SatTimeTag, SatPhyTimeTag, SatMacTimeTag,
 * and SatDevTimeTag. Except of the name difference, they share
 * exactly the same definitions.
 */

/**
 * \ingroup satellite
 * \brief Time tag used to identify the time when packet is enqueued at LLC
 *        level.
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

  Time GetSenderTimestamp (void) const;
  void SetSenderTimestamp (Time senderTimestamp);

private:
  Time m_senderTimestamp;

};


/**
 * \ingroup satellite
 * \brief Time tag used to identify the time when packet is enqueued at PHY
 *        level.
 */
class SatPhyTimeTag : public Tag
{
public:
  static TypeId  GetTypeId (void);
  virtual TypeId  GetInstanceTypeId (void) const;

  SatPhyTimeTag ();
  SatPhyTimeTag (Time senderTimestamp);

  virtual void  Serialize (TagBuffer i) const;
  virtual void  Deserialize (TagBuffer i);
  virtual uint32_t  GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  Time GetSenderTimestamp (void) const;
  void SetSenderTimestamp (Time senderTimestamp);

private:
  Time m_senderTimestamp;

};


/**
 * \ingroup satellite
 * \brief Time tag used to identify the time when packet is enqueued at MAC
 *        level.
 */
class SatMacTimeTag : public Tag
{
public:
  static TypeId  GetTypeId (void);
  virtual TypeId  GetInstanceTypeId (void) const;

  SatMacTimeTag ();
  SatMacTimeTag (Time senderTimestamp);

  virtual void  Serialize (TagBuffer i) const;
  virtual void  Deserialize (TagBuffer i);
  virtual uint32_t  GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  Time GetSenderTimestamp (void) const;
  void SetSenderTimestamp (Time senderTimestamp);

private:
  Time m_senderTimestamp;

};


/**
 * \ingroup satellite
 * \brief Time tag used to identify the time when packet is enqueued at device
 *        level.
 */
class SatDevTimeTag : public Tag
{
public:
  static TypeId  GetTypeId (void);
  virtual TypeId  GetInstanceTypeId (void) const;

  SatDevTimeTag ();
  SatDevTimeTag (Time senderTimestamp);

  virtual void  Serialize (TagBuffer i) const;
  virtual void  Deserialize (TagBuffer i);
  virtual uint32_t  GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  Time GetSenderTimestamp (void) const;
  void SetSenderTimestamp (Time senderTimestamp);

private:
  Time m_senderTimestamp;

};



} //namespace ns3

#endif /* SATELLITE_TIME_TAG_H */
