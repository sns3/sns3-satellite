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

#include "satellite-enums.h"

#include <ns3/address-utils.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/uinteger.h>

#include <map>

NS_LOG_COMPONENT_DEFINE("SatCtrlMessage");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatControlMsgTag);

SatControlMsgTag::SatControlMsgTag()
    : m_msgType(SAT_NON_CTRL_MSG),
      m_msgId(0)
{
    NS_LOG_FUNCTION(this);
}

SatControlMsgTag::~SatControlMsgTag()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SatControlMsgTag::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatControlMsgTag").SetParent<Tag>().AddConstructor<SatControlMsgTag>();
    return tid;
}

TypeId
SatControlMsgTag::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

void
SatControlMsgTag::SetMsgType(SatControlMsgType_t type)
{
    NS_LOG_FUNCTION(this << type);
    m_msgType = type;
}

SatControlMsgTag::SatControlMsgType_t
SatControlMsgTag::GetMsgType(void) const
{
    NS_LOG_FUNCTION(this);
    return m_msgType;
}

uint32_t
SatControlMsgTag::GetSerializedSize(void) const
{
    NS_LOG_FUNCTION(this);

    return (sizeof(m_msgType) + sizeof(m_msgId));
}

void
SatControlMsgTag::Serialize(TagBuffer i) const
{
    NS_LOG_FUNCTION(this << &i);
    i.WriteU32(m_msgType);
    i.WriteU32(m_msgId);
}

void
SatControlMsgTag::Deserialize(TagBuffer i)
{
    NS_LOG_FUNCTION(this << &i);
    m_msgType = (SatControlMsgType_t)i.ReadU32();
    m_msgId = i.ReadU32();
}

void
SatControlMsgTag::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "SatControlMsgType=" << m_msgType << m_msgId;
}

void
SatControlMsgTag::SetMsgId(uint32_t msgId)
{
    NS_LOG_FUNCTION(this << m_msgId);
    m_msgId = msgId;
}

uint32_t
SatControlMsgTag::GetMsgId() const
{
    NS_LOG_FUNCTION(this);

    return m_msgId;
}

// Control message

NS_OBJECT_ENSURE_REGISTERED(SatControlMessage);

TypeId
SatControlMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatControlMessage").SetParent<Object>();
    return tid;
}

// TBTP message

NS_OBJECT_ENSURE_REGISTERED(SatTbtpMessage);

SatTbtpMessage::SatTbtpMessage()
    : m_superframeCounter(0),
      m_superframeSeqId(0),
      m_assignmentFormat(0)
{
    NS_LOG_FUNCTION(this);
}

SatTbtpMessage::SatTbtpMessage(uint8_t seqId)
    : m_superframeCounter(0),
      m_superframeSeqId(seqId),
      m_assignmentFormat(0)
{
    NS_LOG_FUNCTION(this << (uint32_t)seqId);
}

SatTbtpMessage::~SatTbtpMessage()
{
    NS_LOG_FUNCTION(this);

    m_frameIds.clear();
    m_daTimeSlots.clear();
}

TypeId
SatTbtpMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatTbtpMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatTbtpMessage>()
                            .AddAttribute("AssigmentFormat",
                                          "Assignment format of assignment IDs in TBTP.)",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&SatTbtpMessage::m_assignmentFormat),
                                          MakeUintegerChecker<uint8_t>());
    return tid;
}

TypeId
SatTbtpMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

const SatTbtpMessage::DaTimeSlotInfoItem_t&
SatTbtpMessage::GetDaTimeslots(Address utId)
{
    NS_LOG_FUNCTION(this << utId);

    DaTimeSlotMap_t::const_iterator it = m_daTimeSlots.find(utId);

    if (it != m_daTimeSlots.end())
    {
        return it->second;
    }

    return m_emptyDaSlotContainer;
}

void
SatTbtpMessage::SetDaTimeslot(Mac48Address utId, uint8_t frameId, Ptr<SatTimeSlotConf> conf)
{
    NS_LOG_FUNCTION(this << utId << frameId << conf);

    // find container for the UT from map
    DaTimeSlotMap_t::iterator it = m_daTimeSlots.find(utId);

    // If not found, add new UT container to map,
    // otherwise use container found from map
    if (it == m_daTimeSlots.end())
    {
        std::pair<DaTimeSlotMap_t::iterator, bool> result =
            m_daTimeSlots.insert(std::make_pair(utId, DaTimeSlotInfoItem_t()));

        if (result.second)
        {
            it = result.first;
        }
        else
        {
            // container creation for UT has failed, so we need to crash
            NS_FATAL_ERROR("Cannot insert slot to container!!!");
        }
    }

    // store time slot info to user specific container
    it->second.first = frameId;
    it->second.second.push_back(conf);

    // store frame ID to keep track of the used frames count
    m_frameIds.insert(frameId);
}

const SatTbtpMessage::RaChannelInfoContainer_t
SatTbtpMessage::GetRaChannels() const
{
    NS_LOG_FUNCTION(this);

    SatTbtpMessage::RaChannelInfoContainer_t channels;

    for (RaChannelMap_t::const_iterator it = m_raChannels.begin(); it != m_raChannels.end(); it++)
    {
        channels.insert(it->second);
    }

    return channels;
}

void
SatTbtpMessage::SetRaChannel(uint32_t raChannel, uint8_t frameId, uint16_t timeSlotCount)
{
    NS_LOG_FUNCTION(this << raChannel << (uint32_t)frameId << timeSlotCount);

    // find index for the RA channel from map
    RaChannelMap_t::iterator it = m_raChannels.find(raChannel);

    // If not found, add RA channel to map,
    // otherwise raise error
    if (it == m_raChannels.end())
    {
        std::pair<RaChannelMap_t::iterator, bool> result =
            m_raChannels.insert(std::make_pair(raChannel, timeSlotCount));

        if (result.second == false)
        {
            NS_FATAL_ERROR("RA channel insertion failed!!!");
        }
    }
    else
    {
        NS_FATAL_ERROR("RA channel already exists in the container!!!");
    }

    // store frame ID to count used frames
    m_frameIds.insert(frameId);
}

uint32_t
SatTbtpMessage::GetTimeSlotInfoSizeInBytes() const
{
    uint32_t assignmentIdSizeInBytes = 0;

    switch (m_assignmentFormat)
    {
    case 0:
        // assignment id 48 bits
        assignmentIdSizeInBytes = 6;
        break;

    case 1:
        // assignment id 8 bits
        assignmentIdSizeInBytes = 1;
        break;

    case 2:
        // assignment id 16 bits
        assignmentIdSizeInBytes = 2;
        break;

    case 3:
        // assignment id 24 bits
        assignmentIdSizeInBytes = 3;
        break;

    case 10:
        // dynamic tx type 8 bits + assignment id 8 bits
        assignmentIdSizeInBytes = 2;
        break;

    case 11:
        // dynamic tx type 8 bits + assignment id 16 bits
        assignmentIdSizeInBytes = 3;
        break;

    case 12:
        // dynamic tx type 8 bits + assignment id 24 bits
        assignmentIdSizeInBytes = 4;
        break;

    default:
        NS_FATAL_ERROR("Assignment format=" << m_assignmentFormat << " not supported!!!");
        break;
    }

    return assignmentIdSizeInBytes;
}

uint32_t
SatTbtpMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    // see definition for TBTP2 from specification ETSI EN 301 545-2 (V1.1.1), chapter 6.4.9

    uint32_t sizeInBytes = m_tbtpBodySizeInBytes + (m_frameIds.size() * m_tbtpFrameBodySizeInBytes);
    uint32_t assignmentIdSizeInBytes = GetTimeSlotInfoSizeInBytes();

    // add size of DA time slots
    for (DaTimeSlotMap_t::const_iterator it = m_daTimeSlots.begin(); it != m_daTimeSlots.end();
         it++)
    {
        sizeInBytes += (it->second.second.size() * assignmentIdSizeInBytes);
    }

    // add size of RA time slots
    for (RaChannelMap_t::const_iterator it = m_raChannels.begin(); it != m_raChannels.end(); it++)
    {
        sizeInBytes += (it->second * assignmentIdSizeInBytes);
    }

    return sizeInBytes;
}

void
SatTbtpMessage::Dump() const
{
    std::cout << "Superframe counter: " << m_superframeCounter
              << ", superframe sequence id: " << m_superframeSeqId
              << ", assignment format: " << m_assignmentFormat << std::endl;

    for (DaTimeSlotMap_t::const_iterator mit = m_daTimeSlots.begin(); mit != m_daTimeSlots.end();
         ++mit)
    {
        std::cout << "UT: " << mit->first << ": ";
        std::cout << "Frame ID: " << mit->second.first << ": ";
        std::cout << mit->second.second.size() << " ";
        std::cout << std::endl;
    }
}

NS_OBJECT_ENSURE_REGISTERED(SatCrMessage);

TypeId
SatCrMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatCrMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatCrMessage>()
                            .AddAttribute("CrBlockType",
                                          "Capacity request control block size type",
                                          EnumValue(SatCrMessage::CR_BLOCK_SMALL),
                                          MakeEnumAccessor(&SatCrMessage::m_crBlockSizeType),
                                          MakeEnumChecker(SatCrMessage::CR_BLOCK_SMALL,
                                                          "Small",
                                                          SatCrMessage::CR_BLOCK_LARGE,
                                                          "Large"));
    return tid;
}

TypeId
SatCrMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatCrMessage::SatCrMessage()
    : m_crBlockSizeType(SatCrMessage::CR_BLOCK_SMALL),
      m_forwardLinkCNo(NAN)
{
    NS_LOG_FUNCTION(this);
}

SatCrMessage::~SatCrMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatCrMessage::AddControlElement(uint8_t rcIndex,
                                SatEnums::SatCapacityAllocationCategory_t cac,
                                uint32_t value)
{
    NS_LOG_FUNCTION(this << (uint32_t)rcIndex << cac << value);

    RequestDescriptor_t p = std::make_pair(rcIndex, cac);
    m_requestData.insert(std::make_pair(p, value));
}

const SatCrMessage::RequestContainer_t
SatCrMessage::GetCapacityRequestContent() const
{
    return m_requestData;
}

uint32_t
SatCrMessage::GetNumCapacityRequestElements() const
{
    return m_requestData.size();
}

double
SatCrMessage::GetCnoEstimate(void) const
{
    NS_LOG_FUNCTION(this);
    return m_forwardLinkCNo;
}

void
SatCrMessage::SetCnoEstimate(double cno)
{
    NS_LOG_FUNCTION(this << cno);
    m_forwardLinkCNo = cno;
}

uint32_t
SatCrMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    /**
     * CR_BLOCK_SMALL = 2
     * CR_BLOCK_LARGE = 3
     */
    uint32_t crBlockSizeInBytes = (m_crBlockSizeType == SatCrMessage::CR_BLOCK_SMALL ? 2 : 3);

    uint32_t size(CONTROL_MSG_TYPE_VALUE_SIZE_IN_BYTES + CONTROL_MSG_COMMON_HEADER_SIZE_IN_BYTES +
                  m_requestData.size() * crBlockSizeInBytes);
    return size;
}

bool
SatCrMessage::IsNotEmpty() const
{
    return !m_requestData.empty();
}

NS_OBJECT_ENSURE_REGISTERED(SatCnoReportMessage);

TypeId
SatCnoReportMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatCnoReportMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatCnoReportMessage>();
    return tid;
}

TypeId
SatCnoReportMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatCnoReportMessage::SatCnoReportMessage()
    : m_linkCNo(NAN)
{
    NS_LOG_FUNCTION(this);
}

SatCnoReportMessage::~SatCnoReportMessage()
{
    NS_LOG_FUNCTION(this);
}

double
SatCnoReportMessage::GetCnoEstimate(void) const
{
    NS_LOG_FUNCTION(this);
    return m_linkCNo;
}

void
SatCnoReportMessage::SetCnoEstimate(double cno)
{
    NS_LOG_FUNCTION(this << cno);
    m_linkCNo = cno;
}

uint32_t
SatCnoReportMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);
    return sizeof(m_linkCNo);
}

NS_OBJECT_ENSURE_REGISTERED(SatRaMessage);

TypeId
SatRaMessage::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatRaMessage").SetParent<SatControlMessage>().AddConstructor<SatRaMessage>();
    return tid;
}

TypeId
SatRaMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatRaMessage::SatRaMessage()
    : m_allocationChannelId(0),
      m_backoffProbability(0),
      m_backoffTime(0)
{
    NS_LOG_FUNCTION(this);
}

SatRaMessage::~SatRaMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatRaMessage::SetAllocationChannelId(uint8_t allocationChannelId)
{
    NS_LOG_FUNCTION(this << (uint32_t)allocationChannelId);

    m_allocationChannelId = allocationChannelId;
}

uint8_t
SatRaMessage::GetAllocationChannelId() const
{
    return m_allocationChannelId;
}

void
SatRaMessage::SetBackoffProbability(uint16_t backoffProbability)
{
    NS_LOG_FUNCTION(this << backoffProbability);

    m_backoffProbability = backoffProbability;
}

void
SatRaMessage::SetBackoffTime(uint16_t backoffTime)
{
    NS_LOG_FUNCTION(this << backoffTime);

    m_backoffTime = backoffTime;
}

uint16_t
SatRaMessage::GetBackoffProbability() const
{
    return m_backoffProbability;
}

uint16_t
SatRaMessage::GetBackoffTime() const
{
    return m_backoffTime;
}

uint32_t
SatRaMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size(RA_CONTROL_MSG_HEADER_SIZE_IN_BYTES + sizeof(uint8_t) + sizeof(uint16_t) +
                  sizeof(uint16_t));
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatArqAckMessage);

TypeId
SatArqAckMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatArqAckMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatArqAckMessage>();
    return tid;
}

TypeId
SatArqAckMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatArqAckMessage::SatArqAckMessage()
    : m_sequenceNumber(0),
      m_flowId(0)
{
    NS_LOG_FUNCTION(this);
}

SatArqAckMessage::~SatArqAckMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatArqAckMessage::SetSequenceNumber(uint8_t sn)
{
    NS_LOG_FUNCTION(this);
    m_sequenceNumber = sn;
}

uint8_t
SatArqAckMessage::GetSequenceNumber() const
{
    return m_sequenceNumber;
}

void
SatArqAckMessage::SetFlowId(uint8_t flowId)
{
    NS_LOG_FUNCTION(this);
    m_flowId = flowId;
}

uint8_t
SatArqAckMessage::GetFlowId() const
{
    return m_flowId;
}

uint32_t
SatArqAckMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size = 2 * sizeof(uint8_t);
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatTimuMessage);

TypeId
SatTimuMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatTimuMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatTimuMessage>();
    return tid;
}

TypeId
SatTimuMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatTimuMessage::SatTimuMessage()
    : m_beamId(0)
{
    NS_LOG_FUNCTION(this);
}

SatTimuMessage::~SatTimuMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatTimuMessage::SetAllocatedBeamId(uint32_t beamId)
{
    NS_LOG_FUNCTION(this);
    m_beamId = beamId;
}

uint32_t
SatTimuMessage::GetAllocatedBeamId() const
{
    NS_LOG_FUNCTION(this);
    return m_beamId;
}

void
SatTimuMessage::SetSatAddress(Address address)
{
    NS_LOG_FUNCTION(this);
    m_satAddress = address;
}

Address
SatTimuMessage::GetSatAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_satAddress;
}

void
SatTimuMessage::SetGwAddress(Address address)
{
    NS_LOG_FUNCTION(this);
    m_gwAddress = address;
}

Address
SatTimuMessage::GetGwAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_gwAddress;
}

uint32_t
SatTimuMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size = sizeof(uint32_t) + 2*sizeof(Address);
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatHandoverRecommendationMessage);

TypeId
SatHandoverRecommendationMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatHandoverRecommendationMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatHandoverRecommendationMessage>();
    return tid;
}

TypeId
SatHandoverRecommendationMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatHandoverRecommendationMessage::SatHandoverRecommendationMessage()
    : m_beamId(0),
      m_satId(0)
{
    NS_LOG_FUNCTION(this);
}

SatHandoverRecommendationMessage::~SatHandoverRecommendationMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatHandoverRecommendationMessage::SetRecommendedBeamId(uint32_t beamId)
{
    NS_LOG_FUNCTION(this << beamId);
    m_beamId = beamId;
}

uint32_t
SatHandoverRecommendationMessage::GetRecommendedBeamId() const
{
    return m_beamId;
}

void
SatHandoverRecommendationMessage::SetRecommendedSatId(uint32_t satId)
{
    NS_LOG_FUNCTION(this << satId);
    m_satId = satId;
}

uint32_t
SatHandoverRecommendationMessage::GetRecommendedSatId() const
{
    return m_satId;
}

uint32_t
SatHandoverRecommendationMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size = 2 * sizeof(uint32_t);
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatSliceSubscriptionMessage);

TypeId
SatSliceSubscriptionMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatSliceSubscriptionMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatSliceSubscriptionMessage>();
    return tid;
}

TypeId
SatSliceSubscriptionMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatSliceSubscriptionMessage::SatSliceSubscriptionMessage()
    : m_sliceId(0)
{
    NS_LOG_FUNCTION(this);
}

SatSliceSubscriptionMessage::~SatSliceSubscriptionMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatSliceSubscriptionMessage::SetSliceId(uint8_t sliceId)
{
    NS_LOG_FUNCTION(this);
    m_sliceId = sliceId;
}

uint32_t
SatSliceSubscriptionMessage::GetSliceId() const
{
    return m_sliceId;
}

Mac48Address
SatSliceSubscriptionMessage::GetAddress() const
{
    return m_address;
}

void
SatSliceSubscriptionMessage::SetAddress(Mac48Address address)
{
    m_address = address;
}

uint32_t
SatSliceSubscriptionMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size = 1 * sizeof(uint8_t);
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatLogonMessage);

TypeId
SatLogonMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatLogonMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatLogonMessage>();
    return tid;
}

TypeId
SatLogonMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatLogonMessage::SatLogonMessage()
{
    NS_LOG_FUNCTION(this);
}

SatLogonMessage::~SatLogonMessage()
{
    NS_LOG_FUNCTION(this);
}

uint32_t
SatLogonMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size = 1 * sizeof(uint8_t);
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatLogonResponseMessage);

TypeId
SatLogonResponseMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatLogonResponseMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatLogonResponseMessage>();
    return tid;
}

TypeId
SatLogonResponseMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatLogonResponseMessage::SatLogonResponseMessage()
{
    NS_LOG_FUNCTION(this);
}

SatLogonResponseMessage::~SatLogonResponseMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatLogonResponseMessage::SetRaChannel(uint32_t raChannel)
{
    NS_LOG_FUNCTION(this << raChannel);
    m_raChannel = raChannel;
}

uint32_t
SatLogonResponseMessage::GetRaChannel() const
{
    NS_LOG_FUNCTION(this);
    return m_raChannel;
}

uint32_t
SatLogonResponseMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size = 1 * sizeof(uint8_t);
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatLogoffMessage);

TypeId
SatLogoffMessage::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatLogoffMessage")
                            .SetParent<SatControlMessage>()
                            .AddConstructor<SatLogoffMessage>();
    return tid;
}

TypeId
SatLogoffMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatLogoffMessage::SatLogoffMessage()
{
    NS_LOG_FUNCTION(this);
}

SatLogoffMessage::~SatLogoffMessage()
{
    NS_LOG_FUNCTION(this);
}

uint32_t
SatLogoffMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    uint32_t size = 1 * sizeof(uint8_t);
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatNcrMessage);

TypeId
SatNcrMessage::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatNcrMessage").SetParent<SatControlMessage>().AddConstructor<SatNcrMessage>();
    return tid;
}

TypeId
SatNcrMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatNcrMessage::SatNcrMessage()
{
    NS_LOG_FUNCTION(this);
}

SatNcrMessage::~SatNcrMessage()
{
    NS_LOG_FUNCTION(this);
}

void
SatNcrMessage::SetNcrDate(uint64_t ncr)
{
    NS_LOG_FUNCTION(this << ncr);
    m_ncrDateBase = (ncr / 300) % (1UL << 33);
    m_ncrDateExtension = ncr % 300;
}

uint64_t
SatNcrMessage::GetNcrDate() const
{
    NS_LOG_FUNCTION(this);
    return 300 * m_ncrDateBase + m_ncrDateExtension;
}

uint32_t
SatNcrMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    // 33 bits for base, 9 bits for extension, 6 bits reserved
    uint32_t size = 6;
    return size;
}

NS_OBJECT_ENSURE_REGISTERED(SatCmtMessage);

TypeId
SatCmtMessage::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatCmtMessage").SetParent<SatControlMessage>().AddConstructor<SatCmtMessage>();
    return tid;
}

TypeId
SatCmtMessage::GetInstanceTypeId(void) const
{
    NS_LOG_FUNCTION(this);

    return GetTypeId();
}

SatCmtMessage::SatCmtMessage()
    : m_groupId(0),
      m_logonId(0),
      m_burstTimeScaling(0),
      m_burstTimeCorrection(0),
      m_powerCorrection(0),
      m_frequencyCorrection(0)
{
    NS_LOG_FUNCTION(this);
}

SatCmtMessage::~SatCmtMessage()
{
    NS_LOG_FUNCTION(this);
}

uint8_t
SatCmtMessage::GetGroupId() const
{
    return m_groupId;
}

void
SatCmtMessage::SetGroupId(uint8_t groupId)
{
    m_groupId = groupId;
}

uint8_t
SatCmtMessage::GetLogonId() const
{
    return m_logonId;
}

void
SatCmtMessage::SetLogonId(uint8_t logonId)
{
    m_logonId = logonId;
}

int16_t
SatCmtMessage::GetBurstTimeCorrection() const
{
    return m_burstTimeCorrection * (1 << m_burstTimeScaling);
}

void
SatCmtMessage::SetBurstTimeCorrection(int32_t burstTimeCorrection)
{
    if (burstTimeCorrection > 16256)
    {
        NS_FATAL_ERROR("Burst Time Correction too high, should be at most 16256, but got "
                       << burstTimeCorrection);
    }
    if (burstTimeCorrection < -16256)
    {
        NS_FATAL_ERROR("Burst Time Correction too low, should be at least -16256, but got "
                       << burstTimeCorrection);
    }
    m_burstTimeScaling = 0;
    if (burstTimeCorrection > 0)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            if (burstTimeCorrection > 127)
            {
                burstTimeCorrection >>= 1;
                m_burstTimeScaling++;
            }
        }
    }
    else
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            if (-1 * burstTimeCorrection > 127)
            {
                burstTimeCorrection /= 2;
                m_burstTimeScaling++;
            }
        }
    }

    m_burstTimeCorrection = burstTimeCorrection;
}

uint8_t
SatCmtMessage::GetPowerCorrection() const
{
    return m_powerCorrection;
}

void
SatCmtMessage::SetPowerCorrection(uint8_t powerCorrection)
{
    m_powerCorrection = powerCorrection;
}

int16_t
SatCmtMessage::GetFrequencyCorrection() const
{
    return m_frequencyCorrection;
}

void
SatCmtMessage::SetFrequencyCorrection(int16_t frequencyCorrection)
{
    m_frequencyCorrection = frequencyCorrection;
}

uint32_t
SatCmtMessage::GetSizeInBytes() const
{
    NS_LOG_FUNCTION(this);

    // Content of message is
    // m_groupId: 8 bits
    // m_logonId: 16 bits
    // flags (time correction, power correction, frequency correction): 3 bits
    // slot type (always control here): 2 bits
    // m_burstTimeScaling: 3 bits
    // m_burstTimeCorrection: 8 bits
    // power control flag: 1 bit
    // m_powerCorrection: 7 bits
    // m_frequencyCorrection: 16 bits
    // Total is 64 bits = 8 bytes
    uint32_t size = 8;
    return size;
}

// Control message container

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
                if (m_storeTimeout.IsRunning())
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

} // namespace ns3
