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
  SatTimeTag ();

  /**
   * Constructor with initialization parameters.
   * \param senderTimestamp 
   */
  SatTimeTag (Time senderTimestamp);

  /**
   * Serializes information to buffer from this instance of SatTimeTag
   * \param i Buffer in which the information is serialized
   */
  virtual void  Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatTimeTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void  Deserialize (TagBuffer i);

  /**
   * Get serialized size of SatTimeTag
   * \return Serialized size in bytes
   */
  virtual uint32_t  GetSerializedSize () const;

  /**
   * Print time stamp of this instance of SatTimeTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Get sender time stamp of this tag.
   * \return Sender timestamp
   */
  Time GetSenderTimestamp (void) const;

  /**
   * Set sender time stamp of this tag.
   * \param senderTimestamp Sender time stamp
   */
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
  SatPhyTimeTag ();

  /**
   * Constructor with initialization parameters.
   * \param senderTimestamp 
   */
  SatPhyTimeTag (Time senderTimestamp);

  /**
   * Serializes information to buffer from this instance of SatPhyTimeTag
   * \param i Buffer in which the information is serialized
   */
  virtual void  Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatPhyTimeTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void  Deserialize (TagBuffer i);

  /**
   * Get serialized size of SatPhyTimeTag
   * \return Serialized size in bytes
   */
  virtual uint32_t  GetSerializedSize () const;

  /**
   * Print time stamp of this instance of SatPhyTimeTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Get sender time stamp of this tag.
   * \return Sender timestamp
   */
  Time GetSenderTimestamp (void) const;

  /**
   * Set sender time stamp of this tag.
   * \param senderTimestamp Sender time stamp
   */
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
  SatMacTimeTag ();

  /**
   * Constructor with initialization parameters.
   * \param senderTimestamp 
   */
  SatMacTimeTag (Time senderTimestamp);

  /**
   * Serializes information to buffer from this instance of SatMacTimeTag
   * \param i Buffer in which the information is serialized
   */
  virtual void  Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatMacTimeTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void  Deserialize (TagBuffer i);

  /**
   * Get serialized size of SatMacTimeTag
   * \return Serialized size in bytes
   */
  virtual uint32_t  GetSerializedSize () const;

  /**
   * Print time stamp of this instance of SatMacTimeTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Get sender time stamp of this tag.
   * \return Sender timestamp
   */
  Time GetSenderTimestamp (void) const;

  /**
   * Set sender time stamp of this tag.
   * \param senderTimestamp Sender time stamp
   */
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
  SatDevTimeTag ();

  /**
   * Constructor with initialization parameters.
   * \param senderTimestamp 
   */
  SatDevTimeTag (Time senderTimestamp);

  /**
   * Serializes information to buffer from this instance of SatDevTimeTag
   * \param i Buffer in which the information is serialized
   */
  virtual void  Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of SatDevTimeTag
   * \param i Buffer from which the information is deserialized
   */
  virtual void  Deserialize (TagBuffer i);

  /**
   * Get serialized size of SatDevTimeTag
   * \return Serialized size in bytes
   */
  virtual uint32_t  GetSerializedSize () const;

  /**
   * Print time stamp of this instance of SatDevTimeTag
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Get sender time stamp of this tag.
   * \return Sender timestamp
   */
  Time GetSenderTimestamp (void) const;

  /**
   * Set sender time stamp of this tag.
   * \param senderTimestamp Sender time stamp
   */
  void SetSenderTimestamp (Time senderTimestamp);

private:
  Time m_senderTimestamp;

};



} //namespace ns3

#endif /* SATELLITE_TIME_TAG_H */
