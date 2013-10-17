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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#ifndef SATELLITE_FADER_H
#define SATELLITE_FADER_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief
 */
class SatFader : public Object
{
public:
  SatFader ();
  virtual ~SatFader ();
  static TypeId GetTypeId (void);
  virtual double GetChannelGainDb (void) = 0;
  virtual double GetChannelGain (void) = 0;
  virtual void UpdateParameters (uint32_t set, uint32_t state) = 0;

private:
  uint32_t m_setId;   // Set id for this fader
  uint32_t m_stateId; // State id for this fader
};

} // namespace ns3

#endif /* SATELLITE_FADER_H */
