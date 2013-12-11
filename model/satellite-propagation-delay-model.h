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
 * \brief the propagation delay is constant (for satellite use)
 */
class SatConstantPropagationDelayModel : public PropagationDelayModel
{
public:
  static TypeId GetTypeId (void);

  SatConstantPropagationDelayModel ();

  /**
   * \param a the source
   * \param b the destination
   * \returns the propagation delay (s).
   */
  virtual Time GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  virtual double GetDelay (void) const;
  void SetDelay (double delay);

private:
  virtual int64_t DoAssignStreams (int64_t stream);
  double m_delay;
};


} // namespace ns3

#endif /* SATELLITE_PROPAGATION_DELAY_MODEL_H */
