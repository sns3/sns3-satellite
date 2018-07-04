/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions Ltd.
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

#include <cmath>
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include "satellite-bstp-controller.h"

NS_LOG_COMPONENT_DEFINE ("SatBstpController");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBstpController);

SatBstpController::SatBstpController ()
  :m_gwNdCallbacks (),
   m_bhMode (SatBstpController::BH_STATIC),
   m_configFileName ("SatBstpConf.txt")
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  if (m_bhMode == SatBstpController::BH_DYNAMIC)
    {
      NS_FATAL_ERROR ("Beam hopping supports currently only STATIC mode!");
    }

  DoBstpConfiguration ();
}


SatBstpController::~SatBstpController ()
{

}

TypeId
SatBstpController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatBstpController")
    .SetParent<Object> ()
    .AddConstructor<SatBstpController> ()
    .AddAttribute ("BeamHoppingMode",
                   "Beam hopping mode.",
                   EnumValue (SatBstpController::BH_STATIC),
                   MakeEnumAccessor (&SatBstpController::m_bhMode),
                   MakeEnumChecker (SatBstpController::BH_STATIC, "Static",
                                    SatBstpController::BH_DYNAMIC, "Dynamic"))
    .AddAttribute ("StaticBeamHoppingConfigFileName",
                   "Configuration file name for static beam hopping.",
                   StringValue ("SatBstpConf.txt"),
                   MakeStringAccessor (&SatBstpController::m_configFileName),
                   MakeStringChecker ())
  ;
  return tid;
}

TypeId
SatBstpController::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

void SatBstpController::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  for (CallbackContainer_t::iterator it = m_gwNdCallbacks.begin ();
       it != m_gwNdCallbacks.end ();
       ++it)
    {
      it->second.Nullify ();
    }

  Object::DoDispose ();
}

void
SatBstpController::AddNetDeviceCallback (uint32_t beamId,
                                         uint32_t userFreqId,
                                         uint32_t feederFreqId,
                                         uint32_t gwId,
                                         SatBstpController::ToggleCallback cb)
{
  NS_LOG_FUNCTION (this << beamId << userFreqId << feederFreqId << gwId);

  /**
   * userFreqId, feederFreqId and gwId are basically given here
   * just for validity check purposes.
   */

  NS_LOG_INFO ("Add beam: " << beamId << ", userFreqId: " << userFreqId <<
               ", feederFreqId: " << feederFreqId << ", gwId: " << gwId);

  m_gwNdCallbacks.insert (std::make_pair (beamId, cb));
}

void
SatBstpController::DoBstpConfiguration ()
{
  // Read BSTP entry and do configuration
  for (CallbackContainer_t::iterator it = m_gwNdCallbacks.begin ();
       it != m_gwNdCallbacks.end ();
       ++it)
    {
      (*it).second (true);
    }

  // TODO: change the duration of being a multiple of superframe
  // duration
  Time nextConfigurationDuration (MilliSeconds (500));
  Simulator::Schedule (nextConfigurationDuration, &SatBstpController::DoBstpConfiguration, this);
}

}

