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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>

#include "satellite-utils.h"
#include "satellite-geo-user-mac.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoUserMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoUserMac);

TypeId
SatGeoUserMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoUserMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatGeoUserMac> ()
  ;
  return tid;
}

TypeId
SatGeoUserMac::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGeoUserMac::SatGeoUserMac (void)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatGeoUserMac default constructor is not allowed to use");
}

SatGeoUserMac::SatGeoUserMac (uint32_t beamId,
                              SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                              SatEnums::RegenerationMode_t returnLinkRegenerationMode)
 : SatMac (beamId)
{
  NS_LOG_FUNCTION (this);

  m_forwardLinkRegenerationMode = forwardLinkRegenerationMode;
  m_returnLinkRegenerationMode = returnLinkRegenerationMode;
}

SatGeoUserMac::~SatGeoUserMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoUserMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
SatGeoUserMac::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
SatGeoUserMac::SendPackets (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> txParams)
{
  NS_LOG_FUNCTION (this);

  // Update MAC address source and destination
  for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
       it != packets.end (); ++it)
    {
      SatMacTag mTag;
      bool success = (*it)->RemovePacketTag (mTag);

      SatAddressE2ETag addressE2ETag;
      success &= (*it)->PeekPacketTag (addressE2ETag);

      // MAC tag and E2E address tag found
      if (success)
        {
          mTag.SetDestAddress (addressE2ETag.GetFinalDestAddress ());
          mTag.SetSourceAddress (m_nodeInfo->GetMacAddress ());
          (*it)->AddPacketTag (mTag);
        }
    }

  // TODO
  m_txUserCallback (txParams);
}

void
SatGeoUserMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this);

  // TODO
  m_rxUserCallback (packets, rxParams);
}

void
SatGeoUserMac::SetTransmitUserCallback (TransmitUserCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_txUserCallback = cb;
}

void
SatGeoUserMac::SetReceiveUserCallback (ReceiveUserCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxUserCallback = cb;
}

void
SatGeoUserMac::RxTraces (SatPhy::PacketContainer_t packets)
{
  NS_LOG_FUNCTION (this);

  if (m_isStatisticsTagsEnabled)
    {
      // TODO

    } // end of `if (m_isStatisticsTagsEnabled)`
}

SatEnums::SatLinkDir_t
SatGeoUserMac::GetSatLinkTxDir ()
{
  return SatEnums::LD_FORWARD;
}

SatEnums::SatLinkDir_t
SatGeoUserMac::GetSatLinkRxDir ()
{
  return SatEnums::LD_RETURN;
}

} // namespace ns3
