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
 * Modified for Sinr trace source by:
 * - Budiarto Herman (budiarto.herman@magister.fi)
 */

#include <ns3/satellite-sinr-probe.h>
#include <ns3/log.h>
#include <ns3/names.h>
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/traced-value.h>

NS_LOG_COMPONENT_DEFINE ("SatSinrProbe");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatSinrProbe)
;

TypeId
SatSinrProbe::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatSinrProbe")
    .SetParent<Probe> ()
    .AddConstructor<SatSinrProbe> ()
    .AddTraceSource ( "Output",
                      "The SINR plus its socket address that serve as the output for this probe",
                      MakeTraceSourceAccessor (&SatSinrProbe::m_output),
                      "ns3::SatSinrProbe::SinrCallback")
    .AddTraceSource ( "OutputSinr",
                      "The SINR",
                      MakeTraceSourceAccessor (&SatSinrProbe::m_outputSinr),
                      "ns3::TracedValue::DoubleCallback")
  ;
  return tid;
}

SatSinrProbe::SatSinrProbe ()
{
  NS_LOG_FUNCTION (this);
}

SatSinrProbe::~SatSinrProbe ()
{
  NS_LOG_FUNCTION (this);
}

void
SatSinrProbe::SetValue (double sinrDb, const Address& address)
{
  NS_LOG_FUNCTION (this << sinrDb << address);
  m_output (sinrDb, address);
  m_outputSinr (m_sinr, sinrDb);
  m_sinr = sinrDb;
  m_address = address;
}

void
SatSinrProbe::SetValueByPath (std::string path, double sinrDb, const Address& address)
{
  NS_LOG_FUNCTION (path << sinrDb << address);
  Ptr<SatSinrProbe> probe = Names::Find<SatSinrProbe> (path);
  NS_ASSERT_MSG (probe, "Error:  Can't find probe for path " << path);
  probe->SetValue (sinrDb, address);
}

bool
SatSinrProbe::ConnectByObject (std::string traceSource, Ptr<Object> obj)
{
  NS_LOG_FUNCTION (this << traceSource << obj);
  NS_LOG_DEBUG ("Name of probe (if any) in names database: " << Names::FindPath (obj));
  bool connected = obj->TraceConnectWithoutContext (traceSource, MakeCallback (&ns3::SatSinrProbe::TraceSink, this));
  return connected;
}

void
SatSinrProbe::ConnectByPath (std::string path)
{
  NS_LOG_FUNCTION (this << path);
  NS_LOG_DEBUG ("Name of probe to search for in config database: " << path);
  Config::ConnectWithoutContext (path, MakeCallback (&ns3::SatSinrProbe::TraceSink, this));
}

void
SatSinrProbe::TraceSink (double sinrDb, const Address& address)
{
  NS_LOG_FUNCTION (this << sinrDb << address);
  if (IsEnabled ())
    {
      m_output (sinrDb, address);
      m_outputSinr (m_sinr, sinrDb);
      m_sinr = sinrDb;
      m_address = address;
    }
}

} // namespace ns3
