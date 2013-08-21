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

#include "satellite-propagation-delay-model.h"
#include "ns3/double.h"
#include "ns3/string.h"

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatConstantPropagationDelayModel);

TypeId
SatConstantPropagationDelayModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatConstantPropagationDelayModel")
    .SetParent<PropagationDelayModel> ()
    .AddConstructor<SatConstantPropagationDelayModel> ()
    .AddAttribute ("Delay", "The delay (s)",
                   DoubleValue (0.13),
                   MakeDoubleAccessor (&SatConstantPropagationDelayModel::m_delay),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

SatConstantPropagationDelayModel::SatConstantPropagationDelayModel ()
{
}

SatConstantPropagationDelayModel::SatConstantPropagationDelayModel ( double delay )
:m_delay (delay)
{

}

Time
SatConstantPropagationDelayModel::GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return Seconds (m_delay);
}

void
SatConstantPropagationDelayModel::SetDelay (double delay)
{
  m_delay = delay;
}

double
SatConstantPropagationDelayModel::GetDelay (void) const
{
  return m_delay;
}

int64_t
SatConstantPropagationDelayModel::DoAssignStreams (int64_t stream)
{
  return 0;
}


} // namespace ns3
