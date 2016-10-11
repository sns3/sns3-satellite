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
 * \brief SatQueue implements a queue utilized in the satellite module. SatQueue
 * is utilized in both FWD and RTN link to store incoming packets inside either
 * SatGenericStreamEncapsulator (FWD link) or SatReturnLinkEncapsulator (RTN link).
 * SatQueue is capable of collecting statistics from the incoming and outgoing
 * bits and packets.
 *
*/

class SatQueue : public Object
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * QueueStats_t definition for passing queue related statistics
   * to any interested modules.
   */
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

  /**
   * Destructor for SatQueue
   */
  ~SatQueue ();

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * \brief Callback to indicate queue related event
   * \param SatQueue::QueueEvent_t Event type
   * \param uint32_t Queue id
   * \return void
   */
  typedef Callback<void, SatQueue::QueueEvent_t, uint8_t> QueueEventCallback;

  /**
   * \brief Is the queue empty
   * \return true if the queue is empty; false otherwise
   */
  virtual bool IsEmpty (void) const;

  /**
   * \brief Enque pushes packet to the packet container (back)
   * \param p Packet
   * \return bool indicating whether the enque was successfull
   */
  virtual bool Enqueue (Ptr<Packet> p);

  /**
   * \brief Deque takes packet from the packet container (front)
   * \return p Packet
   */
  virtual Ptr<Packet> Dequeue (void);

  /**
   * \brief PushFront pushes a fragmented packet back to the front
   * of the packet container
   * \param p Packet
   */
  virtual void PushFront (Ptr<Packet> p);

  /**
   * \brief Get a copy of the item at the front of the queue without removing it
   * \return Pointer to the packet
   */
  virtual Ptr<const Packet> Peek (void) const;

  /**
   * \brief Flush the queue.
   */
  void DequeueAll (void);

  /**
   * \brief Configured flow index for this queue
   * \param flowId
   */
  void SetFlowId (uint32_t flowId);

  /**
   * \brief Add queue event callback
   * \param cb Callback
   */
  void AddQueueEventCallback (SatQueue::QueueEventCallback cb);

  /**
   * \brief Get number of packets currently stored in the queue
   * \return Number of packets in queue
   */
  uint32_t GetNPackets (void) const;

  /**
   * \brief Get number of bytes currently stored in the queue
   * \return Bytes in queue
   */
  uint32_t GetNBytes (void) const;

  /**
   * \brief Get total number of bytes received by this queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   * \return Received bytes
   */
  uint32_t GetTotalReceivedBytes (void) const;

  /**
   * \brief Get total number of packets received by this Queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   * \return Received packets
   */
  uint32_t GetTotalReceivedPackets (void) const;
  /**
   * \brief Get total number of bytes dropped by this Queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   * \return Dropped bytes
   */
  uint32_t GetTotalDroppedBytes (void) const;

  /**
   * \brief Get total number of bytes dropped by this Queue since the
   * simulation began, or since ResetStatistics was called, according to
   * whichever happened more recently
   * \return Dropped packets
   */
  uint32_t GetTotalDroppedPackets (void) const;

  /**
   * \brief Resets the counts for dropped packets, dropped bytes, received packets, and
   * received bytes.
   */
  void ResetStatistics (void);

  /**
   * \brief GetQueueStatistics returns a struct of KPIs
   * \param reset Reset flag indicating whether the statistics should be reset now
   * \return QueueStats_t Struct of KPIs
   */
  QueueStats_t GetQueueStatistics (bool reset);

  /**
   * \brief Method checks how many packets are smaller or equal in size than the
   * maximum packets size threshold specified as an argument. Note, that each
   * queue is gone through from the front up until there is first packet larger
   * than threshold.
   * \param maxPacketSizeBytes Maximum packet size threshold in Bytes
   * \return Number of packets
   */
  uint32_t GetNumSmallerPackets (uint32_t maxPacketSizeBytes) const;

protected:
  /**
   * \brief Drop a packet
   * \param packet packet that was dropped
   */
  void Drop (Ptr<Packet> packet);

private:
  /**
   * \brief Send queue event to all registered callbacks
   * \param event Queue event
   */
  void SendEvent (SatQueue::QueueEvent_t event);

  /**
   * \brief Reset the short term statistics. Short term reflects here e.g. to
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
