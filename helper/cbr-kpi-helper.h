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

#ifndef CBR_KPI_HELPER_H
#define CBR_KPI_HELPER_H

#include <map>
#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/ipv4-address.h>
#include <ns3/application-container.h>
#include <ns3/packet-sink.h>
#include <ns3/cbr-application.h>
#include <ns3/flow-monitor-helper.h>


namespace ns3 {


class CbrKpiHelper
{
public:

  CbrKpiHelper ();

  /**
   * Print the KPI helper content to standard output
   */
  void Print (); // can't make this const because of non-const methods in FlowMonitor

  /**
   * Add several "server" applications (in this case CBR)
   * \param apps ApplicationContainer
   */
  void AddServer (ApplicationContainer apps);

  /**
   * Add "server" application (in this case CBR)
   * \param cbr CBR application
   */
  void AddServer (Ptr<CbrApplication> cbr);

  /**
   * Add several "client" applications (in this case PacketSink)
   * \param apps ApplicationContainer
   */
  void AddClient (ApplicationContainer apps);

  /**
   * Add "client" application (in this case PacketSink)
   * \param sink PacketSink
   */
  void AddClient (Ptr<PacketSink> sink);

private:

  /**
   * Trace callback from CBR application
   */
  void TxCallback (std::string context, Ptr<const Packet> packet);

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

  FlowMonitorHelper m_flowMonitorHelper;
  std::vector<Ipv4Address> m_serverAddresses;

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

  std::map<Ipv4Address, ClientCounter_t> m_clientCounters;

}; // end of `class CbrKpiHelper`


} // end of `namespace ns3`


#endif /* CBR_KPI_HELPER_H */
