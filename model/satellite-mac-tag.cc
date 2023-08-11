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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 *         Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "satellite-mac-tag.h"

#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE("SatMacTag");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatMacTag);

SatMacTag::SatMacTag()
{
    NS_LOG_FUNCTION(this);
}

SatMacTag::~SatMacTag()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SatMacTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatMacTag").SetParent<Tag>().AddConstructor<SatMacTag>();
    return tid;
}

TypeId
SatMacTag::GetInstanceTypeId() const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

void
SatMacTag::SetDestAddress(Mac48Address dest)
{
    NS_LOG_FUNCTION(this << dest);
    m_destAddress = dest;
}

Mac48Address
SatMacTag::GetDestAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_destAddress;
}

void
SatMacTag::SetSourceAddress(Mac48Address source)
{
    NS_LOG_FUNCTION(this << source);
    m_sourceAddress = source;
}

Mac48Address
SatMacTag::GetSourceAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_sourceAddress;
}

uint32_t
SatMacTag::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);

    return (2 * ADDRESS_LENGHT);
}

void
SatMacTag::Serialize(TagBuffer i) const
{
    NS_LOG_FUNCTION(this << &i);

    uint8_t buff[ADDRESS_LENGHT];

    m_destAddress.CopyTo(buff);
    i.Write(buff, ADDRESS_LENGHT);

    m_sourceAddress.CopyTo(buff);
    i.Write(buff, ADDRESS_LENGHT);
}

void
SatMacTag::Deserialize(TagBuffer i)
{
    NS_LOG_FUNCTION(this << &i);

    uint8_t buff[ADDRESS_LENGHT];

    i.Read(buff, ADDRESS_LENGHT);
    m_destAddress.CopyFrom(buff);

    i.Read(buff, ADDRESS_LENGHT);
    m_sourceAddress.CopyFrom(buff);
}

void
SatMacTag::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "DestAddress=" << m_destAddress << "SourceAddress" << m_sourceAddress;
}

NS_OBJECT_ENSURE_REGISTERED(SatAddressE2ETag);

SatAddressE2ETag::SatAddressE2ETag()
{
    NS_LOG_FUNCTION(this);
}

SatAddressE2ETag::~SatAddressE2ETag()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SatAddressE2ETag::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SatAddressE2ETag").SetParent<Tag>().AddConstructor<SatAddressE2ETag>();
    return tid;
}

TypeId
SatAddressE2ETag::GetInstanceTypeId() const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

void
SatAddressE2ETag::SetE2EDestAddress(Mac48Address e2eDestAddress)
{
    NS_LOG_FUNCTION(this << e2eDestAddress);
    m_e2eDestAddress = e2eDestAddress;
}

Mac48Address
SatAddressE2ETag::GetE2EDestAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_e2eDestAddress;
}

void
SatAddressE2ETag::SetE2ESourceAddress(Mac48Address e2eSourceAddress)
{
    NS_LOG_FUNCTION(this << e2eSourceAddress);
    m_e2eSourceAddress = e2eSourceAddress;
}

Mac48Address
SatAddressE2ETag::GetE2ESourceAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_e2eSourceAddress;
}

uint32_t
SatAddressE2ETag::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);

    return (2 * ADDRESS_LENGHT);
}

void
SatAddressE2ETag::Serialize(TagBuffer i) const
{
    NS_LOG_FUNCTION(this << &i);

    uint8_t buff[ADDRESS_LENGHT];

    m_e2eDestAddress.CopyTo(buff);
    i.Write(buff, ADDRESS_LENGHT);

    m_e2eSourceAddress.CopyTo(buff);
    i.Write(buff, ADDRESS_LENGHT);
}

void
SatAddressE2ETag::Deserialize(TagBuffer i)
{
    NS_LOG_FUNCTION(this << &i);

    uint8_t buff[ADDRESS_LENGHT];

    i.Read(buff, ADDRESS_LENGHT);
    m_e2eDestAddress.CopyFrom(buff);

    i.Read(buff, ADDRESS_LENGHT);
    m_e2eSourceAddress.CopyFrom(buff);
}

void
SatAddressE2ETag::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "E2EDestAddress=" << m_e2eDestAddress << "E2ESourceAddress" << m_e2eSourceAddress;
}

NS_OBJECT_ENSURE_REGISTERED(SatFlowIdTag);

SatFlowIdTag::SatFlowIdTag()
    : m_flowId(0)
{
    NS_LOG_FUNCTION(this);
}

SatFlowIdTag::~SatFlowIdTag()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SatFlowIdTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SatFlowIdTag").SetParent<Tag>().AddConstructor<SatFlowIdTag>();
    return tid;
}

TypeId
SatFlowIdTag::GetInstanceTypeId() const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

void
SatFlowIdTag::SetFlowId(uint8_t flowId)
{
    NS_LOG_FUNCTION(this << (uint32_t)flowId);
    m_flowId = flowId;
}

uint8_t
SatFlowIdTag::GetFlowId() const
{
    NS_LOG_FUNCTION(this);
    return m_flowId;
}

uint32_t
SatFlowIdTag::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);

    return sizeof(uint8_t);
}

void
SatFlowIdTag::Serialize(TagBuffer i) const
{
    NS_LOG_FUNCTION(this << &i);

    i.WriteU8(m_flowId);
}

void
SatFlowIdTag::Deserialize(TagBuffer i)
{
    NS_LOG_FUNCTION(this << &i);

    m_flowId = i.ReadU8();
}

void
SatFlowIdTag::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "FlowId=" << m_flowId;
}

} // namespace ns3
