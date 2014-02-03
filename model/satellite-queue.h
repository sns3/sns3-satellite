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


#ifndef SATELLITE_QUEUE_H_
#define SATELLITE_QUEUE_H_

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief
 *
 *
*/

class SatQueue : public Queue
{
public:
  static TypeId GetTypeId (void);

  /**
   * Default constructor
   */
  SatQueue ();
  ~SatQueue ();

  /**
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   *
   */
  void SetMode (SatQueue::QueueMode mode);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  SatQueue::QueueMode GetMode (void);

private:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  std::queue<Ptr<Packet> > m_packets;
  uint32_t m_maxPackets;
  uint32_t m_maxBytes;
  uint32_t m_bytesInQueue;
  QueueMode m_mode;

};


} // namespace ns3


#endif /* SATELLITE_QUEUE_H_ */
