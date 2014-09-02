/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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

#ifndef CBR_APPLICATION_H
#define CBR_APPLICATION_H

#include <ns3/address.h>
#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>
#include <ns3/nstime.h>

namespace ns3 {

class Socket;

/**
* \ingroup satellite
*
* \brief Generate traffic to a single destination according to a CBR pattern.
*
* This traffic generator follows a CBR pattern: after CbrApplication::StartApplication
* is called, CBR sending starts. The sending continues as long as applications runs.
* This CBR traffic is characterized by the specified "data rate", "packet size" and "interval".
*
* Note:  When an application is started, the first packet transmission
* occurs after given interval. The application sends a packet right after previous packet,
* if interval is shorter than time needed for sending the packet.
*
* If the underlying socket type supports broadcast, this application
* will automatically enable the SetAllowBroadcast(true) socket option.
*/
class CbrApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  /// Constructor for Cbr application.
  CbrApplication ();

  /// Destructor for Cbr application.
  virtual ~CbrApplication();

  /// Get the pointer to associated socket.
  Ptr<Socket> GetSocket (void) const;

  /**
  * \brief Get the number of sent bytes
  * \return the number of sent bytes
  */
  uint32_t GetSent (void) const;

  /**
   * \brief Get the destination address of the application
   * \return the destination address
   */
  const Address GetRemote (void) const;

protected:
  /// Do dispose actions.
  virtual void DoDispose (void);
private:
  Ptr<Socket>     m_socket;       // Associated socket
  Address         m_peer;         // Peer address
  Time            m_interval;     // Time interval for cbr sending
  uint32_t        m_pktSize;      // Size of packets
  Time            m_lastStartTime; // Time last packet sent
  uint32_t        m_totTxBytes;   // Total bytes sent so far
  EventId         m_sendEvent;    // Event id of pending "send packet" event
  TypeId          m_tid;
  bool            m_isStatisticsTagsEnabled;  ///< `EnableStatisticsTags` attribute.
  TracedCallback<Ptr<const Packet> > m_txTrace;

  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /// Create and send packet by scheduling next TX event.
  void SendPacket ();

  /// schedule next packet sending
  void ScheduleNextTx ();

  /**
   * Callback method to handle connection succeeded events
   *
   * \param socket Pointer to socket.
   */
  void ConnectionSucceeded (Ptr<Socket> socket);

  /**
   * Callback method to handle connection failed events
   *
   * \param socket Pointer to socket.
   */
  void ConnectionFailed (Ptr<Socket> socket);
};

} // namespace ns3

#endif /* CBR_APPLICATION_H */
