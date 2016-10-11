/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 */

#ifndef SATELLITE_PROPAGATION_DELAY_MODEL_H
#define SATELLITE_PROPAGATION_DELAY_MODEL_H

#include "ns3/propagation-delay-model.h"
#include "ns3/mobility-model.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief The propagation delay is constant in time.
 */
class SatConstantPropagationDelayModel : public PropagationDelayModel
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatConstantPropagationDelayModel ();

  /**
   * \brief Get the propagation delay in Time
   * \param a the source
   * \param b the destination
   * \returns Propagation delay.
   */
  virtual Time GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  /**
   * \brief Get the propagation delay in Time
   * \returns Propagation delay.
   */
  virtual Time GetDelay (void) const;

  /**
   * Set constant propagation delay.
   * \param delay Delay in Time.
   */
  void SetDelay (Time delay);

  /**
   * DoAssignStreams need to be implemented due to inheritance from
   * PropagationDelayModel
   */
  int64_t DoAssignStreams (int64_t s);

private:
  Time m_delay;
};


} // namespace ns3

#endif /* SATELLITE_PROPAGATION_DELAY_MODEL_H */
