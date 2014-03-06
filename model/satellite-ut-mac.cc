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
 */

#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "ns3/random-variable.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"
#include "ns3/ipv4-l3-protocol.h"
#include "satellite-ut-mac.h"
#include "satellite-enums.h"
#include "satellite-utils.h"
#include "satellite-tbtp-container.h"
#include "satellite-queue.h"
#include "satellite-ut-scheduler.h"
#include "../helper/satellite-wave-form-conf.h"
#include "satellite-crdsa-replica-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatUtMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtMac);

TypeId 
SatUtMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatUtMac> ()
    .AddAttribute ("SuperframeSequence", "Superframe sequence containing information of superframes.",
                    PointerValue (),
                    MakePointerAccessor (&SatUtMac::m_superframeSeq),
                    MakePointerChecker<SatSuperframeSeq> ())
    .AddAttribute ("LowerLayerServiceConf",
                   "Pointer to lower layer service configuration.",
                   PointerValue (),
                   MakePointerAccessor (&SatUtMac::m_llsConf),
                   MakePointerChecker<SatLowerLayerServiceConf> ())
    .AddAttribute ("FramePduHeaderSize",
                   "Frame PDU header size in bytes",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SatUtMac::m_framePduHeaderSizeInBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("GuardTime",
                   "Guard time in return link",
                   TimeValue (MicroSeconds (1)),
                   MakeTimeAccessor (&SatUtMac::m_guardTime),
                   MakeTimeChecker ())
    .AddAttribute ("Scheduler",
                   "UT scheduler used by this Sat UT MAC.",
                   PointerValue (),
                   MakePointerAccessor (&SatUtMac::m_utScheduler),
                   MakePointerChecker<SatUtScheduler> ())
  ;
  return tid;
}

TypeId
SatUtMac::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatUtMac::SatUtMac ()
 : SatMac (),
   m_superframeSeq (),
   m_timingAdvanceCb (0),
   m_llsConf (0),
   m_framePduHeaderSizeInBytes (1),
   m_randomAccess (NULL),
   m_guardTime (MicroSeconds (1)),
   m_raChannel (0)
{
  NS_LOG_FUNCTION (this);

  // default construtctor should not be used
  NS_FATAL_ERROR ("SatUtMac::SatUtMac - Constructor not in use");
}

SatUtMac::SatUtMac (Ptr<SatSuperframeSeq> seq, uint32_t beamId)
 : SatMac (beamId),
   m_superframeSeq (seq),
   m_timingAdvanceCb (0),
   m_llsConf (0),
   m_framePduHeaderSizeInBytes (1),
   m_guardTime (MicroSeconds (1)),
   m_raChannel (0)
{
	NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
  m_tbtpContainer = CreateObject<SatTbtpContainer> ();
}

SatUtMac::~SatUtMac ()
{
  NS_LOG_FUNCTION (this);

  m_randomAccess = NULL;
  m_tbtpContainer = NULL;
}

void
SatUtMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_timingAdvanceCb.Nullify ();
  m_tbtpContainer->DoDispose ();
  m_utScheduler->DoDispose ();
  m_utScheduler = NULL;

  SatMac::DoDispose ();
}

void
SatUtMac::SetNodeInfo (Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);

  m_tbtpContainer->SetMacAddress (nodeInfo->GetMacAddress ());
  m_utScheduler->SetNodeInfo (nodeInfo);
  SatMac::SetNodeInfo (nodeInfo);
}

void
SatUtMac::SetRaChannel (uint32_t raChannel)
{
  NS_LOG_FUNCTION (this << raChannel);

  m_raChannel = raChannel;
}

uint32_t
SatUtMac::GetRaChannel () const
{
  NS_LOG_FUNCTION (this);

  return m_raChannel;
}

void
SatUtMac::SetRandomAccess (Ptr<SatRandomAccess> randomAccess)
{
  NS_LOG_FUNCTION (this);

  m_randomAccess = randomAccess;
  m_randomAccess->SetIsDamaAvailableCallback (MakeCallback(&SatTbtpContainer::HasScheduledTimeSlots, m_tbtpContainer));
}

void
SatUtMac::SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_timingAdvanceCb = cb;
}

Time
SatUtMac::GetSuperFrameTxTime (uint8_t superFrameSeqId) const
{
  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = m_superframeSeq->GetSuperFrameTxTime (superFrameSeqId, timingAdvance);
  return txTime;
}

Time
SatUtMac::GetCurrentSuperFrameStartTime (uint8_t superFrameSeqId) const
{
  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = m_superframeSeq->GetCurrentSuperFrameStartTime (superFrameSeqId, timingAdvance);
  return txTime;
}

void
SatUtMac::ScheduleTimeSlots (Ptr<SatTbtpMessage> tbtp)
{
  NS_LOG_FUNCTION (this << tbtp);
  NS_LOG_LOGIC ("UT: " << m_nodeInfo->GetMacAddress () << " received TBTP " << tbtp->GetSuperframeCounter () << " at time: " << Simulator::Now ().GetSeconds ());

  /**
   * Calculate the sending time of the time slots within this TBTP for this specific UT.
   * UTs may be located at different distances from the satellite, thus they shall have to
   * send the time slots at different times so that the transmissions are received at the GW
   * at correct time.
   */
  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = m_superframeSeq->GetSuperFrameTxTimeWithCount (tbtp->GetSuperframeSeqId (), tbtp->GetSuperframeCounter (), timingAdvance);

  // The delay compared to Now when to start the transmission of this superframe
  Time startDelay = txTime - Simulator::Now ();

  // Add TBTP to a specific container
  m_tbtpContainer->Add (txTime, tbtp);

  // if the calculated start time of the superframe is already in the past
  if (txTime < Simulator::Now ())
    {
      NS_FATAL_ERROR ("UT: " << m_nodeInfo->GetMacAddress () << " received TBTP " << tbtp->GetSuperframeCounter () << ", which should have been sent already in the past");
    }

  // Schedule superframe start
  Simulator::Schedule (startDelay, &SatUtMac::SuperFrameStart, this, tbtp->GetSuperframeSeqId ());

  NS_LOG_LOGIC ("Time to start sending the superframe for this UT: " << txTime.GetSeconds ());
  NS_LOG_LOGIC ("Waiting delay before the superframe start: " << startDelay.GetSeconds ());

  //tbtp->Dump ();

  SatTbtpMessage::DaTimeSlotInfoContainer_t slots = tbtp->GetDaTimeslots (m_nodeInfo->GetMacAddress ());

  if ( !slots.empty ())
    {
      NS_LOG_LOGIC ("TBTP contains " << slots.size () << " timeslots for UT: " << m_nodeInfo->GetMacAddress ());

      uint8_t frameId = 0;

      // schedule time slots
      for ( SatTbtpMessage::DaTimeSlotInfoContainer_t::iterator it = slots.begin (); it != slots.end (); it++ )
        {
          // Store frame id from first slot and check later that frame id is same
          // If frame id changes in TBTP for same UT, raise error.
          if ( it == slots.begin () )
            {
              frameId = it->first;
            }
          else if ( frameId != it->first )
            {
              NS_FATAL_ERROR ("Error in TBTP: slot allocate from different frames for same UT!!!");
            }

          Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
          Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
          Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( it->second );

          // Start time
          Time slotDelay = startDelay + Seconds (timeSlotConf->GetStartTimeInSeconds ());
          NS_LOG_LOGIC ("Slot start delay: " << slotDelay.GetSeconds());

          // Duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
          double duration = wf->GetBurstDurationInSeconds (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

          // Carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

          ScheduleDaTxOpportunity (slotDelay, duration, wf->GetPayloadInBytes (), carrierId);
        }
    }
}

void
SatUtMac::SuperFrameStart (uint8_t superframeSeqId)
{
  NS_LOG_FUNCTION (this << superframeSeqId);
  NS_LOG_LOGIC ("Superframe start time at: " << Simulator::Now ().GetSeconds () << " for UT: " << m_nodeInfo->GetMacAddress ());

  /**
   * Here some functionality may be added for UT related to the superframe start time.
   */
}

void
SatUtMac::ScheduleDaTxOpportunity(Time transmitDelay, double durationInSecs, uint32_t payloadBytes, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << transmitDelay.GetSeconds() << durationInSecs << payloadBytes << carrierId);

  NS_LOG_INFO ("SatUtMac::ScheduleDaTxOpportunity - at time: " << transmitDelay.GetSeconds () << " duration: " << durationInSecs << ", payload: " << payloadBytes << ", carrier: " << carrierId);

  Simulator::Schedule (transmitDelay, &SatUtMac::DoTransmit, this, durationInSecs, payloadBytes, carrierId, -1, SatUtScheduler::LOOSE);
}

void
SatUtMac::DoTransmit (double durationInSecs, uint32_t payloadBytes, uint32_t carrierId, int rcIndex, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << durationInSecs << payloadBytes << carrierId << rcIndex);
  NS_LOG_LOGIC ("Tx opportunity for UT: " << m_nodeInfo->GetMacAddress () << " at time: " << Simulator::Now ().GetSeconds () << ": duration: " << durationInSecs << ", payload: " << payloadBytes << ", carrier: " << carrierId << ", RC index: " << rcIndex);

  TransmitPackets (FetchPackets (payloadBytes, rcIndex, policy), durationInSecs, carrierId);
}

SatPhy::PacketContainer_t
SatUtMac::FetchPackets (uint32_t payloadBytes, int rcIndex, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this);

  /**
   * TODO: the TBTP should hold also the RC_index for each time slot. Here, the RC_index
   * should be passed with txOpportunity to higher layer, so that it knows which RC_index
   * (= queue) to serve.
   */
   
  NS_ASSERT (payloadBytes > m_framePduHeaderSizeInBytes);

  /**
   * The frame PDU header is taken into account as an overhead,
   * thus the payload size of the time slot is reduced by a
   * configured frame PDU header size.
   */
  uint32_t payloadLeft = payloadBytes - m_framePduHeaderSizeInBytes;

  // Packet container to be sent to lower layers.
  // Packet container models FPDU.
  SatPhy::PacketContainer_t packets;

  /**
   * Get new PPDUs from higher layer (LLC) until
   * - The payload is filled to the max OR
   * - The LLC returns NULL packet
   */

  while (payloadLeft > 0)
    {
      NS_LOG_LOGIC ("Tx opportunity: payloadLeft: " << payloadLeft);

      Ptr<Packet> p = m_utScheduler->DoScheduling (payloadLeft, rcIndex, policy);

      // A valid packet received
      if ( p )
        {
          NS_LOG_LOGIC ("Received a PPDU of size: " << p->GetSize ());

          // Add packet trace entry:
          m_packetTrace (Simulator::Now(),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_MAC,
                         SatEnums::LD_RETURN,
                         SatUtils::GetPacketInfo (p));

          packets.push_back (p);
        }
      // LLC returned a NULL packet, break the loop
      else
        {
          break;
        }

      // Update the payloadLeft counter
      if (payloadLeft >= p->GetSize ())
        {
          payloadLeft -= p->GetSize ();
        }
      else
        {
          NS_FATAL_ERROR ("The PPDU was too big for the time slot!");
        }
    }

  NS_ASSERT (payloadLeft >= 0);
  NS_LOG_LOGIC ("The Frame PDU holds " << packets.size () << " packets");
  NS_LOG_LOGIC ("FPDU size:" << payloadBytes - payloadLeft);

  return packets;
}

void
SatUtMac::TransmitPackets (SatPhy::PacketContainer_t packets, double durationInSecs, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this);

  // If there are packets to send
  if (!packets.empty ())
    {
      // Decrease a guard time from time slot duration.
      Time duration (Time::FromDouble(durationInSecs, Time::S) - m_guardTime);
      NS_LOG_LOGIC ("Duration double: " << durationInSecs << " duration time: " << duration.GetSeconds ());
      NS_LOG_LOGIC ("UT: " << m_nodeInfo->GetMacAddress () << " send packet at time: " << Simulator::Now ().GetSeconds () << " duration: " << duration.GetSeconds ());

      SendPacket (packets, carrierId, duration);
    }
}

void
SatUtMac::ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex)
{
  NS_LOG_FUNCTION (this << event << rcIndex);

  NS_LOG_INFO ("SatUtMac::ReceiveQueueEvent - Queue: " << (uint32_t)rcIndex);

  if (rcIndex == 0)
    {
      if (event == SatQueue::FIRST_BUFFERED_PKT || event == SatQueue::BUFFERED_PKT)
        {
          NS_LOG_INFO ("SatUtMac::ReceiveQueueEvent - Buffered packet event received");

          if (m_randomAccess != NULL)
            {
              NS_LOG_INFO ("SatUtMac::ReceiveQueueEvent - Doing Slotted ALOHA");

              DoRandomAccess (SatRandomAccess::RA_SLOTTED_ALOHA_TRIGGER);
            }
        }
    }
}

void
SatUtMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this);

  // Add packet trace entry:
  m_packetTrace (Simulator::Now(),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_MAC,
                 SatEnums::LD_FORWARD,
                 SatUtils::GetPacketInfo (packets));

  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      // Remove packet tag
      SatMacTag macTag;
      bool mSuccess = (*i)->PeekPacketTag (macTag);

      if (!mSuccess)
        {
          NS_FATAL_ERROR ("MAC tag was not found from the packet!");
        }

      NS_LOG_LOGIC("Packet from " << macTag.GetSourceAddress () << " to " << macTag.GetDestAddress ());
      NS_LOG_LOGIC("Receiver " << m_nodeInfo->GetMacAddress ());

      Mac48Address destAddress = Mac48Address::ConvertFrom (macTag.GetDestAddress ());
      if (destAddress == m_nodeInfo->GetMacAddress () || destAddress.IsBroadcast () || destAddress.IsGroup ())
        {
          // Remove control msg tag
          SatControlMsgTag ctrlTag;
          bool cSuccess = (*i)->PeekPacketTag (ctrlTag);

          if (cSuccess)
            {
              SatControlMsgTag::SatControlMsgType_t cType = ctrlTag.GetMsgType ();

              if ( cType != SatControlMsgTag::SAT_NON_CTRL_MSG )
                {
                  // Remove the mac tag
                  (*i)->RemovePacketTag (macTag);
                  ReceiveSignalingPacket (*i, ctrlTag);
                }
              else
                {
                  NS_FATAL_ERROR ("A control message received with not valid msg type!");
                }
            }
          else if (destAddress.IsBroadcast ())
            {
              // TODO: dummy frames and other broadcast needed to handle
              // dummy frames should ignored already in Phy layer
            }
          // Control msg tag not found, send the packet to higher layer
          else
            {
              // Pass the receiver address to LLC
              m_rxCallback (*i, destAddress);
            }
        }
    }
}

void
SatUtMac::ReceiveSignalingPacket (Ptr<Packet> packet, SatControlMsgTag ctrlTag)
{
  NS_LOG_FUNCTION (this);

  switch (ctrlTag.GetMsgType ())
  {
    case SatControlMsgTag::SAT_TBTP_CTRL_MSG:
      {
        uint32_t tbtpId = ctrlTag.GetMsgId ();

        Ptr<SatTbtpMessage> tbtp = DynamicCast<SatTbtpMessage> (m_readCtrlCallback (tbtpId));

        if ( tbtp == NULL )
          {
            NS_FATAL_ERROR ("TBTP not found, check that TBTP storage time is set long enough for superframe sequence!!!");
          }

        ScheduleTimeSlots (tbtp);
        break;
      }
    case SatControlMsgTag::SAT_RA_CTRL_MSG:
    case SatControlMsgTag::SAT_CR_CTRL_MSG:
      {
        NS_FATAL_ERROR ("SatUtMac received a non-supported control packet!");
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("SatUtMac received a non-supported control packet!");
        break;
      }
  }
}

void
SatUtMac::DoRandomAccess (SatRandomAccess::RandomAccessTriggerType_t randomAccessTriggerType)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::DoRandomAccess");

  SatRandomAccess::RandomAccessTxOpportunities_s txOpportunities;

  /// select the RA allocation channel
  uint32_t allocationChannel = GetNextRandomAccessAllocationChannel ();

  /// run random access algorithm
  txOpportunities = m_randomAccess->DoRandomAccess (allocationChannel, randomAccessTriggerType);

  /// process Slotted ALOHA Tx opportunities
  if (txOpportunities.txOpportunityType == SatRandomAccess::RA_SLOTTED_ALOHA_TX_OPPORTUNITY)
    {
      Time txOpportunity = Time::FromInteger (txOpportunities.slottedAlohaTxOpportunity, Time::MS);

      NS_LOG_INFO ("SatUtMac::DoRandomAccess - Processing Slotted ALOHA results, time: " << Now ().GetSeconds () << " seconds, Tx evaluation @: " << (Now () + txOpportunity).GetSeconds () << " seconds");

      /// schedule the check for next available RA slot
      Simulator::Schedule (txOpportunity, &SatUtMac::ScheduleSlottedAlohaTransmission, this, allocationChannel);
    }
  /// process CRDSA Tx opportunities
  else if (txOpportunities.txOpportunityType == SatRandomAccess::RA_CRDSA_TX_OPPORTUNITY)
    {
      NS_LOG_INFO ("SatUtMac::DoRandomAccess - Processing CRDSA results");

      /// schedule CRDSA transmission
      ScheduleCrdsaTransmission (allocationChannel, txOpportunities);
    }
}

uint32_t
SatUtMac::GetNextRandomAccessAllocationChannel ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::GetNextRandomAccessAllocationChannel");

  /// TODO at the moment only allocation channel 0 is supported
  return 0;
}

void
SatUtMac::ScheduleSlottedAlohaTransmission (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission - AC: " << allocationChannel);

  /// check if we have known DAMA allocations
  /// TODO this functionality checks the current and all known future frames for DAMA allocation
  /// it might be better to check only the current frame or a limited subset of frames
  if ( !m_tbtpContainer->HasScheduledTimeSlots () )
    {
      NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission @ " << Now ().GetSeconds () << " - No known DAMA, selecting a slot for Slotted ALOHA");

      Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
      uint8_t frameId = superframeConf->GetRaChannelFrameId (m_raChannel);
      Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
      uint32_t timeSlotCount = frameConf->GetTimeSlotCount ();

      std::pair<bool, uint32_t> result = std::make_pair (false,0);
      uint32_t frameOffset = 0;
      Time superframeStartTime;

      uint32_t superFrameId = m_superframeSeq->GetCurrentSuperFrameCount (0, m_timingAdvanceCb ());

      /// search for the next available slot
      /// if there is no free slots in the current frame, look for it in the following frames
      while (!result.first)
        {
          superframeStartTime = GetCurrentSuperFrameStartTime (0)
              + Seconds (frameOffset * frameConf->GetDurationInSeconds ());

          result = SearchFrameForAvailableSlot (superframeStartTime, frameConf, timeSlotCount, superFrameId, allocationChannel);

          if (!result.first)
            {
              NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission - Increasing frame offset!");
              frameOffset++;
              superFrameId++;
            }
        }

      /// time slot configuration
      Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( result.second );

      /// start time
      Time slotStartTime = superframeStartTime + Seconds (timeSlotConf->GetStartTimeInSeconds ());
      Time offset = slotStartTime - Now ();

      if (offset.IsNegative ())
        {
          NS_FATAL_ERROR ("SatUtMac::ScheduleSlottedAlohaTransmission - Invalid transmit time");
        }

      /// duration
      Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
      double duration = wf->GetBurstDurationInSeconds (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

      /// carrier
      uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

      NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission - Starting to schedule @ " << Now ().GetSeconds () <<
                   ", SF ID: " << superFrameId <<
                   " slot: " << result.second <<
                   " SF start: " << superframeStartTime.GetSeconds () <<
                   " Tx start: " << (Now () + offset).GetSeconds () <<
                   " duration: " << duration <<
                   " carrier ID: " << carrierId <<
                   " payload in bytes: " << wf->GetPayloadInBytes ());

      /// schedule transmission
      /// TODO get rid of the hard coded RC index 0
      Simulator::Schedule (offset, &SatUtMac::DoTransmit, this, duration, wf->GetPayloadInBytes (), carrierId, 0, SatUtScheduler::STRICT);
    }
  else
    {
      NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission @ " << Now ().GetSeconds () << " - UT has known DAMA, aborting Slotted ALOHA");
    }
}

std::pair<bool,uint32_t>
SatUtMac::SearchFrameForAvailableSlot (Time superframeStartTime,
                                       Ptr<SatFrameConf> frameConf,
                                       uint32_t timeSlotCount,
                                       uint32_t superFrameId,
                                       uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  Time opportunityOffset = Now () - superframeStartTime;

  NS_LOG_INFO ("SatUtMac::SearchFrameForAvailableSlot - offset: " << opportunityOffset.GetSeconds ());

  if (opportunityOffset.IsNegative ())
    {
      NS_FATAL_ERROR ("SatUtMac::FindNextAvailableRandomAccessSlot - Invalid Tx opportunity time");
    }

  return FindNextAvailableRandomAccessSlot (opportunityOffset, frameConf, timeSlotCount, superFrameId, allocationChannel);
}

std::pair<bool,uint32_t>
SatUtMac::FindNextAvailableRandomAccessSlot (Time opportunityOffset,
                                             Ptr<SatFrameConf> frameConf,
                                             uint32_t timeSlotCount,
                                             uint32_t superFrameId,
                                             uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::FindNextAvailableRandomAccessSlot");

  Ptr<SatTimeSlotConf> slotConf;
  uint32_t slotId;
  bool availableSlotFound = false;

  /// iterate through slots in this frame
  for (slotId = 0; slotId < timeSlotCount; slotId++)
    {
      slotConf = frameConf->GetTimeSlotConf (slotId);

      //NS_LOG_INFO ("SatUtMac::FindNextAvailableRandomAccessSlot - Slot: " << slotId <<
      //             " slot offset: " << slotConf->GetStartTimeInSeconds () <<
      //             " opportunity offset: " << opportunityOffset.GetSeconds ());

      /// if slot offset is equal or larger than Tx opportunity offset, i.e., the slot is in the future
      if (slotConf->GetStartTimeInSeconds () >= opportunityOffset.GetSeconds ())
        {
          /// if slot is available, set the slot as used and continue with the transmission
          if (UpdateUsedRandomAccessSlots (superFrameId, allocationChannel, slotId))
            {
              availableSlotFound = true;
              break;
            }
        }
    }

  NS_LOG_INFO ("SatUtMac::FindNextAvailableRandomAccessSlot - Success: " << availableSlotFound
                                                            << " SF: " << superFrameId
                                                            << " AC: " << allocationChannel
                                                            << " slot: " << slotId << "/" << timeSlotCount);

  return std::make_pair(availableSlotFound, slotId);
}

void
SatUtMac::ScheduleCrdsaTransmission (uint32_t allocationChannel, SatRandomAccess::RandomAccessTxOpportunities_s txOpportunities)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::ScheduleCrdsaTransmission - AC: " << allocationChannel);

  /// get current superframe ID
  /// TODO get rid of the hard coded 0
  uint32_t superFrameId = m_superframeSeq->GetCurrentSuperFrameCount (0, m_timingAdvanceCb ());

  /// loop through the unique packets
  for (uint32_t i = 0; i < txOpportunities.crdsaTxOpportunities.size (); i++)
    {
      std::set<uint32_t>::iterator iterSet;

      /// loop through the replicas
      for (iterSet = txOpportunities.crdsaTxOpportunities[i].begin(); iterSet != txOpportunities.crdsaTxOpportunities[i].end(); iterSet++)
        {
          /// check and update used slots
          if (!UpdateUsedRandomAccessSlots (superFrameId, allocationChannel, (*iterSet)))
            {
              /// TODO this needs to be handled better
              NS_FATAL_ERROR ("SatUtMac::ScheduleCrdsaTransmission - Slot unavailable");
            }
        }

      /// create replicas and schedule the packets
      CreateCrdsaPacketInstances (allocationChannel, txOpportunities.crdsaTxOpportunities[i]);
    }
}

void
SatUtMac::CreateCrdsaPacketInstances (uint32_t allocationChannel, std::set<uint32_t> slots)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - AC: " << allocationChannel);

  Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
  uint8_t frameId = superframeConf->GetRaChannelFrameId (m_raChannel);
  Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);

  Time superframeStartTime = GetCurrentSuperFrameStartTime (0);

  /// get the slot payload
  uint32_t payloadBytes = superframeConf->GetRaChannelPayloadInBytes (m_raChannel);

  /// get the next packet
  Ptr<Packet> packet = m_utScheduler->DoScheduling (payloadBytes, -1, SatUtScheduler::LOOSE);

  if ( packet )
    {
      std::vector < std::pair< uint16_t,Ptr<Packet> > > replicas;
      std::vector < SatCrdsaReplicaTag > tags;
      std::set<uint32_t>::iterator iterSet;

      /// create replicas
      for (iterSet = slots.begin(); iterSet != slots.end(); iterSet++)
        {
          replicas.push_back (std::make_pair(*iterSet,packet->Copy ()));
        }

      /// create replica slot ID tags
      for (uint32_t i = 0; i < replicas.size (); i++)
        {
          SatCrdsaReplicaTag replicaTag;

          for (uint32_t j = 0; j < replicas.size (); j++)
            {
              if (i != j)
                {
                  replicaTag.AddSlotId (replicas[j].first);
                }
            }
          tags.push_back (replicaTag);
        }

      /// loop through the replicas
      for (uint32_t i = 0; i < replicas.size (); i++)
        {
          /// create packet container
          SatPhy::PacketContainer_t packets;

          /// get packet
          Ptr<Packet> packet = replicas[i].second;

          /// attach the replica tag
          packet->AddPacketTag (tags[i]);

          /// push the replica into the container
          packets.push_back (packet);

          /// time slot configuration
          Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( replicas[i].first );

          /// start time
          Time slotDelay = superframeStartTime + Seconds (timeSlotConf->GetStartTimeInSeconds ());
          Time offset = slotDelay - Now ();

          if (offset.IsNegative ())
            {
              NS_FATAL_ERROR ("SatUtMac::CreateCrdsaPacketInstances - Invalid transmit time");
            }

          /// duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
          double duration = wf->GetBurstDurationInSeconds (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

          /// carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

          /// schedule transmission
          Simulator::Schedule (offset, &SatUtMac::TransmitPackets, this, packets, duration, carrierId);
        }
      replicas.clear ();
      tags.clear ();
    }
}

bool
SatUtMac::UpdateUsedRandomAccessSlots (uint32_t superFrameId, uint32_t allocationChannelId, uint32_t slotId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::UpdateUsedRandomAccessSlots - SF: " << superFrameId << " AC: " << allocationChannelId << " slot: " << slotId);

  std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator iter;
  bool isSlotFree = false;

  /// remove past RA Tx opportunity information
  RemovePastRandomAccessSlots (superFrameId);

  std::pair <uint32_t, uint32_t> key = std::make_pair (superFrameId, allocationChannelId);

  iter = m_usedRandomAccessSlots.find (key);

  if (iter == m_usedRandomAccessSlots.end ())
    {
      std::set<uint32_t> txOpportunities;
      std::pair <std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator, bool> result;

      txOpportunities.insert (slotId);

      result = m_usedRandomAccessSlots.insert (std::make_pair (key, txOpportunities));

      if (result.second)
        {
          isSlotFree = true;
          NS_LOG_INFO ("SatUtMac::UpdateUsedRandomAccessSlots - No saved SF, slot " << slotId << " saved in SF " << superFrameId);
        }
    }
  else
    {
      std::pair<std::set<uint32_t>::iterator,bool> result;
      result = iter->second.insert (slotId);

      if (result.second)
        {
          isSlotFree = true;
          NS_LOG_INFO ("SatUtMac::UpdateUsedRandomAccessSlots - Saved SF exist, slot " << slotId << " saved in SF " << superFrameId);
        }
    }
  return isSlotFree;
}

void
SatUtMac::RemovePastRandomAccessSlots (uint32_t superFrameId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::RemovePastRandomAccessSlots - SF: " << superFrameId);

  //PrintUsedRandomAccessSlots ();

  std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator iter;

  /// TODO this functionality needs to be checked!
  for (iter = m_usedRandomAccessSlots.begin (); iter != m_usedRandomAccessSlots.end (); )
    {
      if (iter->first.first < superFrameId)
        {
          m_usedRandomAccessSlots.erase (iter++);
        }
      else
        {
          ++iter;
        }
    }

  //PrintUsedRandomAccessSlots ();
}

void
SatUtMac::PrintUsedRandomAccessSlots ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::PrintUsedRandomAccessSlots");

  std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator iter;

  for (iter = m_usedRandomAccessSlots.begin (); iter != m_usedRandomAccessSlots.end (); iter++)
    {
      std::set<uint32_t>::iterator iterSet;

      for (iterSet = iter->second.begin (); iterSet != iter->second.end (); iterSet++)
        {
          std::cout << "SF: " << iter->first.first << " AC: " << iter->first.second << " slot: " << *iterSet << std::endl;
        }
    }
}

} // namespace ns3
