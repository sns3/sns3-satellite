/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef CBR_APPLICATION_H
#define CBR_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class Socket;

/**
* \ingroup satellite
*
* \brief Generate traffic to a single destination according to an
*        CBR pattern.
*
* This traffic generator follows an CBR pattern: after
* Application::StartApplication
* is called, CBR sending starts. The sending continues as long as applications runs.
* This CBR traffic is characterized by the specified "data rate", "packet size" and "interval".
*
* Note:  When an application is started, the first packet transmission
* occurs _after_ given interval.  Note also, that an application sends a packet
* right after previous packet if interval is shorter than time needed for sending the packet.
*
* If the underlying socket type supports broadcast, this application
* will automatically enable the SetAllowBroadcast(true) socket option.
*/
class CbrApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  CbrApplication ();

  virtual ~CbrApplication();

  /**
   * \return pointer to associated socket
   */
  Ptr<Socket> GetSocket (void) const;

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  //int64_t AssignStreams (int64_t stream);

  /**
  * \brief returns the number of sent bytes
  * \return the number of received bytes
  */
  uint32_t GetSent (void) const;

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  // Event handlers
  void SendPacket ();

  Ptr<Socket>     m_socket;       // Associated socket
  Address         m_peer;         // Peer address
  Time            m_interval;     // Time interval for cbr sending
  DataRate        m_cbrRate;      // Rate that data is generated
  uint32_t        m_pktSize;      // Size of packets
  Time            m_lastStartTime; // Time last packet sent
  uint32_t        m_totTxBytes;   // Total bytes sent so far
  EventId         m_sendEvent;    // Eventid of pending "send packet" event
  bool            m_sending;      // True if currently in sending state
  TypeId          m_tid;
  TracedCallback<Ptr<const Packet> > m_txTrace;

private:
  void ScheduleNextTx ();
  void ScheduleStartEvent ();
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void HandleRead (Ptr<Socket> socket);
};

} // namespace ns3

#endif /* CBR_APPLICATION_H */
