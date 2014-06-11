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
#include "ns3/singleton.h"
#include "satellite-ut-mac.h"
#include "satellite-enums.h"
#include "satellite-utils.h"
#include "satellite-tbtp-container.h"
#include "satellite-queue.h"
#include "satellite-ut-scheduler.h"
#include "satellite-rtn-link-time.h"
#include "satellite-wave-form-conf.h"
#include "satellite-crdsa-replica-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatUtMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtMac);

//#define SAT_CHECK_TBTP_CONTENT

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
    .AddAttribute ("UseCrdsaOnlyForControlPackets",
                   "CRDSA utilized only for control packets or also for user data.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatUtMac::m_crdsaOnlyForControl),
                   MakeBooleanChecker ())
    .AddTraceSource ("DaResourcesTrace",
                     "Assigned dedicated access resources in return link to this UT.",
                     MakeTraceSourceAccessor (&SatUtMac::m_tbtpResourcesTrace))
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
   m_randomAccess (NULL),
   m_guardTime (MicroSeconds (1)),
   m_raChannel (0),
   m_crdsaUniquePacketId (1),
   m_crdsaOnlyForControl (false)
{
  NS_LOG_FUNCTION (this);

  // default constructor should not be used
  NS_FATAL_ERROR ("SatUtMac::SatUtMac - Constructor not in use");
}

SatUtMac::SatUtMac (Ptr<SatSuperframeSeq> seq, uint32_t beamId)
 : SatMac (beamId),
   m_superframeSeq (seq),
   m_timingAdvanceCb (0),
   m_guardTime (MicroSeconds (1)),
   m_raChannel (0),
   m_crdsaUniquePacketId (1),
   m_crdsaOnlyForControl (false)
{
	NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
  m_tbtpContainer = CreateObject<SatTbtpContainer> (m_superframeSeq->GetDuration(m_currentSuperframeSequence));
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

bool
SatUtMac::ControlMsgTransmissionPossible () const
{
  NS_LOG_FUNCTION (this);

  bool da = m_tbtpContainer->HasScheduledTimeSlots ();
  bool ra = (m_randomAccess != NULL);
  return da || ra;
}


void
SatUtMac::SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_timingAdvanceCb = cb;

  /// schedule the next frame start
  Time nextSuperFrameTxTime = GetNextSuperFrameTxTime (m_currentSuperframeSequence);

  if (Now () >= nextSuperFrameTxTime)
    {
      NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
    }

  Time schedulingDelay = nextSuperFrameTxTime - Now ();

  Simulator::Schedule (schedulingDelay, &SatUtMac::DoFrameStart, this);
}

void
SatUtMac::SetAssignedDaResourcesCallback (SatUtMac::AssignedDaResourcesCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_assignedDaResourcesCallback = cb;
}

Time
SatUtMac::GetNextSuperFrameTxTime (uint8_t superFrameSeqId) const
{
  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameTxTime (superFrameSeqId, timingAdvance);
  return txTime;
}

Time
SatUtMac::GetCurrentSuperFrameStartTime (uint8_t superFrameSeqId) const
{
  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = Singleton<SatRtnLinkTime>::Get ()->GetCurrentSuperFrameTxTime (superFrameSeqId, timingAdvance);
  return txTime;
}

void
SatUtMac::ScheduleTimeSlots (Ptr<SatTbtpMessage> tbtp)
{
  NS_LOG_FUNCTION (this << tbtp);
  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " received TBTP " << tbtp->GetSuperframeCounter () << " at time: " << Simulator::Now ().GetSeconds ());

  /**
   * Calculate the sending time of the time slots within this TBTP for this specific UT.
   * UTs may be located at different distances from the satellite, thus they shall have to
   * send the time slots at different times so that the transmissions are received at the GW
   * at correct time.
   */

#ifdef SAT_CHECK_TBTP_CONTENT
  CheckTbtpMessage (tbtp);
#endif

  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = Singleton<SatRtnLinkTime>::Get ()->GetSuperFrameTxTime (tbtp->GetSuperframeSeqId (), tbtp->GetSuperframeCounter (), timingAdvance);

  // The delay compared to Now when to start the transmission of this superframe
  Time startDelay = txTime - Simulator::Now ();

  // Add TBTP to a specific container
  m_tbtpContainer->Add (txTime, tbtp);

  // if the calculated start time of the superframe is already in the past
  if (txTime < Simulator::Now ())
    {
      NS_FATAL_ERROR ("UT: " << m_nodeInfo->GetMacAddress () << " received TBTP " << tbtp->GetSuperframeCounter () << ", which should have been sent already in the past");
    }

  NS_LOG_INFO ("Time to start sending the superframe for this UT: " << txTime.GetSeconds ());
  NS_LOG_INFO ("Waiting delay before the superframe start: " << startDelay.GetSeconds ());

  //tbtp->Dump ();

  SatTbtpMessage::DaTimeSlotInfoContainer_t slots = tbtp->GetDaTimeslots (m_nodeInfo->GetMacAddress ());

  // Counters for allocated TBTP resources
  uint32_t payloadSumInSuperFrame = 0;
  uint32_t payloadSumPerRcIndex [SatEnums::NUM_FIDS] = { };

  if (!slots.empty ())
    {

      NS_LOG_INFO ("TBTP contains " << slots.size () << " timeslots for UT: " << m_nodeInfo->GetMacAddress ());

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
          Ptr<SatTimeSlotConf> timeSlotConf = it->second;

          // Start time
          Time slotDelay = startDelay + timeSlotConf->GetStartTime ();
          NS_LOG_INFO ("Slot start delay: " << slotDelay.GetSeconds());

          // Duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
          Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

          // Carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

          //ScheduleDaTxOpportunity (slotDelay, duration, wf, timeSlotConf->GetSlotType (), timeSlotConf->GetRcIndex (), carrierId);
          ScheduleDaTxOpportunity (slotDelay, duration, wf, timeSlotConf, carrierId);

          payloadSumInSuperFrame += wf->GetPayloadInBytes ();
          payloadSumPerRcIndex [timeSlotConf->GetRcIndex ()] += wf->GetPayloadInBytes ();
        }
    }

  // Assigned TBTP resources
  m_tbtpResourcesTrace (payloadSumInSuperFrame);

  // Update the allocated TBTP resources for each RC index
  for (uint32_t i = 0; i < SatEnums::NUM_FIDS; ++i)
    {
      m_assignedDaResourcesCallback (i, payloadSumPerRcIndex[i]);
    }
}

void
SatUtMac::ScheduleDaTxOpportunity(Time transmitDelay, Time duration, Ptr<SatWaveform> wf, Ptr<SatTimeSlotConf> tsConf, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << transmitDelay.GetSeconds() << duration.GetSeconds () << wf->GetPayloadInBytes () << tsConf->GetRcIndex () << carrierId);
  NS_LOG_LOGIC ("SatUtMac::ScheduleDaTxOpportunity - at time: " << transmitDelay.GetSeconds () << " duration: " << duration.GetSeconds () << ", payload: " << wf->GetPayloadInBytes () << ", rcIndex: " << tsConf->GetRcIndex () << ", carrier: " << carrierId);

  Simulator::Schedule (transmitDelay, &SatUtMac::DoTransmit, this, duration, carrierId, wf, tsConf, SatUtScheduler::LOOSE);
}


void
SatUtMac::DoTransmit (Time duration, uint32_t carrierId, Ptr<SatWaveform> wf, Ptr<SatTimeSlotConf> tsConf, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << wf->GetPayloadInBytes () << carrierId << tsConf->GetRcIndex ());
  NS_LOG_LOGIC ("DA Tx opportunity for UT: " << m_nodeInfo->GetMacAddress () << " at time: " << Simulator::Now ().GetSeconds () << ": duration: " << duration.GetSeconds () << ", payload: " << wf->GetPayloadInBytes () << ", carrier: " << carrierId << ", RC index: " << tsConf->GetRcIndex ());

  SatSignalParameters::txInfo_s txInfo;
  txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
  txInfo.modCod = wf->GetModCod ();
  txInfo.frameType = SatEnums::UNDEFINED_FRAME;
  txInfo.waveformId = wf->GetWaveformId ();

  TransmitPackets (FetchPackets (wf->GetPayloadInBytes (), tsConf->GetSlotType (), tsConf->GetRcIndex (), policy), duration, carrierId, txInfo);
}

void
SatUtMac::DoSlottedAlohaTransmit (Time duration, Ptr<SatWaveform> waveform, uint32_t carrierId, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << waveform->GetPayloadInBytes () << carrierId << rcIndex);
  NS_LOG_INFO ("SatUtMac::DoSlottedAlohaTransmit - Tx opportunity for UT: " << m_nodeInfo->GetMacAddress () << " at time: " << Simulator::Now ().GetSeconds () << ": duration: " << duration.GetSeconds () << ", payload: " << waveform->GetPayloadInBytes () << ", carrier: " << carrierId << ", RC index: " << rcIndex);

  SatPhy::PacketContainer_t packets;

  /// get the slot payload
  uint32_t payloadBytes = waveform->GetPayloadInBytes ();

  /// reduce the CRDSA signaling overhead from the payload
  payloadBytes -= m_randomAccess->GetSlottedAlohaSignalingOverheadInBytes ();

  if (payloadBytes < 1)
    {
      NS_FATAL_ERROR ("SatUtMac::DoSlottedAlohaTransmit - Not enough capacity in Slotted ALOHA payload");
    }

  m_utScheduler->DoScheduling (packets, payloadBytes, SatTimeSlotConf::SLOT_TYPE_C, rcIndex, policy);

  if ( !packets.empty () )
    {
      NS_LOG_LOGIC ("Number of packets sent in a slotted ALOHA slot: " << packets.size ());

      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
          it != packets.end ();
          ++it)
        {
          // Add packet trace entry:
          m_packetTrace (Simulator::Now(),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_MAC,
                         SatEnums::LD_RETURN,
                         SatUtils::GetPacketInfo (*it));
        }

      SatSignalParameters::txInfo_s txInfo;
      txInfo.packetType = SatEnums::PACKET_TYPE_SLOTTED_ALOHA;
      txInfo.modCod = waveform->GetModCod ();
      txInfo.frameType = SatEnums::UNDEFINED_FRAME;
      txInfo.waveformId = waveform->GetWaveformId ();

      TransmitPackets (packets, duration, carrierId, txInfo);
    }
}
SatPhy::PacketContainer_t
SatUtMac::FetchPackets (uint32_t payloadBytes, SatTimeSlotConf::SatTimeSlotType_t type, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this);

  // Packet container to be sent to lower layers.
  // Packet container models FPDU.
  SatPhy::PacketContainer_t packets;

  NS_ASSERT (payloadBytes > 0);

  NS_LOG_LOGIC ("Tx opportunity: payloadBytes: " << payloadBytes);

  m_utScheduler->DoScheduling (packets, payloadBytes, type, rcIndex, policy);

  // A valid packet received
  if ( !packets.empty () )
    {
      NS_LOG_LOGIC ("Number of packets: " << packets.size ());

      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
          it != packets.end ();
          ++it)
        {
          // Add packet trace entry:
          m_packetTrace (Simulator::Now(),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_MAC,
                         SatEnums::LD_RETURN,
                         SatUtils::GetPacketInfo (*it));
        }
    }

  NS_LOG_LOGIC ("The Frame PDU holds " << packets.size () << " packets");

  return packets;
}

void
SatUtMac::TransmitPackets (SatPhy::PacketContainer_t packets, Time duration, uint32_t carrierId, SatSignalParameters::txInfo_s txInfo)
{
  NS_LOG_FUNCTION (this);

  // If there are packets to send
  if (!packets.empty ())
    {
      NS_LOG_INFO ("SatUtMac::TransmitPackets - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << ", transmitting " << packets.size () << " packets, duration: " << duration.GetSeconds () << ", carrier: " << carrierId);

      // Decrease a guard time from time slot duration.
      Time durationWithGuardPeriod (duration - m_guardTime);
      NS_LOG_LOGIC ("Duration: " << duration.GetSeconds () << " duration with guard period: " << duration.GetSeconds ());
      NS_LOG_LOGIC ("UT: " << m_nodeInfo->GetMacAddress () << " send packet at time: " << Simulator::Now ().GetSeconds () << " duration: " << durationWithGuardPeriod.GetSeconds ());

      SendPacket (packets, carrierId, durationWithGuardPeriod, txInfo);
    }
}

void
SatUtMac::ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex)
{
  NS_LOG_FUNCTION (this << event << rcIndex);

  NS_LOG_INFO ("SatUtMac::ReceiveQueueEvent - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " Queue: " << (uint32_t)rcIndex);

  // Check only the queue events from the control queue
  if (rcIndex == SatEnums::CONTROL_FID)
    {
      if (event == SatQueue::FIRST_BUFFERED_PKT || event == SatQueue::BUFFERED_PKT)
        {
          NS_LOG_INFO ("SatUtMac::ReceiveQueueEvent - Buffered packet event received");

          if (m_randomAccess != NULL)
            {
              NS_LOG_INFO ("SatUtMac::ReceiveQueueEvent - Doing Slotted ALOHA");

              DoRandomAccess (SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA);
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

  // Invoke the `Rx` and `RxDelay` trace sources.
  RxTraces (packets);

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

      Mac48Address destAddress = macTag.GetDestAddress ();
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
                  ReceiveSignalingPacket (*i);
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
SatUtMac::ReceiveSignalingPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  // Remove the mac tag
  SatMacTag macTag;
  packet->PeekPacketTag (macTag);

  // Peek control msg tag
  SatControlMsgTag ctrlTag;
  bool cSuccess = packet->PeekPacketTag (ctrlTag);

  if (!cSuccess)
    {
      NS_FATAL_ERROR ("SatControlMsgTag not found in the packet!");
    }

  switch (ctrlTag.GetMsgType ())
  {
    case SatControlMsgTag::SAT_TBTP_CTRL_MSG:
      {
        uint32_t tbtpId = ctrlTag.GetMsgId ();

        Ptr<SatTbtpMessage> tbtp = DynamicCast<SatTbtpMessage> (m_readCtrlCallback (tbtpId));

        if (tbtp == NULL)
          {
            NS_FATAL_ERROR ("TBTP not found, check that control message storage time is set long enough for superframe sequence!!!");
          }

        ScheduleTimeSlots (tbtp);

        packet->RemovePacketTag (macTag);
        packet->RemovePacketTag (ctrlTag);

        break;
      }
    case SatControlMsgTag::SAT_ARQ_ACK:
      {
        // ARQ ACK messages are forwarded to LLC, since they may be fragmented
        Mac48Address destAddress = macTag.GetDestAddress ();
        m_rxCallback (packet, destAddress);
        break;
      }
    case SatControlMsgTag::SAT_RA_CTRL_MSG:
      {
        uint32_t raCtrlId = ctrlTag.GetMsgId();
        Ptr<SatRaMessage> raMsg = DynamicCast<SatRaMessage> (m_readCtrlCallback (raCtrlId));

        if (raMsg == NULL)
          {
            NS_FATAL_ERROR ("Random access control message not found, check that control message storage time is long enough");
          }

        uint32_t allocationChannelId = raMsg->GetAllocationChannelId ();
        uint16_t backoffProbability = raMsg->GetBackoffProbability ();
        uint16_t backoffTime = raMsg->GetBackoffTime ();

        NS_LOG_INFO ("SatUtMac::ReceiveSignalingPacket - UT: " << m_nodeInfo->GetMacAddress () << " @ time: " << Now ().GetSeconds () << " - Updating RA backoff probability for AC: " << allocationChannelId << " to: " << backoffProbability);

        m_randomAccess->SetCrdsaBackoffProbability (allocationChannelId, backoffProbability);
        m_randomAccess->SetCrdsaBackoffTimeInMilliSeconds (allocationChannelId, backoffTime);

        packet->RemovePacketTag (macTag);
        packet->RemovePacketTag (ctrlTag);

        break;
      }
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
SatUtMac::DoRandomAccess (SatEnums::RandomAccessTriggerType_t randomAccessTriggerType)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::DoRandomAccess - UT: " << m_nodeInfo->GetMacAddress () << " @ time: " << Now ().GetSeconds ());

  SatRandomAccess::RandomAccessTxOpportunities_s txOpportunities;

  /// select the RA allocation channel
  uint32_t allocationChannel = GetNextRandomAccessAllocationChannel ();

  /// run random access algorithm
  txOpportunities = m_randomAccess->DoRandomAccess (allocationChannel, randomAccessTriggerType);

  /// process Slotted ALOHA Tx opportunities
  if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_SLOTTED_ALOHA)
    {
      Time txOpportunity = Time::FromInteger (txOpportunities.slottedAlohaTxOpportunity, Time::MS);

      NS_LOG_INFO ("SatUtMac::DoRandomAccess - Processing Slotted ALOHA results, time: " << Now ().GetSeconds () << " seconds, Tx evaluation @: " << (Now () + txOpportunity).GetSeconds () << " seconds");

      /// schedule the check for next available RA slot
      Simulator::Schedule (txOpportunity, &SatUtMac::ScheduleSlottedAlohaTransmission, this, allocationChannel);
    }
  /// process CRDSA Tx opportunities
  else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_CRDSA)
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

  NS_LOG_INFO ("SatUtMac::GetNextRandomAccessAllocationChannel - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds ());

  /// TODO at the moment only one allocation channel is supported
  return m_raChannel;
}

void
SatUtMac::ScheduleSlottedAlohaTransmission (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " AC: " << allocationChannel);

  /// check if we have known DAMA allocations
  /// this functionality checks the current and all known future frames for DAMA allocation
  /// TODO it might be better to check only the current frame or a limited subset of frames
  if ( !m_tbtpContainer->HasScheduledTimeSlots () )
    {
      NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission @ " << Now ().GetSeconds () << " - No known DAMA, selecting a slot for Slotted ALOHA");

      Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
      uint8_t frameId = superframeConf->GetRaChannelFrameId (allocationChannel);
      Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
      uint32_t timeSlotCount = frameConf->GetTimeSlotCount ();

      std::pair<bool, uint32_t> result = std::make_pair (false, 0);
      Time superframeStartTime = GetCurrentSuperFrameStartTime (0);

      if ( Now () < superframeStartTime )
        {
          NS_FATAL_ERROR ("SatUtMac::ScheduleSlottedAlohaTransmission - Invalid SF start time");
        }

      uint32_t superFrameId = Singleton<SatRtnLinkTime>::Get ()->GetCurrentSuperFrameCount (0, m_timingAdvanceCb ());

      NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission - Searching for next available slot");

      /// search for the next available slot
      /// if there is no free slots in the current frame, look for it in the following frames
      while (!result.first)
        {
          NS_LOG_INFO ("Time now: " << Now ().GetSeconds () <<
                       ", superFrameId: " << superFrameId <<
                       ", superframeStartTime: " << superframeStartTime.GetSeconds ());

          result = SearchFrameForAvailableSlot (superframeStartTime, frameConf, timeSlotCount, superFrameId, allocationChannel);

          if (!result.first)
            {
              NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission - Increasing frame offset!");
              superFrameId++;
              superframeStartTime += frameConf->GetDuration ();
            }
        }

      /// time slot configuration
      Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( result.second );

      /// start time
      Time slotStartTime = superframeStartTime + timeSlotConf->GetStartTime ();
      Time offset = slotStartTime - Now ();

      if (offset.IsStrictlyNegative())
        {
          NS_FATAL_ERROR ("SatUtMac::ScheduleSlottedAlohaTransmission - Invalid transmit time: " << offset.GetSeconds ());
        }

      /// duration
      Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (timeSlotConf->GetWaveFormId ());
      Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

      /// carrier
      uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

      NS_LOG_INFO ("SatUtMac::ScheduleSlottedAlohaTransmission - Starting to schedule @ " << Now ().GetSeconds () <<
                   ", SF ID: " << superFrameId <<
                   " slot: " << result.second <<
                   " SF start: " << superframeStartTime.GetSeconds () <<
                   " Tx start: " << (Now () + offset).GetSeconds () <<
                   " duration: " << duration.GetSeconds () <<
                   " carrier ID: " << carrierId <<
                   " payload in bytes: " << wf->GetPayloadInBytes ());

      /// schedule transmission
      Simulator::Schedule (offset, &SatUtMac::DoSlottedAlohaTransmit, this, duration, wf, carrierId, uint8_t(SatEnums::CONTROL_FID), SatUtScheduler::STRICT);
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

  NS_LOG_INFO ("SatUtMac::SearchFrameForAvailableSlot - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " offset: " << opportunityOffset.GetSeconds ());

  /// if we can not use the current superframe, e.g., we are in the the middle of the last slot of the frame, we will use offset 0, e.g., the first slot of the next frame
  if (opportunityOffset.IsStrictlyNegative())
    {
      opportunityOffset = Seconds (0);
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

  NS_LOG_INFO ("SatUtMac::FindNextAvailableRandomAccessSlot - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds ());

  Ptr<SatTimeSlotConf> slotConf;
  uint32_t slotId;
  bool availableSlotFound = false;

  /// iterate through slots in this frame
  for (slotId = 0; slotId < timeSlotCount; slotId++)
    {
      slotConf = frameConf->GetTimeSlotConf (slotId);

      //NS_LOG_INFO ("SatUtMac::FindNextAvailableRandomAccessSlot - Slot: " << slotId <<
      //             " slot offset: " << slotConf->GetStartTime.GetSeconds () <<
      //             " opportunity offset: " << opportunityOffset.GetSeconds ());

      /// if slot offset is equal or larger than Tx opportunity offset, i.e., the slot is in the future
      if (slotConf->GetStartTime () >= opportunityOffset)
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

  /// get current superframe ID
  uint32_t superFrameId = Singleton<SatRtnLinkTime>::Get ()->GetCurrentSuperFrameCount (m_currentSuperframeSequence, m_timingAdvanceCb ());

  NS_LOG_INFO ("SatUtMac::ScheduleCrdsaTransmission - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " AC: " << allocationChannel << ", SF: " << superFrameId << ", num of opportunities: " << txOpportunities.crdsaTxOpportunities.size ());

  std::map<uint32_t,std::set<uint32_t> >::iterator iter;

  /// loop through the unique packets
  for (iter = txOpportunities.crdsaTxOpportunities.begin (); iter != txOpportunities.crdsaTxOpportunities.end (); iter++)
    {
      std::set<uint32_t>::iterator iterSet;

      /// loop through the replicas
      for (iterSet = iter->second.begin (); iterSet != iter->second.end (); iterSet++)
        {
          /// check and update used slots
          if (!UpdateUsedRandomAccessSlots (superFrameId, allocationChannel, (*iterSet)))
            {
              /// TODO this needs to be handled better when multiple allocation channels are in use
              /// In that case a slot exclusion list should be used when randomizing the Tx slots
              NS_FATAL_ERROR ("SatUtMac::ScheduleCrdsaTransmission - Slot unavailable");
            }
        }

      /// create replicas and schedule the packets
      NS_LOG_INFO ("SatUtMac::ScheduleCrdsaTransmission - Creating replicas for packet " << (uint32_t)m_crdsaUniquePacketId);
      CreateCrdsaPacketInstances (allocationChannel, iter->second);
    }
}

void
SatUtMac::CreateCrdsaPacketInstances (uint32_t allocationChannel, std::set<uint32_t> slots)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " AC: " << allocationChannel);

  Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (m_currentSuperframeSequence);
  uint8_t frameId = superframeConf->GetRaChannelFrameId (allocationChannel);
  Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);

  /// CRDSA is evaluated only at the frame start
  /// TODO this has to be updated when a proper mobility model is added
  Time superframeStartTime = Now ();

  /// get the slot payload
  uint32_t payloadBytes = superframeConf->GetRaChannelTimeSlotPayloadInBytes (allocationChannel);

  /// reduce the CRDSA signaling overhead from the payload
  payloadBytes -= m_randomAccess->GetCrdsaSignalingOverheadInBytes ();

  if (payloadBytes < 1)
    {
      NS_FATAL_ERROR ("SatUtMac::CreateCrdsaPacketInstances - Not enough capacity in CRDSA payload");
    }

  /// CRDSA can be utilized to transmit only control messages or
  /// both control and user data
  SatUtScheduler::SatCompliancePolicy_t policy;
  if (m_crdsaOnlyForControl)
    {
      policy = SatUtScheduler::STRICT;
    }
  else
    {
      policy = SatUtScheduler::LOOSE;
    }

  /// get the next packet
  SatPhy::PacketContainer_t uniq;
  m_utScheduler->DoScheduling (uniq, payloadBytes, SatTimeSlotConf::SLOT_TYPE_TRC, uint8_t (SatEnums::CONTROL_FID), policy);

  NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - Processing the packet container, fragments: " << uniq.size ());

  if ( !uniq.empty () )
    {
      std::vector < std::pair< uint16_t, SatPhy::PacketContainer_t > > replicas;
      std::map <uint16_t, SatCrdsaReplicaTag> tags;
      std::set<uint32_t>::iterator iterSet;

      NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - Creating replicas for a packet");

      /// create replicas
      for (iterSet = slots.begin(); iterSet != slots.end(); iterSet++)
        {
          SatPhy::PacketContainer_t rep;
          SatPhy::PacketContainer_t::const_iterator it = uniq.begin ();

          for ( ; it != uniq.end (); ++it)
            {
              rep.push_back ((*it)->Copy ());
              NS_LOG_INFO ("Replica in slot: " << (*iterSet)
                           << ", original (HL packet) fragment UID: " << (*it)->GetUid ()
                           << ", copied replica fragment (HL packet) UID: " << rep.back()->GetUid ());
            }

          NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - One replica created");

          replicas.push_back (std::make_pair(*iterSet, rep));
        }

      NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - Creating replica tags");

      /// create replica slot ID tags
      for (uint32_t i = 0; i < replicas.size (); i++)
        {
          SatCrdsaReplicaTag replicaTag;

          replicaTag.AddSlotId (replicas[i].first);

          NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - Own packet tag: " << replicas[i].first);

          for (uint32_t j = 0; j < replicas.size (); j++)
            {
              if (i != j)
                {
                  replicaTag.AddSlotId (replicas[j].first);

                  NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - Other packet tag: " << replicas[j].first);
                }
            }
          tags.insert (std::make_pair(replicas[i].first,replicaTag));
        }

      NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - Scheduling replicas");

      /// loop through the replicas
      for (uint32_t i = 0; i < replicas.size (); i++)
        {
          for (uint32_t j = 0; j < replicas[i].second.size (); j++)
            {
              NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - replica: " << i
                           << ", fragment: " << j
                           << ", key: " << replicas[i].first
                           << ", tag: " << tags.at (replicas[i].first).GetSlotIds ().at (0)
                           << ", fragment (HL packet) UID: " << replicas[i].second.at (j)->GetUid ());

              /// attach the replica tag
              replicas[i].second.at (j)->AddPacketTag (tags.at (replicas[i].first));
            }

          /// time slot configuration
          Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( replicas[i].first );

          /// start time
          Time slotDelay = superframeStartTime + timeSlotConf->GetStartTime ();
          Time offset = slotDelay - Now ();

          if (offset.IsStrictlyNegative ())
            {
              NS_FATAL_ERROR ("SatUtMac::CreateCrdsaPacketInstances - Invalid transmit time: " << offset.GetSeconds ());
            }

          /// duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
          Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

          /// carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (m_currentSuperframeSequence, frameId, timeSlotConf->GetCarrierId () );

          /// create CRDSA Tx params
          SatSignalParameters::txInfo_s txInfo;
          txInfo.packetType = SatEnums::PACKET_TYPE_CRDSA;
          txInfo.modCod = wf->GetModCod ();
          txInfo.frameType = SatEnums::UNDEFINED_FRAME;
          txInfo.waveformId = wf->GetWaveformId ();
          txInfo.crdsaUniquePacketId = m_crdsaUniquePacketId;

          /// schedule transmission
          Simulator::Schedule (offset, &SatUtMac::TransmitPackets, this, replicas[i].second, duration, carrierId, txInfo);
          NS_LOG_INFO ("SatUtMac::CreateCrdsaPacketInstances - Scheduled a replica in slot " << replicas[i].first << " with offset " << offset.GetSeconds ());
        }
      replicas.clear ();
      tags.clear ();

      m_crdsaUniquePacketId++;
    }
}

bool
SatUtMac::UpdateUsedRandomAccessSlots (uint32_t superFrameId, uint32_t allocationChannelId, uint32_t slotId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::UpdateUsedRandomAccessSlots - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " SF: " << superFrameId << " AC: " << allocationChannelId << " slot: " << slotId);

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

  NS_LOG_INFO ("SatUtMac::RemovePastRandomAccessSlots - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " SF: " << superFrameId);

  //PrintUsedRandomAccessSlots ();

  std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator iter;

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

  NS_LOG_INFO ("SatUtMac::PrintUsedRandomAccessSlots - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds ());

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

void
SatUtMac::DoFrameStart ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatUtMac::DoFrameStart - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds ());

  if (m_randomAccess != NULL)
    {
      /// reset packet ID counter for this frame
      m_crdsaUniquePacketId = 1;

      /// execute CRDSA trigger
      DoRandomAccess (SatEnums::RA_TRIGGER_TYPE_CRDSA);
    }

  Time nextSuperFrameTxTime = GetNextSuperFrameTxTime (0);

  if (Now () >= nextSuperFrameTxTime)
    {
      NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
    }

  Time schedulingDelay = nextSuperFrameTxTime - Now ();

  Simulator::Schedule (schedulingDelay, &SatUtMac::DoFrameStart, this);
}


bool
SatUtMac::CheckTbtpMessage (Ptr<SatTbtpMessage> tbtp) const
{
  NS_LOG_LOGIC (this);
  SatTbtpMessage::DaTimeSlotInfoContainer_t slots = tbtp->GetDaTimeslots (m_nodeInfo->GetMacAddress ());

  Time prevEnd;
  bool first (true);

  if (!slots.empty ())
    {
      NS_LOG_LOGIC ("Number of time slots: " << slots.size ());

      // Ensure that the time slots are in increasing order
      // based on start time
      std::sort (slots.begin (), slots.end (), SortTimeSlots ());

      uint32_t i (0);
      uint8_t frameId (0);
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
          Ptr<SatTimeSlotConf> timeSlotConf = it->second;

          // Start time
          Time startTime = timeSlotConf->GetStartTime ();

          // Carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

          NS_LOG_LOGIC ("Time slot: " << i << " start time: " << startTime.GetSeconds () << " carrier id: " << carrierId);
          if (first != true && prevEnd >= startTime)
            {
              NS_FATAL_ERROR ("Time slots in TBTP are overlapping! Current start time: " << startTime.GetSeconds () << " previous timeslot end time: " << prevEnd.GetSeconds () <<  " carrier id: " << carrierId);
            }

          // Duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
          Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());
          prevEnd = startTime + duration - m_guardTime;

          NS_LOG_LOGIC ("Time slot: " << i << " end time: " << prevEnd.GetSeconds () << " carrier id: " << carrierId);

          first = false;
          ++i;
        }
    }

  return true;
}

} // namespace ns3
