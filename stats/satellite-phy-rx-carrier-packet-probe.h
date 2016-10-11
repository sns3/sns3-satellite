/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Bucknell University
 * Copyright (c) 2014 Magister Solutions
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
 * Authors of original work (application-packet-probe.h) which this work
 * derives from:
 * - L. Felipe Perrone (perrone@bucknell.edu)
 * - Tiago G. Rodrigues (tgr002@bucknell.edu)
 * - Mitch Watrous (watrous@u.washington.edu)
 *
 * Modified for the signature of SatPhyRxCarrier trace sources by:
 * - Budiarto Herman (budiarto.herman@magister.fi)
 */

#ifndef SATELLITE_PHY_RX_CARRIER_PACKET_PROBE_H
#define SATELLITE_PHY_RX_CARRIER_PACKET_PROBE_H

#include <ns3/nstime.h>
#include <ns3/traced-callback.h>
#include <ns3/address.h>
#include <ns3/probe.h>

namespace ns3 {

/**
 * \brief Probe to translate from a TraceSource to two more easily parsed TraceSources.
 *
 * This class is designed to probe an underlying ns3 TraceSource exporting a
 * packet burst information from SatPhyRxCarrier.  This probe exports a trace
 * source "Output" with arguments of type uint32_t, const Address&, and bool.
 * This probe exports other trace sources "OutputUinteger" and "OutputBool"
 * with arguments of type uint32_t and bool, respectively.  The trace sources
 * emit values when either the probed trace source emits a new value, or when
 * SetValue () is called.
 */
class SatPhyRxCarrierPacketProbe : public Probe
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Default constructor.
   */
  SatPhyRxCarrierPacketProbe ();

  /**
   * Destructor for SatPhyRxCarrierPacketProbe
   */
  virtual ~SatPhyRxCarrierPacketProbe ();

  /**
   * \brief Set a probe value
   *
   * \param nPackets set the traced number of packets in the packet burst equal to this
   * \param address set the socket address for the traced packet burst equal to this
   * \param statusFlag set the traced packet burst status flag equal to this
   */
  void SetValue (uint32_t nPackets, const Address& address, bool statusFlag);

  /**
   * \brief Set a probe value by its name in the Config system
   *
   * \param path config path to access the probe
   * \param nPackets set the traced number of packets in the packet burst equal to this
   * \param address set the socket address for the traced packet burst equal to this
   * \param statusFlag set the traced packet burst status flag equal to this
   */
  static void SetValueByPath (std::string path, uint32_t nPackets,
                              const Address& address, bool statusFlag);

  /**
   * \brief connect to a trace source attribute provided by a given object
   *
   * \param traceSource the name of the attribute TraceSource to connect to
   * \param obj ns3::Object to connect to
   * \return true if the trace source was successfully connected
   */
  virtual bool ConnectByObject (std::string traceSource, Ptr<Object> obj);

  /**
   * \brief connect to a trace source provided by a config path
   *
   * \param path Config path to bind to
   *
   * Note, if an invalid path is provided, the probe will not be connected
   * to anything.
   */
  virtual void ConnectByPath (std::string path);

  /**
   * \brief Common callback signature for trace sources related to packets
   *        reception by PHY and its status.
   * \param nPackets number of upper layer packets in the received packet burst.
   * \param from the MAC48 address of the sender of the packets.
   * \param status whether a PHY error or collision has occurred.
   */
  typedef void (*RxStatusCallback)
    (uint32_t nPackets, const Address &from, bool status);

private:
  /**
   * \brief Method to connect to an underlying ns3::TraceSource with
   * arguments of type double and const Address&
   *
   * \param nPackets the traced number of packets in the packet burst
   * \param address the socket address for the traced packet burst
   * \param statusFlag the traced packet burst status flag
   *
   */
  void TraceSink (uint32_t nPackets, const Address& address, bool statusFlag);

  /// Output trace, the number of packets, source address, and status flag
  TracedCallback<uint32_t, const Address&, bool> m_output;
  /// Output trace, previous and current number of packets
  TracedCallback<uint32_t, uint32_t> m_outputUinteger;
  /// Output trace, previous and current status flag
  TracedCallback<bool, bool> m_outputBool;

  /// The traced number of packets in the packet burst.
  uint32_t m_nPackets;
  /// The socket address for the traced packet burst.
  Address m_address;
  /// The traced packet burst status flag.
  bool m_statusFlag;

};


} // namespace ns3

#endif // SATELLITE_PHY_RX_CARRIER_PACKET_PROBE_H
