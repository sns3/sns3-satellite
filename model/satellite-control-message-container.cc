/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include "satellite-control-message.h"

namespace ns3
{
namespace satellite
{

NS_LOG_COMPONENT_DEFINE("SatControlMsgContainer");

SatControlMsgContainer::SatControlMsgContainer()
    : m_sendId(0),
      m_recvId(0),
      m_storeTime(MilliSeconds(300)),
      m_deleteOnRead(false)
{
    NS_LOG_FUNCTION(this);
}

SatControlMsgContainer::SatControlMsgContainer(Time storeTime, bool deleteOnRead)
    : m_sendId(0),
      m_recvId(0),
      m_storeTime(storeTime),
      m_deleteOnRead(deleteOnRead)

{
    NS_LOG_FUNCTION(this);
}

SatControlMsgContainer::~SatControlMsgContainer()
{
    NS_LOG_FUNCTION(this);
}

uint32_t
SatControlMsgContainer::ReserveIdAndStore(Ptr<SatControlMessage> ctrlMsg)
{
    NS_LOG_FUNCTION(this << ctrlMsg);

    NS_LOG_INFO("Reserve id (send id): " << m_sendId);

    uint32_t id = m_sendId;
    m_sendId++;

    m_reservedCtrlMsgs.insert(std::make_pair(id, ctrlMsg));

    return id;
}

uint32_t
SatControlMsgContainer::Send(uint32_t sendId)
{
    NS_LOG_FUNCTION(this << sendId);

    uint32_t recvId = m_recvId;

    ReservedCtrlMsgMap_t::iterator it = m_reservedCtrlMsgs.find(sendId);

    // Found
    if (it != m_reservedCtrlMsgs.end())
    {
        Time now = Simulator::Now();

        NS_LOG_INFO("Send id: " << sendId << ", recv id: " << m_recvId);

        CtrlMsgMapValue_t mapValue = std::make_pair(now, it->second);
        std::pair<CtrlMsgMap_t::iterator, bool> cResult =
            m_ctrlMsgs.insert(std::make_pair(recvId, mapValue));

        if (cResult.second == false)
        {
            NS_FATAL_ERROR("Control message cannot be added.");
        }

        // Add it to id map for possible future use
        std::pair<CtrlIdMap_t::iterator, bool> idResult =
            m_ctrlIdMap.insert(std::make_pair(sendId, recvId));
        if (idResult.second == false)
        {
            NS_FATAL_ERROR("ID map entry cannot be added!");
        }

        if (m_storeTimeout.IsExpired())
        {
            m_storeTimeout =
                Simulator::Schedule(m_storeTime, &SatControlMsgContainer::EraseFirst, this);
        }

        // Increase the receive id
        ++m_recvId;

        // Erase the entry from the temporary reserved container
        m_reservedCtrlMsgs.erase(it);
    }
    // Not found
    else
    {
        // Try to find it from ID map
        CtrlIdMap_t::iterator idIter = m_ctrlIdMap.find(sendId);
        if (idIter != m_ctrlIdMap.end())
        {
            recvId = idIter->second;
        }
        else
        {
            NS_FATAL_ERROR("The id: "
                           << sendId
                           << " not found from either reserved control messages nor ID map!");
        }
    }

    return recvId;
}

Ptr<SatControlMessage>
SatControlMsgContainer::Read(uint32_t recvId)
{
    NS_LOG_FUNCTION(this << recvId);

    Ptr<SatControlMessage> msg = NULL;

    CtrlMsgMap_t::iterator it = m_ctrlMsgs.find(recvId);

    NS_LOG_INFO("Receive id: " << recvId);

    if (it != m_ctrlMsgs.end())
    {
        msg = it->second.second;

        if (m_deleteOnRead)
        {
            if (it == m_ctrlMsgs.begin())
            {
                if (m_storeTimeout.IsPending())
                {
                    m_storeTimeout.Cancel();
                }

                EraseFirst();
            }
            else
            {
                NS_LOG_INFO("Remove id: " << recvId);
                CleanUpIdMap(recvId);
                m_ctrlMsgs.erase(it);
            }
        }
    }
    else
    {
        NS_FATAL_ERROR("Receive side control message id: "
                       << recvId << " not found from SatControlMsgContainer (m_ctrlMsgs)!");
    }

    return msg;
}

void
SatControlMsgContainer::EraseFirst()
{
    NS_LOG_FUNCTION(this);

    CtrlMsgMap_t::iterator it = m_ctrlMsgs.begin();
    CleanUpIdMap(it->first);
    m_ctrlMsgs.erase(it);

    it = m_ctrlMsgs.begin();

    if (it != m_ctrlMsgs.end())
    {
        Time storedMoment = it->second.first;
        Time elapsedTime = Simulator::Now() - storedMoment;

        m_storeTimeout = Simulator::Schedule(m_storeTime - elapsedTime,
                                             &SatControlMsgContainer::EraseFirst,
                                             this);
    }
}

void
SatControlMsgContainer::CleanUpIdMap(uint32_t recvId)
{
    NS_LOG_FUNCTION(this << recvId);

    CtrlIdMap_t::iterator it = m_ctrlIdMap.begin();
    for (; it != m_ctrlIdMap.end(); ++it)
    {
        if (it->second == recvId)
        {
            m_ctrlIdMap.erase(it);
            break;
        }
    }
}

}
} // namespace ns3
