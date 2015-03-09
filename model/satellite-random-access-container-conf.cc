/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
 */
#include "satellite-random-access-container-conf.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("SatRandomAccessConf");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRandomAccessConf);

TypeId
SatRandomAccessConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRandomAccessConf")
    .SetParent<Object> ()
    .AddConstructor<SatRandomAccessConf> ()
    .AddAttribute ("CrdsaSignalingOverheadInBytes", "CRDSA signaling overhead in bytes.",
                   UintegerValue (5),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_crdsaSignalingOverheadInBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SlottedAlohaSignalingOverheadInBytes", "Slotted ALOHA signaling overhead in bytes.",
                   UintegerValue (3),
                   MakeUintegerAccessor (&SatRandomAccessConf::m_slottedAlohaSignalingOverheadInBytes),
                   MakeUintegerChecker<uint32_t> ());
  return tid;
}
SatRandomAccessConf::SatRandomAccessConf ()
  : m_slottedAlohaControlRandomizationIntervalInMilliSeconds (),
    m_allocationChannelCount (),
    m_crdsaSignalingOverheadInBytes (),
    m_slottedAlohaSignalingOverheadInBytes ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatRandomAccessConf::SatRandomAccessConf - Constructor not in use");
}

SatRandomAccessConf::SatRandomAccessConf (Ptr<SatLowerLayerServiceConf> llsConf, Ptr<SatSuperframeSeq> superframeSeq)
  : m_slottedAlohaControlRandomizationIntervalInMilliSeconds (),
    m_allocationChannelCount (llsConf->GetRaServiceCount ()),
    m_crdsaSignalingOverheadInBytes (5),
    m_slottedAlohaSignalingOverheadInBytes (3)
{
  NS_LOG_FUNCTION (this);

  if (m_allocationChannelCount < 1)
    {
      NS_FATAL_ERROR ("SatRandomAccessConf::SatRandomAccessConf - No random access allocation channel");
    }

  m_slottedAlohaControlRandomizationIntervalInMilliSeconds = (llsConf->GetDefaultControlRandomizationInterval ()).GetMilliSeconds ();
  DoSlottedAlohaVariableSanityCheck ();

  for (uint32_t i = 0; i < m_allocationChannelCount; i++)
    {
      Ptr<SatRandomAccessAllocationChannel> allocationChannel = CreateObject<SatRandomAccessAllocationChannel> ();
      m_allocationChannelConf.insert (std::make_pair (i,allocationChannel));

      GetAllocationChannelConfiguration (i)->SetCrdsaMaxUniquePayloadPerBlock (llsConf->GetRaMaximumUniquePayloadPerBlock (i));
      GetAllocationChannelConfiguration (i)->SetCrdsaMaxConsecutiveBlocksAccessed (llsConf->GetRaMaximumConsecutiveBlockAccessed (i));
      GetAllocationChannelConfiguration (i)->SetCrdsaMinIdleBlocks (llsConf->GetRaMinimumIdleBlock (i));
      GetAllocationChannelConfiguration (i)->SetCrdsaNumOfInstances (llsConf->GetRaNumberOfInstances (i));
      GetAllocationChannelConfiguration (i)->SetCrdsaBackoffProbability (llsConf->GetRaBackOffProbability (i));
      GetAllocationChannelConfiguration (i)->SetCrdsaBackoffTimeInMilliSeconds (llsConf->GetRaBackOffTimeInMilliSeconds (i));
      /// this assumes that the slot IDs for each allocation channel start at 0
      GetAllocationChannelConfiguration (i)->SetCrdsaMinRandomizationValue (0);
      /// TODO Get rid of the hard coded 0 in GetSuperframeConf
      /// this assumes that the slot IDs for each allocation channel start at 0
      GetAllocationChannelConfiguration (i)->SetCrdsaMaxRandomizationValue (superframeSeq->GetSuperframeConf (0)->GetRaSlotCount (i) - 1);

      GetAllocationChannelConfiguration (i)->DoCrdsaVariableSanityCheck ();
    }
}

SatRandomAccessConf::~SatRandomAccessConf ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<SatRandomAccessAllocationChannel>
SatRandomAccessConf::GetAllocationChannelConfiguration (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  std::map<uint32_t,Ptr<SatRandomAccessAllocationChannel> >::iterator iter = m_allocationChannelConf.find (allocationChannel);

  if (iter == m_allocationChannelConf.end ())
    {
      NS_FATAL_ERROR ("SatRandomAccessConf::GetAllocationChannelConfiguration - Invalid allocation channel");
    }

  return (iter->second);
}

void
SatRandomAccessConf::DoSlottedAlohaVariableSanityCheck ()
{
  NS_LOG_FUNCTION (this);

  if (m_slottedAlohaControlRandomizationIntervalInMilliSeconds < 1)
    {
      NS_FATAL_ERROR ("SatRandomAccessConf::DoSlottedAlohaVariableSanityCheck - m_slottedAlohaControlRandomizationIntervalInMilliSeconds < 1");
    }

  NS_LOG_INFO ("SatRandomAccessConf::DoSlottedAlohaVariableSanityCheck - Variable sanity check done");
}

} // namespace ns3
