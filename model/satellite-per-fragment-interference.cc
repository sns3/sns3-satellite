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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "satellite-per-fragment-interference.h"

NS_LOG_COMPONENT_DEFINE ("SatPerFragmentInterference");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPerFragmentInterference);

TypeId
SatPerFragmentInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPerFragmentInterference")
    .SetParent<SatPerPacketInterference> ()
    .AddConstructor<SatPerFragmentInterference> ();

  return tid;
}


TypeId
SatPerFragmentInterference::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}


SatPerFragmentInterference::SatPerFragmentInterference ()
  : SatPerPacketInterference (),
  m_ifPowerAtEventChangeW ()
{
  NS_LOG_FUNCTION (this);
}


SatPerFragmentInterference::SatPerFragmentInterference (SatEnums::ChannelType_t channelType, double rxBandwidthHz)
  : SatPerPacketInterference (channelType, rxBandwidthHz),
  m_ifPowerAtEventChangeW ()
{
  NS_LOG_FUNCTION (this);
}


SatPerFragmentInterference::~SatPerFragmentInterference ()
{
  NS_LOG_FUNCTION (this);
}


std::vector< std::pair<double, double> >
SatPerFragmentInterference::DoCalculate (Ptr<SatInterference::InterferenceChangeEvent> event)
{
  NS_LOG_FUNCTION (this);

  m_ifPowerAtEventChangeW.clear ();

  std::vector< std::pair<double, double> > ifPowerPerFragment = SatPerPacketInterference::DoCalculate (event);
  ifPowerPerFragment.clear ();
  ifPowerPerFragment.reserve (m_ifPowerAtEventChangeW.size ());

  std::map<double, double>::const_iterator iter = m_ifPowerAtEventChangeW.begin ();
  std::pair<double, double> eventChangeInPower = *iter;
  for (++iter; iter != m_ifPowerAtEventChangeW.end (); ++iter)
    {
      ifPowerPerFragment.emplace_back (iter->first - eventChangeInPower.first, eventChangeInPower.second);
      eventChangeInPower = *iter;
    }
  ifPowerPerFragment.emplace_back (1.0 - eventChangeInPower.first, eventChangeInPower.second);

  return ifPowerPerFragment;
}


void
SatPerFragmentInterference::onOwnStartReached (double ifPowerW)
{
  m_ifPowerAtEventChangeW[0.0] = ifPowerW;
}


void
SatPerFragmentInterference::onInterferentEvent (long double timeRatio, double interferenceValue, double& ifPowerW)
{
  ifPowerW += interferenceValue;
  m_ifPowerAtEventChangeW[1.0 - timeRatio] = ifPowerW;
}

}
