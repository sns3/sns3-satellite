/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#ifndef SATELLITE_UT_HANDOVER_MODULE_H
#define SATELLITE_UT_HANDOVER_MODULE_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/callback.h>

#include "satellite-antenna-gain-pattern-container.h"


namespace ns3 {

/**
 * \ingroup satellite
 * \brief UT handover module
 */
class SatUtHandoverModule : public Object
{
public:
  /**
   * \brief Handover recommendation message sending callback
   * \param uint32_t The beam ID this UT want to change to
   */
  typedef Callback<void, uint32_t> HandoverRequestCallback;

  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * Default constructor, which is not used.
   */
  SatUtHandoverModule ();

  /**
   * Construct a SatUtHandoverModule
   * \param agpContainer the antenna gain patterns of the simulation
   */
  SatUtHandoverModule (Ptr<SatAntennaGainPatternContainer> agpContainer);

  /**
   * Destroy a SatUtHandoverModule
   */
  ~SatUtHandoverModule ();

  /**
   * \brief Set the handover recommendation message sending callback.
   * \param cb callback to send handover recommendation messages
   */
  void SetHandoverRequestCallback (SatUtHandoverModule::HandoverRequestCallback cb);

  /**
   * \brief Get the best beam ID
   * \return The best beam ID
   */
  uint32_t GetAskedBeamId ();

  /**
   * \brief Inspect whether or not the given beam is still suitable for
   * the underlying mobility model.
   * \param beamId The current beam ID the underlying mobility model is emitting in
   * \return whether or not an handover recommendation has been sent
   */
  bool CheckForHandoverRecommendation (uint32_t beamId);

private:
  HandoverRequestCallback m_handoverCallback;

  Ptr<SatAntennaGainPatternContainer> m_antennaGainPatterns;

  Time m_lastMessageSentAt;
  Time m_repeatRequestTimeout;
  bool m_hasPendingRequest;
  uint32_t m_askedBeamId;

  TracedCallback<double> m_antennaGainTrace;
};

}

#endif /* SATELLITE_UT_HANDOVER_MODULE_H */
