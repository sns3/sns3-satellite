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
 * Modified for FrameLoadTrace trace source by:
 * - Budiarto Herman (budiarto.herman@magister.fi)
 */

#ifndef SATELLITE_FRAME_SYMBOL_LOAD_PROBE_H
#define SATELLITE_FRAME_SYMBOL_LOAD_PROBE_H

#include "ns3/nstime.h"
#include "ns3/traced-callback.h"
#include "ns3/probe.h"

namespace ns3 {

/**
 * \brief Probe to translate from a FrameLoadTrace trace source.
 *
 * This class is designed to probe an underlying ns3 TraceSource exporting a
 * frame load information.  This probe exports a trace source "Output" with two
 * arguments: frame ID (uint32_t) and ratio of allocated symbols (double).  The
 * trace sources emit values when either the probed trace source emits a new
 * value, or when SetValue () is called.
 */
class SatFrameSymbolLoadProbe : public Probe
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
  SatFrameSymbolLoadProbe ();

  /**
   * Destructor for SatFrameSymbolLoadProbe
   */
  virtual ~SatFrameSymbolLoadProbe ();

  /**
   * \brief Set a probe value
   *
   * \param frameId set the frame number equal to this
   * \param loadRatio set the traced ratio of allocated symbols equal to this
   */
  void SetValue (uint32_t frameId, double loadRatio);

  /**
   * \brief Set a probe value by its name in the Config system
   *
   * \param path config path to access the probe
   * \param frameId set the frame number equal to this
   * \param loadRatio set the traced ratio of allocated symbols equal to this
   */
  static void SetValueByPath (std::string path, uint32_t frameId, double loadRatio);

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
   * \brief Callback signature for frame load in unit of symbols.
   *
   * \param frameId The current frame number.
   * \param loadRatio the ratio of allocated symbols over the total number of
   *                  symbols in the frame.
   */
  typedef void (*FrameSymbolLoadCallback)
    (uint32_t frameId, double loadRatio);

private:
  /**
   * \brief Method to connect to an underlying ns3::TraceSource with
   * arguments of type double and const Address&
   *
   * \param frameId frame number
   * \param loadRatio the traced ratio of allocated symbols
   */
  void TraceSink (uint32_t frameId, double loadRatio);

  /// Output trace, the frame ID and ratio of allocated symbols
  TracedCallback<uint32_t, double> m_output;

};


} // namespace ns3

#endif // SATELLITE_FRAME_SYMBOL_LOAD_PROBE_H
