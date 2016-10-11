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
 * Modified for FrameUtLoadTrace trace source by:
 * - Budiarto Herman (budiarto.herman@magister.fi)
 */

#ifndef SATELLITE_FRAME_USER_LOAD_PROBE_H
#define SATELLITE_FRAME_USER_LOAD_PROBE_H

#include "ns3/nstime.h"
#include "ns3/traced-callback.h"
#include "ns3/probe.h"

namespace ns3 {

/**
 * \brief Probe to translate from a FrameUtLoadTrace trace source.
 *
 * This class is designed to probe an underlying ns3 TraceSource exporting a
 * frame load information.  This probe exports a trace source "Output" with two
 * arguments of type uint32_t: frame ID and number of scheduled users.  The
 * trace sources emit values when either the probed trace source emits a new
 * value, or when SetValue () is called.
 */
class SatFrameUserLoadProbe : public Probe
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
  SatFrameUserLoadProbe ();

  /**
   * Destructor for SatFrameUserLoadProbe
   */
  virtual ~SatFrameUserLoadProbe ();

  /**
   * \brief Set a probe value
   *
   * \param frameId set the frame number equal to this
   * \param utCount set the traced number of scheduled users equal to this
   */
  void SetValue (uint32_t frameId, uint32_t utCount);

  /**
   * \brief Set a probe value by its name in the Config system
   *
   * \param path config path to access the probe
   * \param frameId set the frame number equal to this
   * \param utCount set the traced number of scheduled users equal to this
   */
  static void SetValueByPath (std::string path, uint32_t frameId, uint32_t utCount);

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
   * \brief Callback signature for frame load in unit of users.
   *
   * \param frameId The current frame number.
   * \param utCount The number of scheduled users.
   */
  typedef void (*FrameUserLoadCallback)
    (uint32_t frameId, uint32_t utCount);

private:
  /**
   * \brief Method to connect to an underlying ns3::TraceSource with
   * arguments of type uint32_t and uint32_t
   *
   * \param frameId frame number
   * \param utCount the traced number of scheduled users
   */
  void TraceSink (uint32_t frameId, uint32_t utCount);

  /// Output trace, the frame ID and number of scheduled users
  TracedCallback<uint32_t, uint32_t> m_output;

};


} // namespace ns3

#endif // SATELLITE_FRAME_USER_LOAD_PROBE_H
