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
 * Modified for FrameUtLoadTrace trace source by:
 * - Budiarto Herman (budiarto.herman@magister.fi)
 */

#include "ns3/satellite-frame-symbol-load-probe.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"

NS_LOG_COMPONENT_DEFINE ("SatFrameSymbolLoadProbe");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFrameSymbolLoadProbe)
;

TypeId
SatFrameSymbolLoadProbe::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatFrameSymbolLoadProbe")
    .SetParent<Probe> ()
    .AddConstructor<SatFrameSymbolLoadProbe> ()
    .AddTraceSource ( "Output",
                      "The frame ID and the ratio of allocated symbols that serve as the output for this probe",
                      MakeTraceSourceAccessor (&SatFrameSymbolLoadProbe::m_output),
                      "ns3::SatFrameSymbolLoadProbe::FrameSymbolLoadCallback")
  ;
  return tid;
}

SatFrameSymbolLoadProbe::SatFrameSymbolLoadProbe ()
{
  NS_LOG_FUNCTION (this);
}

SatFrameSymbolLoadProbe::~SatFrameSymbolLoadProbe ()
{
  NS_LOG_FUNCTION (this);
}

void
SatFrameSymbolLoadProbe::SetValue (uint32_t frameId, double loadRatio)
{
  NS_LOG_FUNCTION (this << frameId << loadRatio);
  m_output (frameId, loadRatio);
}

void
SatFrameSymbolLoadProbe::SetValueByPath (std::string path, uint32_t frameId, double loadRatio)
{
  NS_LOG_FUNCTION (path << frameId << loadRatio);
  Ptr<SatFrameSymbolLoadProbe> probe = Names::Find<SatFrameSymbolLoadProbe> (path);
  NS_ASSERT_MSG (probe, "Error:  Can't find probe for path " << path);
  probe->SetValue (frameId, loadRatio);
}

bool
SatFrameSymbolLoadProbe::ConnectByObject (std::string traceSource, Ptr<Object> obj)
{
  NS_LOG_FUNCTION (this << traceSource << obj);
  NS_LOG_DEBUG ("Name of probe (if any) in names database: " << Names::FindPath (obj));
  bool connected = obj->TraceConnectWithoutContext (traceSource, MakeCallback (&ns3::SatFrameSymbolLoadProbe::TraceSink, this));
  return connected;
}

void
SatFrameSymbolLoadProbe::ConnectByPath (std::string path)
{
  NS_LOG_FUNCTION (this << path);
  NS_LOG_DEBUG ("Name of probe to search for in config database: " << path);
  Config::ConnectWithoutContext (path, MakeCallback (&ns3::SatFrameSymbolLoadProbe::TraceSink, this));
}

void
SatFrameSymbolLoadProbe::TraceSink (uint32_t frameId, double loadRatio)
{
  NS_LOG_FUNCTION (this << frameId << loadRatio);
  if (IsEnabled ())
    {
      m_output (frameId, loadRatio);
    }
}

} // namespace ns3
