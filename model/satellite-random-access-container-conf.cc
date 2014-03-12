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
    ;
  return tid;
}
SatRandomAccessConf::SatRandomAccessConf () :
  slottedAlohaControlRandomizationInterval (),
  m_allocationChannelCount ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatRandomAccessConf::SatRandomAccessConf - Constructor not in use");
}

SatRandomAccessConf::SatRandomAccessConf (Ptr<SatLowerLayerServiceConf> llsConf) :
  slottedAlohaControlRandomizationInterval (),
  m_allocationChannelCount (llsConf->GetRaServiceCount ())
{
  NS_LOG_FUNCTION (this);

  if (m_allocationChannelCount < 1)
    {
      NS_FATAL_ERROR ("SatRandomAccessConf::SatRandomAccessConf - No random access allocation channel");
    }

  slottedAlohaControlRandomizationInterval = (llsConf->GetDefaultControlRandomizationInterval ()).GetMilliSeconds ();
  DoSlottedAlohaVariableSanityCheck ();

  /// TODO this is temporary setup for debugging until all the configurations are available
  for (uint32_t i = 0; i < m_allocationChannelCount; i++)
    {
      Ptr<SatRandomAccessAllocationChannel> allocationChannel = CreateObject<SatRandomAccessAllocationChannel> ();
      m_allocationChannelConf.insert (std::make_pair (i,allocationChannel));

      GetAllocationChannelConfiguration (i)->SetCrdsaMaxUniquePayloadPerBlock (llsConf->GetRaMaximumUniquePayloadPerBlock (i));
      GetAllocationChannelConfiguration (i)->SetCrdsaMaxConsecutiveBlocksAccessed (llsConf->GetRaMaximumConsecutiveBlockAccessed (i));
      GetAllocationChannelConfiguration (i)->SetCrdsaMinIdleBlocks (llsConf->GetRaMinimumIdleBlock (i));

      /// TODO this comes from frame configuration?
      GetAllocationChannelConfiguration (i)->SetCrdsaMinRandomizationValue (0);
      GetAllocationChannelConfiguration (i)->SetCrdsaMaxRandomizationValue (79);

      /// TODO this comes from RA dynamic load control
      GetAllocationChannelConfiguration (i)->SetCrdsaMaximumBackoffProbability (0.2);

      /// TODO from random access traffic method descriptor 6.4.17.26
      GetAllocationChannelConfiguration (i)->SetCrdsaNumOfInstances (2);

      /// TODO these need to be implemented from RA service descriptor
      GetAllocationChannelConfiguration (i)->SetCrdsaBackoffProbability (0.05);
      GetAllocationChannelConfiguration (i)->SetCrdsaBackoffTime (5);

      /// TODO this comes from waveform configuration
      GetAllocationChannelConfiguration (i)->SetCrdsaPayloadBytes (10000);

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

  if (slottedAlohaControlRandomizationInterval < 1)
    {
      NS_FATAL_ERROR ("SatRandomAccessConf::DoSlottedAlohaVariableSanityCheck - slottedAlohaControlRandomizationInterval < 1");
    }

  NS_LOG_INFO ("SatRandomAccessConf::DoSlottedAlohaVariableSanityCheck - Variable sanity check done");
}

} // namespace ns3
