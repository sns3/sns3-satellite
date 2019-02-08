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

#include <algorithm>

#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include "satellite-bstp-controller.h"
#include "satellite-static-bstp.h"

NS_LOG_COMPONENT_DEFINE ("SatBstpController");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBstpController);

SatBstpController::SatBstpController ()
  :m_gwNdCallbacks (),
   m_bhMode (SatBstpController::BH_STATIC),
   m_configFileName ("SatBstpConf.txt"),
   m_superFrameDuration (MilliSeconds (100)),
   m_staticBstp ()
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  if (m_bhMode == SatBstpController::BH_STATIC)
    {
      m_staticBstp = Create<SatStaticBstp> (m_configFileName);
    }
  else if (m_bhMode == SatBstpController::BH_DYNAMIC)
    {
      NS_FATAL_ERROR ("Beam hopping supports currently only STATIC mode!");
    }
}


SatBstpController::~SatBstpController ()
{
  m_staticBstp = NULL;
}

void
SatBstpController::Initialize ()
{
  NS_LOG_FUNCTION (this);

  if (m_staticBstp)
    {
      m_staticBstp->CheckValidity ();
    }

  DoBstpConfiguration ();
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
                   StringValue ("SatBstpConf_GW1.txt"),
                   MakeStringAccessor (&SatBstpController::m_configFileName),
                   MakeStringChecker ())
    .AddAttribute ("SuperframeDuration",
                   "Superframe duration in Time.",
                   TimeValue (MilliSeconds (10)),
                   MakeTimeAccessor (&SatBstpController::m_superFrameDuration),
                   MakeTimeChecker ())
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

  if (m_staticBstp)
    {
      m_staticBstp->AddEnabledBeamInfo (beamId, userFreqId, feederFreqId, gwId);
    }

  m_gwNdCallbacks.insert (std::make_pair (beamId, cb));
}

void
SatBstpController::DoBstpConfiguration ()
{
  NS_LOG_FUNCTION (this);

  uint32_t validityInSuperframes (1);

  if (m_staticBstp)
    {
      // Read next BSTP configuration
      std::vector<uint32_t> nextConf = m_staticBstp->GetNextConf ();

      // First column is the validity
      validityInSuperframes = nextConf.front ();

      // Read BSTP entry and do configuration
      for (CallbackContainer_t::iterator it = m_gwNdCallbacks.begin ();
           it != m_gwNdCallbacks.end ();
           ++it)
        {
          uint32_t beamId = (*it).first;

          /**
           * Try to find the enabled beam id from the next BSTP configuration!
           * If found, enable it, if not, disable it. Note, search from the second
           * item of the vector, since the first column is the validity!
           */
          if (std::find(nextConf.begin()+1, nextConf.end(), beamId) != nextConf.end())
            {
              (*it).second (true);
            }
          else
            {
              (*it).second (false);
            }
        }
    }
  else
    {
      NS_FATAL_ERROR ("Dynamic beam switching time plan not yet supported!");
    }

  /**
   * Next BSTP configuration time is the validity * superframe duration
   */
  Time nextConfigurationDuration (validityInSuperframes * m_superFrameDuration);
  Simulator::Schedule (nextConfigurationDuration, &SatBstpController::DoBstpConfiguration, this);
}

}
