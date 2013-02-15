/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "cbr-application.h"

NS_LOG_COMPONENT_DEFINE ("CbrApplication");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CbrApplication);

TypeId
CbrApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CbrApplication")
    .SetParent<Application> ()
    .AddConstructor<CbrApplication> ()
    .AddAttribute ("DataRate", "The data rate for constant sending.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&CbrApplication::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of constant packets sent.",
                   UintegerValue (512),
                   MakeUintegerAccessor (&CbrApplication::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&CbrApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("Interval", "Interval to send constant packets. The value zero means that no sending.",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&CbrApplication::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&CbrApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&CbrApplication::m_txTrace))
  ;
  return tid;
}


CbrApplication::CbrApplication ()
  : m_socket (0),
    m_lastStartTime (Seconds (0)),
    m_totTxBytes (0)
{
  NS_LOG_FUNCTION (this);
}

CbrApplication::~CbrApplication()
{
  NS_LOG_FUNCTION (this);
}


Ptr<Socket>
CbrApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
CbrApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void CbrApplication::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind ();

      m_socket->SetConnectCallback (
          MakeCallback (&CbrApplication::ConnectionSucceeded, this),
          MakeCallback (&CbrApplication::ConnectionFailed, this));

      m_socket->Connect ((const Address&)m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();
    }
  // Insure no pending event
  Simulator::Cancel (m_sendEvent);
}

void CbrApplication::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  Simulator::Cancel (m_startEvent);
  Simulator::Cancel (m_sendEvent);

  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("CbrApplication found null socket to close in StopApplication");
    }
}

uint32_t CbrApplication::GetSent (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totTxBytes;
}

// Event handlers
void CbrApplication::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
}

// Private helpers
void CbrApplication::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  uint32_t bits = m_pktSize * 8;
  NS_LOG_LOGIC ("bits = " << bits);
  Time nextTime (Seconds (bits / static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet

  // use interval for next sending in case that time for sending of packet doesn't last longer than interval
  if ( nextTime < m_interval )
    {
      nextTime = m_interval;
    }

  NS_LOG_LOGIC ("nextTime = " << nextTime);

  m_sendEvent = Simulator::Schedule (nextTime, &CbrApplication::SendPacket, this);
}

void CbrApplication::ScheduleStartEvent ()
{  // Schedules the event to start sending cbr data
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("start at " << m_interval);
  m_startEvent = Simulator::Schedule (m_interval, &CbrApplication::StartSending, this);
}

void CbrApplication::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> packet = Create<Packet> (m_pktSize);
  m_txTrace (packet);
  m_socket->Send (packet);
  m_totTxBytes += m_pktSize;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s cbr application sent "
                   <<  packet->GetSize () << " bytes to "
                   << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totTxBytes << " bytes");
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s cbr application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totTxBytes << " bytes");
    }
  m_lastStartTime = Simulator::Now ();

  ScheduleNextTx ();
}


void CbrApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  ScheduleStartEvent ();
}

void CbrApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

} // Namespace ns3
