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
 *         Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "satellite-rx-cno-input-trace-container.h"

#include "../utils/satellite-env-variables.h"
#include "satellite-id-mapper.h"

#include <ns3/singleton.h>

NS_LOG_COMPONENT_DEFINE("SatRxCnoInputTraceContainer");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatRxCnoInputTraceContainer);

TypeId
SatRxCnoInputTraceContainer::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatRxCnoInputTraceContainer")
                            .SetParent<SatBaseTraceContainer>()
                            .AddConstructor<SatRxCnoInputTraceContainer>();
    return tid;
}

TypeId
SatRxCnoInputTraceContainer::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatRxCnoInputTraceContainer::SatRxCnoInputTraceContainer()
{
    NS_LOG_FUNCTION(this);
}

SatRxCnoInputTraceContainer::~SatRxCnoInputTraceContainer()
{
    NS_LOG_FUNCTION(this);

    Reset();
}

void
SatRxCnoInputTraceContainer::DoDispose()
{
    NS_LOG_FUNCTION(this);

    Reset();

    SatBaseTraceContainer::DoDispose();
}

void
SatRxCnoInputTraceContainer::Reset()
{
    NS_LOG_FUNCTION(this);

    if (!m_container.empty())
    {
        m_container.clear();
    }

    if (!m_containerConstantCno.empty())
    {
        m_containerConstantCno.clear();
    }
}

Ptr<SatInputFileStreamTimeDoubleContainer>
SatRxCnoInputTraceContainer::AddNode(key_t key)
{
    NS_LOG_FUNCTION(this);

    std::stringstream filename;
    std::string dataPath = Singleton<SatEnvVariables>::Get()->LocateDataDirectory();

    int32_t gwId = Singleton<SatIdMapper>::Get()->GetGwIdWithMac(key.first);
    int32_t utId = Singleton<SatIdMapper>::Get()->GetUtIdWithMac(key.first);
    int32_t beamId = Singleton<SatIdMapper>::Get()->GetBeamIdWithMac(key.first);

    if (beamId < 0 || (utId < 0 && gwId < 0))
    {
        return NULL;
    }
    else
    {
        if (utId >= 0 && gwId < 0)
        {
            filename << dataPath << "/rxcnotraces/input/BEAM_" << beamId << "_UT_" << utId
                     << "_channelType_" << SatEnums::GetChannelTypeName(key.second);
        }

        if (gwId >= 0 && utId < 0)
        {
            filename << dataPath << "/rxcnotraces/input/BEAM_" << beamId << "_GW_" << gwId
                     << "_channelType_" << SatEnums::GetChannelTypeName(key.second);
        }

        std::pair<container_t::iterator, bool> result = m_container.insert(
            std::make_pair(key,
                           CreateObject<SatInputFileStreamTimeDoubleContainer>(
                               filename.str().c_str(),
                               std::ios::in,
                               SatBaseTraceContainer::RX_CNO_TRACE_DEFAULT_NUMBER_OF_COLUMNS)));

        if (result.second == false)
        {
            NS_FATAL_ERROR("SatRxCnoInputTraceContainer::AddNode failed");
        }

        NS_LOG_INFO("Added node with MAC " << key.first << " channel type " << key.second);

        return result.first->second;
    }

    NS_FATAL_ERROR("SatRxCnoInputTraceContainer::AddNode failed");
    return NULL;
}

Ptr<SatInputFileStreamTimeDoubleContainer>
SatRxCnoInputTraceContainer::FindNode(key_t key)
{
    NS_LOG_FUNCTION(this);

    container_t::iterator iter = m_container.find(key);

    if (iter == m_container.end())
    {
        return AddNode(key);
    }

    return iter->second;
}

double
SatRxCnoInputTraceContainer::GetRxCno(key_t key)
{
    NS_LOG_FUNCTION(this);

    containerConstantCno_t::iterator iter = m_containerConstantCno.find(key);
    if (iter == m_containerConstantCno.end())
    {
        // No manual value has been set, read it from file
        return FindNode(key)->ProceedToNextClosestTimeSample().at(
            SatBaseTraceContainer::RX_CNO_TRACE_DEFAULT_RX_POWER_DENSITY_INDEX);
    }

    // Otherwise return the manual value stored in the container
    return iter->second;
}

void
SatRxCnoInputTraceContainer::SetRxCno(key_t key, double cno)
{
    NS_LOG_FUNCTION(this << cno);

    containerConstantCno_t::iterator iter = m_containerConstantCno.find(key);
    if (iter != m_containerConstantCno.end())
    {
        // Key already existing, updating value
        iter->second = cno;
    }
    else
    {
        // Add a new key and corresponding value to container
        std::pair<containerConstantCno_t::iterator, bool> result =
            m_containerConstantCno.insert(std::make_pair(key, cno));
        if (result.second == false)
        {
            NS_FATAL_ERROR("SatRxCnoInputTraceContainer::SetRxCno failed");
        }
    }
}

void
SatRxCnoInputTraceContainer::SetRxCnoFile(key_t key, std::string path)
{
    NS_LOG_FUNCTION(this);

    container_t::iterator iter = m_container.find(key);
    if (iter != m_container.end())
    {
        // Key already existing, updating value
        iter->second = CreateObject<SatInputFileStreamTimeDoubleContainer>(
            path,
            std::ios::in,
            SatBaseTraceContainer::RX_CNO_TRACE_DEFAULT_NUMBER_OF_COLUMNS);
    }
    else
    {
        // Add a new key and corresponding value to container
        std::pair<container_t::iterator, bool> result = m_container.insert(
            std::make_pair(key,
                           CreateObject<SatInputFileStreamTimeDoubleContainer>(
                               path,
                               std::ios::in,
                               SatBaseTraceContainer::RX_CNO_TRACE_DEFAULT_NUMBER_OF_COLUMNS)));

        if (result.second == false)
        {
            NS_FATAL_ERROR("SatRxCnoInputTraceContainer::SetRxCnoFile failed");
        }
    }

    // Remove key in m_containerConstantCno
    if (m_containerConstantCno.find(key) != m_containerConstantCno.end())
    {
        // Key already existing, updating value
        m_containerConstantCno.erase(key);
    }
}

} // namespace ns3
