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
 * Authors of original work (application-packet-probe.cc) which this work
 * derives from:
 * - L. Felipe Perrone (perrone@bucknell.edu)
 * - Tiago G. Rodrigues (tgr002@bucknell.edu)
 * - Mitch Watrous (watrous@u.washington.edu)
 *
 * Modified for the signature of SatPhyRxCarrier trace sources by:
 * - Budiarto Herman (budiarto.herman@magister.fi)
 */

#include <ns3/satellite-phy-rx-carrier-packet-probe.h>
#include <ns3/log.h>
#include <ns3/names.h>
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/traced-value.h>

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierPacketProbe");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhyRxCarrierPacketProbe)
;

TypeId
SatPhyRxCarrierPacketProbe::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierPacketProbe")
    .SetParent<Probe> ()
    .AddConstructor<SatPhyRxCarrierPacketProbe> ()
    .AddTraceSource ( "Output",
                      "The number of packets in the packet burst, its "
                      "associated socket address, and a status flag, that "
                      "serve as the output for this probe",
                      MakeTraceSourceAccessor (&SatPhyRxCarrierPacketProbe::m_output),
                      "ns3::SatPhyRxCarrierPacketProbe::RxStatusCallback")
    .AddTraceSource ( "OutputUinteger",
                      "The number of packets",
                      MakeTraceSourceAccessor (&SatPhyRxCarrierPacketProbe::m_outputUinteger),
                      "ns3::TracedValue::UintCallback")
    .AddTraceSource ( "OutputBool",
                      "The status flag",
                      MakeTraceSourceAccessor (&SatPhyRxCarrierPacketProbe::m_outputBool),
                      "ns3::TracedValue::BoolCallback")
  ;
  return tid;
}

SatPhyRxCarrierPacketProbe::SatPhyRxCarrierPacketProbe ()
{
  NS_LOG_FUNCTION (this);
}

SatPhyRxCarrierPacketProbe::~SatPhyRxCarrierPacketProbe ()
{
  NS_LOG_FUNCTION (this);
}

void
SatPhyRxCarrierPacketProbe::SetValue (uint32_t nPackets,
                                      const Address& address,
                                      bool statusFlag)
{
  NS_LOG_FUNCTION (this << nPackets << address << statusFlag);
  m_output (nPackets, address, statusFlag);
  m_outputUinteger (m_nPackets, nPackets);
  m_outputBool (m_statusFlag, statusFlag);
  m_nPackets = nPackets;
  m_address = address;
  m_statusFlag = statusFlag;
}

void
SatPhyRxCarrierPacketProbe::SetValueByPath (std::string path,
                                            uint32_t nPackets,
                                            const Address& address,
                                            bool statusFlag)
{
  NS_LOG_FUNCTION (path << nPackets << address << statusFlag);
  Ptr<SatPhyRxCarrierPacketProbe> probe = Names::Find<SatPhyRxCarrierPacketProbe> (path);
  NS_ASSERT_MSG (probe, "Error:  Can't find probe for path " << path);
  probe->SetValue (nPackets, address, statusFlag);
}

bool
SatPhyRxCarrierPacketProbe::ConnectByObject (std::string traceSource, Ptr<Object> obj)
{
  NS_LOG_FUNCTION (this << traceSource << obj);
  NS_LOG_DEBUG ("Name of probe (if any) in names database: " << Names::FindPath (obj));
  bool connected = obj->TraceConnectWithoutContext (traceSource, MakeCallback (&ns3::SatPhyRxCarrierPacketProbe::TraceSink, this));
  return connected;
}

void
SatPhyRxCarrierPacketProbe::ConnectByPath (std::string path)
{
  NS_LOG_FUNCTION (this << path);
  NS_LOG_DEBUG ("Name of probe to search for in config database: " << path);
  Config::ConnectWithoutContext (path, MakeCallback (&ns3::SatPhyRxCarrierPacketProbe::TraceSink, this));
}

void
SatPhyRxCarrierPacketProbe::TraceSink (uint32_t nPackets,
                                       const Address& address,
                                       bool statusFlag)
{
  NS_LOG_FUNCTION (this << nPackets << address << statusFlag);
  if (IsEnabled ())
    {
      m_output (nPackets, address, statusFlag);
      m_outputUinteger (m_nPackets, nPackets);
      m_outputBool (m_statusFlag, statusFlag);
      m_nPackets = nPackets;
      m_address = address;
      m_statusFlag = statusFlag;
    }
}

} // namespace ns3
