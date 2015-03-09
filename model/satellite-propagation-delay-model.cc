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

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/double.h"
#include "ns3/string.h"

#include "satellite-propagation-delay-model.h"

NS_LOG_COMPONENT_DEFINE ("SatConstantPropagationDelayModel");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatConstantPropagationDelayModel);

TypeId
SatConstantPropagationDelayModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatConstantPropagationDelayModel")
    .SetParent<PropagationDelayModel> ()
    .AddConstructor<SatConstantPropagationDelayModel> ()
    .AddAttribute ("Delay", "The delay",
                   TimeValue ( Seconds (0.13)),
                   MakeTimeAccessor (&SatConstantPropagationDelayModel::m_delay),
                   MakeTimeChecker ())
  ;
  return tid;
}

SatConstantPropagationDelayModel::SatConstantPropagationDelayModel ()
{
}

Time
SatConstantPropagationDelayModel::GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this);
  return m_delay;
}

void
SatConstantPropagationDelayModel::SetDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay.GetSeconds ());
  m_delay = delay;
}

Time
SatConstantPropagationDelayModel::GetDelay () const
{
  NS_LOG_FUNCTION (this);
  return m_delay;
}

int64_t
SatConstantPropagationDelayModel::DoAssignStreams (int64_t s)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

} // namespace ns3
