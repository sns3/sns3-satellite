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
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatQueue implements a queue utilized in the satellite module.
 * It is utilized in both FWD and RTN links for both control and user data.
 *
*/

class SatQueue : public Object
{
public:
  static TypeId GetTypeId (void);

  struct QueueStats_t
  {
    QueueStats_t ()
    : m_incomingRateKbps (0.0),
      m_outgoingRateKbps (0.0),
      m_volumeInBytes (0),
      m_volumeOutBytes (0),
      m_queueSizeBytes (0)
    {
    }
    double   m_incomingRateKbps;
    double   m_outgoingRateKbps;
    uint32_t m_volumeInBytes;
    uint32_t m_volumeOutBytes;
    uint32_t m_queueSizeBytes;
  };

  typedef enum
  {
    FIRST_BUFFERED_PKT,
    BUFFERED_PKT
  } QueueEvent_t;

  /**
   * Default constructor
   */
  SatQueue ();

  /**
   * Constructor
   */
  SatQueue (uint8_t flowId);
  ~SatQueue ();

  virtual void DoDispose ();

  /**
   * Callback to indicate queue related event
   * \param SatQueue::QueueEvent_t Event type
   * \param uint32_t Queue id
   * \return void
   */
  typedef Callback<void, SatQueue::QueueEvent_t, uint8_t> QueueEventCallback;

  /**
   * Is the queue empty
   * \return true if the queue is empty; false otherwise
   */
  virtual bool IsEmpty (void) const;

  /**
   * Enque pushes packet to the packet container (back)
   * \param p Packet
   * \return bool indicating whether the enque was successfull
   */
  virtual bool Enqueue (Ptr<Packet> p);

  /**
   * Deque takes packet from the packet container (front)
   * \return p Packet
   */
  virtual Ptr<Packet> Dequeue (void);

  /**
   * PushFront pushes a fragmented packet back to the front
   * of the packet container
   * \param p Packet
   */
  virtual void PushFront (Ptr<Packet> p);

  /**
   * Get a copy of the item at the front of the queue without removing it
   * \return Pointer to the packet
   */
  virtual Ptr<const Packet> Peek (void) const;

  /**
   * Flush the queue.
   */
  void DequeueAll (void);

  /**
   * Configured flow index for this queue
   * \param flowId
   */
  void SetFlowId (uint32_t flowId);

  /**
   * Add queue event callback
   * \param cb Callback
   */
  void AddQueueEventCallback (SatQueue::QueueEventCallback cb);

  /**
   * \return The number of packets currently stored in the Queue
   */
  uint32_t GetNPackets (void) const;

  /**
   * \return The number of bytes currently occupied by the packets in the Queue
   */
  uint32_t GetNBytes (void) const;

  /**
   * \return The total number of bytes received by this Queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   *
   */
  uint32_t GetTotalReceivedBytes (void) const;
  /**
   * \return The total number of packets received by this Queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   */
  uint32_t GetTotalReceivedPackets (void) const;
  /**
   * \return The total number of bytes dropped by this Queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   */
  uint32_t GetTotalDroppedBytes (void) const;
  /**
   * \return The total number of bytes dropped by this Queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   */
  uint32_t GetTotalDroppedPackets (void) const;
  /**
   * Resets the counts for dropped packets, dropped bytes, received packets, and
   * received bytes.
   */
  void ResetStatistics (void);

  /**
   * GetQueueStatistics returns a struct of KPIs
   * \param reset Reset flag indicating whether the statistics should be reset now
   * \return QueueStats_t Struct of KPIs
   */
  QueueStats_t GetQueueStatistics (bool reset);

  /**
   * Method checks how many packets are smaller or equal in size than the
   * maximum packets size threshold specified as an argument. Note, that each
   * queue is gone through from the front up until there is first packet larger
   * than threshold.
   * \param maxPacketSize Maximum packet size threshold in Bytes
   * \return uint32_t Number of packets
   */
  uint32_t GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const;

protected:

  /**
   *  \brief Drop a packet
   *  \param packet packet that was dropped
   *  This method is called by subclasses to notify parent (this class) of packet drops.
   */
  void Drop (Ptr<Packet> packet);

private:

  /**
   * Send queue event to all registered callbacks
   * /param event Queue event
   */
  void SendEvent (QueueEvent_t event);

  /**
   * Reset the short term statistics. Short term reflects here e.g. to
   * superframe duration.
   */
  void ResetShortTermStatistics ();

  typedef std::vector<QueueEventCallback> EventCallbackContainer_t;
  typedef std::deque<Ptr<Packet> > PacketContainer_t;

  /**
   * Container of callbacks for queue related events
   */
  EventCallbackContainer_t m_queueEventCallbacks;

  /**
   * Packet container
   */
  PacketContainer_t m_packets;

  /**
   * Maximum allowed packets within the packet container
   */
  uint32_t m_maxPackets;

  /**
   * An unique id for each queue
   */
  uint8_t m_flowId;

  // Statistics
  uint32_t m_nBytes;
  uint32_t m_nTotalReceivedBytes;
  uint32_t m_nPackets;
  uint32_t m_nTotalReceivedPackets;
  uint32_t m_nTotalDroppedBytes;
  uint32_t m_nTotalDroppedPackets;

  // Short term statistics
  uint32_t m_nEnqueBytesSinceReset;
  uint32_t m_nDequeBytesSinceReset;
  Time m_statResetTime;

  // Trace callbacks
  TracedCallback<Ptr<const Packet> > m_traceEnqueue;
  TracedCallback<Ptr<const Packet> > m_traceDequeue;
  TracedCallback<Ptr<const Packet> > m_traceDrop;
};


} // namespace ns3


#endif /* SATELLITE_QUEUE_H_ */
