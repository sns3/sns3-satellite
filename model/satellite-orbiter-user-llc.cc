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

#include "satellite-orbiter-user-llc.h"

#include "satellite-generic-stream-encapsulator-arq.h"
#include "satellite-generic-stream-encapsulator.h"
#include "satellite-return-link-encapsulator-arq.h"
#include "satellite-return-link-encapsulator.h"

#include <stdint.h>
#include <utility>

namespace ns3
{
namespace satellite
{

NS_LOG_COMPONENT_DEFINE("SatOrbiterUserLlc");

NS_OBJECT_ENSURE_REGISTERED(SatOrbiterUserLlc);

TypeId
SatOrbiterUserLlc::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatOrbiterUserLlc").SetParent<SatOrbiterLlc>();
    return tid;
}

SatOrbiterUserLlc::SatOrbiterUserLlc()
    : SatOrbiterLlc()
{
    NS_LOG_FUNCTION(this);
}

SatOrbiterUserLlc::~SatOrbiterUserLlc()
{
    NS_LOG_FUNCTION(this);
}

void
SatOrbiterUserLlc::DoDispose()
{
    Object::DoDispose();
}

void
SatOrbiterUserLlc::CreateEncap(Ptr<EncapKey> key)
{
    NS_LOG_FUNCTION(this << key->m_encapAddress << key->m_decapAddress
                         << (uint32_t)(key->m_flowId));

    Ptr<SatBaseEncapsulator> userEncap;

    if (key->m_flowId == 0 || m_forwardLinkRegenerationMode != SatEnums::REGENERATION_NETWORK)
    {
        // Control packet
        userEncap = CreateObject<SatBaseEncapsulator>(key->m_encapAddress,
                                                      key->m_decapAddress,
                                                      key->m_sourceE2EAddress,
                                                      key->m_destE2EAddress,
                                                      key->m_flowId);
    }
    else if (m_fwdLinkArqEnabled)
    {
        userEncap = CreateObject<SatGenericStreamEncapsulatorArq>(key->m_encapAddress,
                                                                  key->m_decapAddress,
                                                                  key->m_sourceE2EAddress,
                                                                  key->m_destE2EAddress,
                                                                  key->m_flowId,
                                                                  m_additionalHeaderSize);
    }
    else
    {
        userEncap = CreateObject<SatGenericStreamEncapsulator>(key->m_encapAddress,
                                                               key->m_decapAddress,
                                                               key->m_sourceE2EAddress,
                                                               key->m_destE2EAddress,
                                                               key->m_flowId,
                                                               m_additionalHeaderSize);
    }

    Ptr<SatQueue> queue = CreateObject<SatQueue>(key->m_flowId);

    userEncap->SetQueue(queue);

    NS_LOG_INFO("Create encapsulator with key (" << key->m_encapAddress << ", "
                                                 << key->m_decapAddress << ", "
                                                 << (uint32_t)key->m_flowId << ")");

    // Store the encapsulator
    std::pair<EncapContainer_t::iterator, bool> result =
        m_encaps.insert(std::make_pair(key, userEncap));
    if (result.second == false)
    {
        NS_FATAL_ERROR("Insert to map with key (" << key->m_encapAddress << ", "
                                                  << key->m_decapAddress << ", "
                                                  << (uint32_t)key->m_flowId << ") failed!");
    }
}

void
SatOrbiterUserLlc::CreateDecap(Ptr<EncapKey> key)
{
    NS_LOG_FUNCTION(this << key->m_encapAddress << key->m_decapAddress
                         << (uint32_t)(key->m_flowId));

    Ptr<SatBaseEncapsulator> userDecap;

    if (key->m_flowId == 0 || m_returnLinkRegenerationMode != SatEnums::REGENERATION_NETWORK)
    {
        // Control packet
        userDecap = CreateObject<SatBaseEncapsulator>(key->m_encapAddress,
                                                      key->m_decapAddress,
                                                      key->m_sourceE2EAddress,
                                                      key->m_destE2EAddress,
                                                      key->m_flowId);
    }
    else if (m_rtnLinkArqEnabled)
    {
        userDecap = CreateObject<SatReturnLinkEncapsulatorArq>(key->m_encapAddress,
                                                               key->m_decapAddress,
                                                               key->m_sourceE2EAddress,
                                                               key->m_destE2EAddress,
                                                               key->m_flowId,
                                                               m_additionalHeaderSize);
    }
    else
    {
        userDecap = CreateObject<SatReturnLinkEncapsulator>(key->m_encapAddress,
                                                            key->m_decapAddress,
                                                            key->m_sourceE2EAddress,
                                                            key->m_destE2EAddress,
                                                            key->m_flowId,
                                                            m_additionalHeaderSize);
    }

    userDecap->SetReceiveCallback(MakeCallback(&SatLlc::ReceiveHigherLayerPdu, this));

    NS_LOG_INFO("Create decapsulator with key (" << key->m_encapAddress << ", "
                                                 << key->m_decapAddress << ", "
                                                 << (uint32_t)key->m_flowId << ")");

    // Store the decapsulator
    std::pair<EncapContainer_t::iterator, bool> result =
        m_decaps.insert(std::make_pair(key, userDecap));
    if (result.second == false)
    {
        NS_FATAL_ERROR("Insert to map with key (" << key->m_encapAddress << ", "
                                                  << key->m_decapAddress << ", "
                                                  << (uint32_t)key->m_flowId << ") failed!");
    }
}

}
} // namespace ns3
