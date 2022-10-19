/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#include "satellite-return-link-encapsulator.h"
#include "satellite-return-link-encapsulator-arq.h"
#include "satellite-generic-stream-encapsulator.h"
#include "satellite-generic-stream-encapsulator-arq.h"

#include "satellite-geo-feeder-llc.h"


NS_LOG_COMPONENT_DEFINE ("SatGeoFeederLlc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoFeederLlc);

TypeId
SatGeoFeederLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGeoFeederLlc")
    .SetParent<SatGeoLlc> ()
  ;
  return tid;
}

SatGeoFeederLlc::SatGeoFeederLlc ()
 : SatGeoLlc ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false); // this version of the constructor should not been used
}

SatGeoFeederLlc::SatGeoFeederLlc (SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                  SatEnums::RegenerationMode_t returnLinkRegenerationMode)
 : SatGeoLlc (forwardLinkRegenerationMode, returnLinkRegenerationMode)
{
  NS_LOG_FUNCTION (this);
}

SatGeoFeederLlc::~SatGeoFeederLlc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGeoFeederLlc::DoDispose ()
{
  Object::DoDispose ();
}

void
SatGeoFeederLlc::CreateEncap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_encapAddress << key->m_decapAddress << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> utEncap;

  if (m_rtnLinkArqEnabled)
    {
      utEncap = CreateObject<SatReturnLinkEncapsulatorArq> (key->m_encapAddress,
                                                            key->m_decapAddress,
                                                            key->m_sourceE2EAddress,
                                                            key->m_destE2EAddress,
                                                            key->m_flowId,
                                                            m_additionalHeaderSize);
    }
  else
    {
      utEncap = CreateObject<SatReturnLinkEncapsulator> (key->m_encapAddress,
                                                         key->m_decapAddress,
                                                         key->m_sourceE2EAddress,
                                                         key->m_destE2EAddress,
                                                         key->m_flowId,
                                                         m_additionalHeaderSize);
    }

  Ptr<SatQueue> queue = CreateObject<SatQueue> (key->m_flowId);

  utEncap->SetQueue (queue);

  NS_LOG_INFO ("Create encapsulator with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ")");

  // Store the encapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_encaps.insert (std::make_pair (key, utEncap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}

void
SatGeoFeederLlc::CreateDecap (Ptr<EncapKey> key)
{
  NS_LOG_FUNCTION (this << key->m_encapAddress << key->m_decapAddress << (uint32_t)(key->m_flowId));

  Ptr<SatBaseEncapsulator> userDecap;

  if (m_rtnLinkArqEnabled)
    {
      userDecap = CreateObject<SatGenericStreamEncapsulatorArq> (key->m_encapAddress,
                                                                 key->m_decapAddress,
                                                                 key->m_sourceE2EAddress,
                                                                 key->m_destE2EAddress,
                                                                 key->m_flowId,
                                                                 m_additionalHeaderSize);
    }
  else
    {
      userDecap = CreateObject<SatGenericStreamEncapsulator> (key->m_encapAddress,
                                                              key->m_decapAddress,
                                                              key->m_sourceE2EAddress,
                                                              key->m_destE2EAddress,
                                                              key->m_flowId,
                                                              m_additionalHeaderSize);
    }

  userDecap->SetReceiveCallback (MakeCallback (&SatLlc::ReceiveHigherLayerPdu, this));

  NS_LOG_INFO ("Create decapsulator with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ")");

  // Store the decapsulator
  std::pair<EncapContainer_t::iterator, bool> result = m_decaps.insert (std::make_pair (key, userDecap));
  if (result.second == false)
    {
      NS_FATAL_ERROR ("Insert to map with key (" << key->m_encapAddress << ", " << key->m_decapAddress << ", " << (uint32_t) key->m_flowId << ") failed!");
    }
}

} // namespace ns3


