/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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
 *
 */

#ifndef KPI_HELPER_H
#define KPI_HELPER_H

#include <map>
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/ipv4-address.h"
#include "ns3/application-container.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-helper.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * KpiHelper is a base class for gathering application layer statistics from
 * simulations. KpiHelper is capable of tracing the transmitted packets/bytes,
 * received packets/bytes, packet delay and received IP level packets. This
 * information may be used to calculate the application throughput and error rate.
 * KpiHelper is using the packet trace callbacks defined in the applications and
 * currently supports CbrApplication, OnOffApplication and PacketSink application.
 * KpiHelper is not used by itself, but actual used KpiHelper is inherited from it,
 * so that it implements proper trace callback set functions to the actual applications.
 */
class KpiHelper
{
public:
  // Enumerator used for for configuring the KpiHelper for
  // RTN and FWD link applications
  typedef enum
  {
    KPI_FWD = 0,
    KPI_RTN = 1
  } KpiMode_t;

  /**
   * Constructor
   * \param mode Used KpiMode
   */
  KpiHelper (KpiHelper::KpiMode_t mode);

  /**
   * Destructor
   */
  virtual ~KpiHelper ();

  /**
   * Print the KPI helper content to standard output
   */
  virtual void Print (); // can't make this const because of non-const methods in FlowMonitor

  /**
   * Add applications
   * \param apps ApplicationContainer
   */
  virtual void AddSender (ApplicationContainer apps);

  /**
   * Add application
   * \param application
   */
  virtual void AddServer (Ptr<Application> app);

  /**
   * Add several applications (in this case PacketSink)
   * \param apps ApplicationContainer
   */
  virtual void AddSink (ApplicationContainer apps);

  /**
   * Add application (in this case PacketSink)
   * \param sink PacketSink
   */
  virtual void AddSink (Ptr<Application> app);

  /**
   * Trace callback from CBR application
   */
  void TxCallback (std::string context, Ptr<const Packet> packet);


protected:
  /**
   * Add a client counter for calculating user specific
   * statistics.
   */
  void AddClientCounter (const Ipv4Address address);

  /**
   * Configure the application in server mode (several
   * applications in one node)
   */
  void ConfigureAsServer (Ptr<Application> app);

  /**
   * Configure the application in client mode (only one
   * application per node)
   */
  void ConfigureAsClient (Ptr<Application> app);

  /**
   * Trace callback from PacketSink application
   */
  void RxCallback (std::string context, Ptr<const Packet> packet, const Address &from);

  /**
   * \param node pointer to the node
   * \return the IP address of the given node
   */
  static Ipv4Address GetAddress (Ptr<Node> node);

  /**
   * \param address an IP address
   * \return the given IP address in the form of printable string
   */
  static std::string AddressToString (const Ipv4Address address);

  double GetKbps (uint64_t bytes, Time duration);

  // Mode for the KPI collection depending on the traffic direction.
  KpiMode_t m_mode;

  FlowMonitorHelper m_flowMonitorHelper;

  /**
   * Configured server addresses. The server stands for a node in the
   * "Internet" which may hold several applications.
   */
  std::vector<Ipv4Address> m_serverAddresses;

  /**
   * Struct holding the client specific statistics
   */
  struct ClientCounter_t
  {
    uint64_t txBytes;
    uint64_t txPackets;
    uint64_t rxBytes;
    uint64_t rxPackets;
    uint64_t rxIpLevelPackets;
    Time sumPacketDelay;
    Time appStart;
    Time appStop;
  };

  /**
   * map of individual client counters. Key is the
   * Ipv4Address of the UT end user.
   */
  std::map<Ipv4Address, ClientCounter_t> m_clientCounters;
};


} // end of `namespace ns3`


#endif /* KPI_HELPER_H */
