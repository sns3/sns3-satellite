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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#include <ns3/log.h>
#include <ns3/random-variable-stream.h>
#include <ns3/simulator.h>
#include <ns3/mac48-address.h>
#include <ns3/boolean.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include <ns3/singleton.h>

#include <ns3/satellite-utils.h>
#include <ns3/satellite-tbtp-container.h>
#include <ns3/satellite-rtn-link-time.h>
#include <ns3/satellite-wave-form-conf.h>
#include <ns3/satellite-crdsa-replica-tag.h>
#include <ns3/satellite-superframe-sequence.h>
#include <ns3/satellite-control-message.h>
#include <ns3/satellite-frame-conf.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-log.h>
#include <ns3/satellite-encap-pdu-status-tag.h>
#include "satellite-ut-mac.h"

NS_LOG_COMPONENT_DEFINE ("SatUtMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtMac);


TypeId
SatUtMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtMac")
    .SetParent<SatMac> ()
    .AddConstructor<SatUtMac> ()
    .AddAttribute ("MaxHandoverMessages", "Max amount of handover messages to send without a handover response before loging off",
                   UintegerValue (20),
                   MakeUintegerAccessor (&SatUtMac::m_maxHandoverMessagesSent),
                   MakeUintegerChecker<uint32_t> ())
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
    .AddAttribute ("WindowInitLogon",
                   "The initial window for logon waiting time before transmission.",
                   TimeValue (Seconds (20)),
                   MakeTimeAccessor (&SatUtMac::m_windowInitLogon),
                   MakeTimeChecker ())
    .AddAttribute ("MaxWaitingTimeLogonResponse",
                   "Timeout for waiting for a response for a logon message.",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&SatUtMac::m_maxWaitingTimeLogonResponse),
                   MakeTimeChecker ())
    .AddAttribute ("ClockDrift",
                   "Clock drift (number of ticks per second).",
                   IntegerValue (0),
                   MakeIntegerAccessor (&SatUtMac::m_clockDrift),
                   MakeIntegerChecker<int32_t> ())
    .AddTraceSource ("DaResourcesTrace",
                     "Assigned dedicated access resources in return link to this UT.",
                     MakeTraceSourceAccessor (&SatUtMac::m_tbtpResourcesTrace),
                     "ns3::SatUtMac::TbtpResourcesTraceCallback")
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
  m_logonChannel (0),
  m_loggedOn (true),
  m_useLogon (false),
  m_sendLogonTries (0),
  m_windowInitLogon (Seconds (20)),
  m_maxWaitingTimeLogonResponse (Seconds (1)),
  m_waitingTimeLogonRng (CreateObject<UniformRandomVariable> ()),
  m_nextLogonTransmissionPossible (Seconds (0)),
  m_crdsaUniquePacketId (1),
  m_crdsaOnlyForControl (false),
  m_nextPacketTime (Now ()),
  m_isRandomAccessScheduled (false),
  m_timuInfo (0),
  m_rcstState (SatUtMacState ()),
  m_lastNcrDateReceived (Seconds (0)),
  m_ncr (0),
  m_deltaNcr (0),
  m_clockDrift (0),
  m_handoverState (NO_HANDOVER),
  m_handoverMessagesCount (0),
  m_maxHandoverMessagesSent (20),
  m_firstTransmittableSuperframeId (0),
  m_handoverCallback (0),
  m_gatewayUpdateCallback (0),
  m_routingUpdateCallback (0),
  m_beamCheckerCallback (0),
  m_askedBeamCallback (0),
  m_txCheckCallback (0),
  m_sliceSubscriptionCallback (0),
  m_sendLogonCallback (0),
  m_updateGwAddressCallback (0),
  m_beamScheculerCallback (0)
{
  NS_LOG_FUNCTION (this);

  // default constructor should not be used
  NS_FATAL_ERROR ("SatUtMac::SatUtMac - Constructor not in use");
}

SatUtMac::SatUtMac (Ptr<SatSuperframeSeq> seq, uint32_t beamId, bool crdsaOnlyForControl)
  : SatMac (beamId),
  m_superframeSeq (seq),
  m_timingAdvanceCb (0),
  m_guardTime (MicroSeconds (1)),
  m_raChannel (0),
  m_logonChannel (0),
  m_loggedOn (true),
  m_useLogon (false),
  m_sendLogonTries (0),
  m_windowInitLogon (Seconds (20)),
  m_maxWaitingTimeLogonResponse (Seconds (1)),
  m_waitingTimeLogonRng (CreateObject<UniformRandomVariable> ()),
  m_nextLogonTransmissionPossible (Seconds (0)),
  m_crdsaUniquePacketId (1),
  m_crdsaOnlyForControl (crdsaOnlyForControl),
  m_nextPacketTime (Now ()),
  m_isRandomAccessScheduled (false),
  m_timuInfo (0),
  m_rcstState (SatUtMacState ()),
  m_lastNcrDateReceived (Seconds (0)),
  m_ncr (0),
  m_deltaNcr (0),
  m_clockDrift (0),
  m_handoverState (NO_HANDOVER),
  m_handoverMessagesCount (0),
  m_maxHandoverMessagesSent (20),
  m_firstTransmittableSuperframeId (0),
  m_handoverCallback (0),
  m_gatewayUpdateCallback (0),
  m_routingUpdateCallback (0),
  m_beamCheckerCallback (0),
  m_askedBeamCallback (0),
  m_txCheckCallback (0),
  m_sliceSubscriptionCallback (0),
  m_sendLogonCallback (0),
  m_updateGwAddressCallback (0),
  m_beamScheculerCallback (0)
{
  NS_LOG_FUNCTION (this);

  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
  m_tbtpContainer = CreateObject<SatTbtpContainer> (m_superframeSeq);
}

SatUtMac::~SatUtMac ()
{
  NS_LOG_FUNCTION (this);

  m_randomAccess = NULL;
  m_tbtpContainer = NULL;
  m_timuInfo = NULL;
}

void
SatUtMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_timingAdvanceCb.Nullify ();
  m_handoverCallback.Nullify ();
  m_gatewayUpdateCallback.Nullify ();
  m_routingUpdateCallback.Nullify ();
  m_beamCheckerCallback.Nullify ();
  m_askedBeamCallback.Nullify ();
  m_txCheckCallback.Nullify ();
  m_sliceSubscriptionCallback.Nullify ();
  m_sendLogonCallback.Nullify ();
  m_updateGwAddressCallback.Nullify ();
  m_beamScheculerCallback.Nullify ();
  m_tbtpContainer->DoDispose ();
  m_utScheduler->DoDispose ();
  m_utScheduler = NULL;

  SatMac::DoDispose ();
}

void
SatUtMac::SetHandoverCallback (SatUtMac::HandoverCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_handoverCallback = cb;
}

void
SatUtMac::SetGatewayUpdateCallback (SatUtMac::GatewayUpdateCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_gatewayUpdateCallback = cb;
}

void
SatUtMac::SetRoutingUpdateCallback (SatUtMac::RoutingUpdateCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_routingUpdateCallback = cb;
}

void
SatUtMac::SetBeamCheckerCallback (SatUtMac::BeamCheckerCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_beamCheckerCallback = cb;
}

void
SatUtMac::SetAskedBeamCallback (SatUtMac::AskedBeamCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_askedBeamCallback = cb;
}

void
SatUtMac::SetTxCheckCallback (SatUtMac::TxCheckCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_txCheckCallback = cb;
}

void
SatUtMac::SetSliceSubscriptionCallback (SatUtMac::SliceSubscriptionCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_sliceSubscriptionCallback = cb;
}

void
SatUtMac::SetSendLogonCallback (SatUtMac::SendLogonCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_sendLogonCallback = cb;
}

void
SatUtMac::SetUpdateGwAddressCallback (SatUtMac::UpdateGwAddressCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_updateGwAddressCallback = cb;
}

void
SatUtMac::SetBeamScheculerCallback (SatUtMac::BeamScheculerCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_beamScheculerCallback = cb;
}

void
SatUtMac::SetGwAddress (Mac48Address gwAddress)
{
  NS_LOG_FUNCTION (this << gwAddress);

  m_gatewayUpdateCallback (gwAddress);
  m_gwAddress = gwAddress;
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
SatUtMac::LogOff ()
{
  NS_LOG_FUNCTION (this);
  m_loggedOn = false;
  m_raChannel = m_logonChannel;
  m_rcstState.SwitchToOffStandby ();
  std::cout << Simulator::Now () << " LOGOFF" << std::endl;
}

void
SatUtMac::SetLogonChannel (uint32_t channelId)
{
  NS_LOG_FUNCTION (this);
  m_logonChannel = channelId;
  m_useLogon = true;
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
  m_randomAccess->SetIsDamaAvailableCallback (MakeCallback (&SatTbtpContainer::HasScheduledTimeSlots, m_tbtpContainer));
}

bool
SatUtMac::ControlMsgTransmissionPossible () const
{
  NS_LOG_FUNCTION (this);

  bool da = m_tbtpContainer->HasScheduledTimeSlots ();
  bool ra = (m_randomAccess != NULL);
  if (m_useLogon)
    {
      ra = ra && (m_raChannel != m_logonChannel);
    }
  return da || ra;
}

bool
SatUtMac::LogonMsgTransmissionPossible () const
{
  NS_LOG_FUNCTION (this);

  bool stateCorrect = (m_rcstState.GetState () == SatUtMacState::RcstState_t::READY_FOR_LOGON);
  bool ncrReceived = !m_rcstState.IsNcrTimeout ();

  return m_useLogon && stateCorrect && ncrReceived;
}

void
SatUtMac::SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_timingAdvanceCb = cb;

  /// schedule the next frame start
  Time nextSuperFrameTxTime = GetNextSuperFrameTxTime (SatConstVariables::SUPERFRAME_SEQUENCE);

  if (Now () >= nextSuperFrameTxTime)
    {
      NS_FATAL_ERROR ("Scheduling next superframe start time to the past!");
    }

  Time schedulingDelay = nextSuperFrameTxTime - Now ();

  Simulator::Schedule (GetRealSendingTime (schedulingDelay), &SatUtMac::DoFrameStart, this);
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
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId);

  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameTxTime (superFrameSeqId, timingAdvance);
  return txTime;
}

Time
SatUtMac::GetCurrentSuperFrameStartTime (uint8_t superFrameSeqId) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId);

  Time timingAdvance = m_timingAdvanceCb ();
  Time txTime = Singleton<SatRtnLinkTime>::Get ()->GetCurrentSuperFrameTxTime (superFrameSeqId, timingAdvance);
  return txTime;
}

uint32_t
SatUtMac::GetCurrentSuperFrameId (uint8_t superFrameSeqId) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId);

  return Singleton<SatRtnLinkTime>::Get ()->GetCurrentSuperFrameCount (superFrameSeqId, m_timingAdvanceCb ());
}

void
SatUtMac::ScheduleTimeSlots (Ptr<SatTbtpMessage> tbtp)
{
  NS_LOG_FUNCTION (this << tbtp);
  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " received TBTP " << tbtp->GetSuperframeCounter ());

  /**
   * Calculate the sending time of the time slots within this TBTP for this specific UT.
   * UTs may be located at different distances from the satellite, thus they shall have to
   * send the time slots at different times so that the transmissions are received at the GW
   * at correct time.
   */

  /// If using asynchronous access (no timeslots), return
  /// TODO find better way of doing this
  if (m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetConfigType () == SatSuperframeConf::CONFIG_TYPE_4)
    {
      return;
    }

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

  SatTbtpMessage::DaTimeSlotInfoItem_t info = tbtp->GetDaTimeslots (m_nodeInfo->GetMacAddress ());

  // Counters for allocated TBTP resources
  uint32_t payloadSumInSuperFrame = 0;
  uint32_t payloadSumPerRcIndex [SatEnums::NUM_FIDS] = { };

  if (!info.second.empty ())
    {
      NS_LOG_INFO ("TBTP contains " << info.second.size () << " timeslots for UT: " << m_nodeInfo->GetMacAddress ());

      uint8_t frameId = info.first;

      // schedule time slots
      for ( SatTbtpMessage::DaTimeSlotConfContainer_t::iterator it = info.second.begin (); it != info.second.end (); it++ )
        {
          Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
          Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
          Ptr<SatTimeSlotConf> timeSlotConf = *it;

          // Start time
          Time slotDelay = startDelay + timeSlotConf->GetStartTime ();
          NS_LOG_INFO ("Slot start delay: " << slotDelay.GetSeconds ());

          // Duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (timeSlotConf->GetWaveFormId ());
          Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

          bool drop = false;
          if (timeSlotConf->GetSlotType () == SatTimeSlotConf::SLOT_TYPE_C)
            {
              // TODO add new tag to specify control ?
              wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (2);
              duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());
              if (m_useLogon && m_rcstState.GetState () != SatUtMacState::RcstState_t::TDMA_SYNC && m_rcstState.GetState () != SatUtMacState::RcstState_t::READY_FOR_TDMA_SYNC)
                {
                  drop = true;
                }
              //std::cout << "Sending control burst " << timeSlotConf->GetStartTime () << " after SF start, drop=" << drop << ". slotDelay = " << slotDelay << std::endl;
            }

          // Carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

          if (!drop)
            {
              // Schedule individual time slot
              ScheduleDaTxOpportunity (slotDelay, duration, wf, timeSlotConf, carrierId);

              payloadSumInSuperFrame += wf->GetPayloadInBytes ();
              payloadSumPerRcIndex [timeSlotConf->GetRcIndex ()] += wf->GetPayloadInBytes ();
            }
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
SatUtMac::ScheduleDaTxOpportunity (Time transmitDelay, Time duration, Ptr<SatWaveform> wf, Ptr<SatTimeSlotConf> tsConf, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << transmitDelay.GetSeconds () << duration.GetSeconds () << wf->GetPayloadInBytes () << (uint32_t)(tsConf->GetRcIndex ()) << carrierId);
  NS_LOG_INFO ("After delay: " << transmitDelay.GetSeconds () <<
               " duration: " << duration.GetSeconds () <<
               ", payload: " << wf->GetPayloadInBytes () <<
               ", rcIndex: " << (uint32_t)(tsConf->GetRcIndex ()) <<
               ", carrier: " << carrierId);

  transmitDelay = GetRealSendingTime (transmitDelay);
  if (transmitDelay >= Seconds (0))
    {
      Simulator::Schedule (transmitDelay, &SatUtMac::DoTransmit, this, duration, carrierId, wf, tsConf, SatUtScheduler::LOOSE);
    }
}


void
SatUtMac::DoTransmit (Time duration, uint32_t carrierId, Ptr<SatWaveform> wf, Ptr<SatTimeSlotConf> tsConf, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << wf->GetPayloadInBytes () << carrierId << (uint32_t)(tsConf->GetRcIndex ()));

  if (!m_txCheckCallback ())
    {
      NS_LOG_INFO ("Tx is unavailable");
      m_rcstState.SwitchToHoldStandby ();
      return;
    }

  if ((m_rcstState.GetState () != SatUtMacState::RcstState_t::TDMA_SYNC) && (tsConf->GetSlotType () != SatTimeSlotConf::SLOT_TYPE_C) && m_useLogon)
    {
      return;
    }


  SatPhy::PacketContainer_t packets = FetchPackets (wf->GetPayloadInBytes (), tsConf->GetSlotType (), tsConf->GetRcIndex (), policy);

  if (wf == m_superframeSeq->GetWaveformConf ()->GetWaveform (2))
    {
      if (packets.size () == 0) // TODO send dummy packet if constrol slot empty ?
        {
          Ptr<Packet> p = Create<Packet> (wf->GetPayloadInBytes ());

          // Mark the PDU with FULL_PDU tag
          SatEncapPduStatusTag tag;
          tag.SetStatus (SatEncapPduStatusTag::FULL_PDU);
          p->AddPacketTag (tag);

          // Add MAC tag to identify the packet in lower layers
          SatMacTag mTag;
          mTag.SetDestAddress (m_gwAddress);
          mTag.SetSourceAddress (m_nodeInfo->GetMacAddress ());
          p->AddPacketTag (mTag);




          packets.push_back (p);
          //std::cout << "Send dummy control burst for UT " << m_nodeInfo->GetMacAddress () << std::endl;
          //NS_FATAL_ERROR ("STOP");
        }
    }

  NS_LOG_INFO ("DA Tx opportunity for UT: " << m_nodeInfo->GetMacAddress () <<
               " duration: " << duration.GetSeconds () <<
               ", payload: " << wf->GetPayloadInBytes () <<
               ", carrier: " << carrierId <<
               ", RC index: " << (uint32_t)(tsConf->GetRcIndex ()));

  SatSignalParameters::txInfo_s txInfo;
  txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
  txInfo.modCod = wf->GetModCod ();
  txInfo.fecBlockSizeInBytes = wf->GetPayloadInBytes ();
  txInfo.frameType = SatEnums::UNDEFINED_FRAME;
  txInfo.waveformId = wf->GetWaveformId ();

  if (txInfo.waveformId == 2 && tsConf->GetSlotType () != SatTimeSlotConf::SLOT_TYPE_C)
    {
      // TODO remove after use
      NS_FATAL_ERROR ("NO !");
    }

  TransmitPackets (packets, duration, carrierId, txInfo);
}

void
SatUtMac::DoSlottedAlohaTransmit (Time duration, Ptr<SatWaveform> waveform, uint32_t carrierId, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << waveform->GetPayloadInBytes () << carrierId << (uint32_t)(rcIndex));
  NS_LOG_INFO ("Tx opportunity for UT: " << m_nodeInfo->GetMacAddress () <<
               " duration: " << duration.GetSeconds () <<
               ", payload: " << waveform->GetPayloadInBytes () <<
               ", carrier: " << carrierId <<
               ", RC index: " << (uint32_t)(rcIndex));

  SatPhy::PacketContainer_t packets;

  /// get the slot payload
  uint32_t payloadBytes = waveform->GetPayloadInBytes ();

  /// reduce the SA signaling overhead from the payload
  payloadBytes -= m_randomAccess->GetSlottedAlohaSignalingOverheadInBytes ();

  if (payloadBytes < 1)
    {
      NS_FATAL_ERROR ("SatUtMac::DoSlottedAlohaTransmit - Not enough capacity in Slotted ALOHA payload");
    }

  ExtractPacketsToSchedule (packets, payloadBytes, SatTimeSlotConf::SLOT_TYPE_C, rcIndex, policy, true);

  if ( !packets.empty () )
    {
      NS_LOG_INFO ("Number of packets sent in a slotted ALOHA slot: " << packets.size ());

      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
           it != packets.end ();
           ++it)
        {
          // Add packet trace entry:
          m_packetTrace (Simulator::Now (),
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
      txInfo.fecBlockSizeInBytes = waveform->GetPayloadInBytes ();
      txInfo.frameType = SatEnums::UNDEFINED_FRAME;
      txInfo.waveformId = waveform->GetWaveformId ();

      TransmitPackets (packets, duration, carrierId, txInfo);
    }
}

void
SatUtMac::DoEssaTransmit (Time duration, Ptr<SatWaveform> waveform, uint32_t carrierId, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << duration.GetSeconds () << waveform->GetPayloadInBytes () << carrierId << (uint32_t)(rcIndex));
  NS_LOG_INFO ("SatUtMac::DoEssaTransmit - Tx opportunity for UT: " << m_nodeInfo->GetMacAddress () << " at time: " << Simulator::Now ().GetSeconds () << " duration: " << duration.GetSeconds () << ", payload: " << waveform->GetPayloadInBytes () << ", carrier: " << carrierId << ", RC index: " << (uint32_t)(rcIndex));

  /// get the slot payload
  uint32_t payloadBytes = waveform->GetPayloadInBytes ();

  /// get the next packets
  SatPhy::PacketContainer_t packets;
  m_utScheduler->DoScheduling (packets, payloadBytes, SatTimeSlotConf::SLOT_TYPE_TRC, rcIndex, policy);

  if ( !packets.empty () )
    {
      NS_LOG_INFO ("Number of packets sent in a ESSA frame: " << packets.size ());

      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
           it != packets.end ();
           ++it)
        {
          // Add packet trace entry:
          m_packetTrace (Simulator::Now (),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_MAC,
                         SatEnums::LD_RETURN,
                         SatUtils::GetPacketInfo (*it));
        }

      /// create ESSA Tx params
      SatSignalParameters::txInfo_s txInfo;
      txInfo.packetType = SatEnums::PACKET_TYPE_ESSA;
      txInfo.modCod = waveform->GetModCod ();
      txInfo.fecBlockSizeInBytes = waveform->GetPayloadInBytes ();
      txInfo.frameType = SatEnums::UNDEFINED_FRAME;
      txInfo.waveformId = waveform->GetWaveformId ();
      txInfo.crdsaUniquePacketId = m_crdsaUniquePacketId; // reuse the crdsaUniquePacketId to identify ESSA frames

      TransmitPackets (packets, duration, carrierId, txInfo);
      m_crdsaUniquePacketId++;
      /// update m_nextPacketTime
      m_nextPacketTime = Now () + duration; // TODO: this doesn't take into account the guard bands !!
      /// schedule a DoRandomAccess then in case there still are packets to transmit
      /// ( schedule DoRandomAccess in case there is a back-off to compute )
      duration = GetRealSendingTime (duration);
      if (duration >= Seconds (0))
        {
          Simulator::Schedule (duration, &SatUtMac::DoRandomAccess, this, SatEnums::RA_TRIGGER_TYPE_ESSA);
        }
      m_isRandomAccessScheduled = true;
    }
  else
    {
      m_isRandomAccessScheduled = false;
    }
}

SatPhy::PacketContainer_t
SatUtMac::FetchPackets (uint32_t payloadBytes, SatTimeSlotConf::SatTimeSlotType_t type, uint8_t rcIndex, SatUtScheduler::SatCompliancePolicy_t policy)
{
  NS_LOG_FUNCTION (this << payloadBytes << (uint32_t) (rcIndex));

  /**
   * Instruct the UT scheduler to fill the packet container based on given
   * input; e.g. payload, RC index. The packet container models the FPDU,
   * which may contain several RLE PDUs
   */
  SatPhy::PacketContainer_t packets;

  if (payloadBytes <= 0)
    {
      NS_FATAL_ERROR ("SatUtMac::FetchPackets - unvalid slot payload: " << payloadBytes);
    }

  ExtractPacketsToSchedule (packets, payloadBytes, type, rcIndex, policy, false);

  // A valid packet received
  if ( !packets.empty () )
    {
      NS_LOG_INFO ("Number of packets: " << packets.size ());

      for (SatPhy::PacketContainer_t::const_iterator it = packets.begin ();
           it != packets.end ();
           ++it)
        {
          // Add packet trace entry:
          m_packetTrace (Simulator::Now (),
                         SatEnums::PACKET_SENT,
                         m_nodeInfo->GetNodeType (),
                         m_nodeInfo->GetNodeId (),
                         m_nodeInfo->GetMacAddress (),
                         SatEnums::LL_MAC,
                         SatEnums::LD_RETURN,
                         SatUtils::GetPacketInfo (*it));
        }
    }

  NS_LOG_INFO ("The Frame PDU holds " << packets.size () << " RLE PDUs");

  return packets;
}

void
SatUtMac::TransmitPackets (SatPhy::PacketContainer_t packets, Time duration, uint32_t carrierId, SatSignalParameters::txInfo_s txInfo)
{
  NS_LOG_FUNCTION (this << packets.size () << duration.GetSeconds () << carrierId);

  if (m_rcstState.GetState () == SatUtMacState::RcstState_t::HOLD_STANDBY)
    {
      m_rcstState.SwitchToOffStandby ();
    }

  // TODO improve TDMA sync
  // TODO put somewhere else ?
  if (m_rcstState.GetState () == SatUtMacState::RcstState_t::READY_FOR_TDMA_SYNC)
    {
      m_rcstState.SwitchToTdmaSync ();
    }

  // If there are packets to send
  if (!packets.empty ())
    {
      NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () <<
                   ", transmitting " << packets.size () <<
                   " packets, duration: " << duration.GetSeconds () <<
                   ", carrier: " << carrierId);

      // Decrease a guard time from time slot duration.
      Time durationWithoutGuardPeriod (duration - m_guardTime);
      NS_LOG_INFO ("Duration: " << duration.GetSeconds () << " duration with guard period: " << durationWithoutGuardPeriod.GetSeconds ());
      NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " send packet");

      //uint32_t driftTicks = Simulator::Now ().GetMicroSeconds ()*m_clockDrift/1000000;
      //int32_t deltaTicks = driftTicks - m_deltaNcr;

      SendPacket (packets, carrierId, durationWithoutGuardPeriod, txInfo);
    }
}

void
SatUtMac::ReceiveQueueEvent (SatQueue::QueueEvent_t event, uint8_t rcIndex)
{
  NS_LOG_FUNCTION (this << event << (uint32_t) rcIndex);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " Queue: " << (uint32_t)rcIndex);

  // Check only the queue events from the control queue
  if (rcIndex == SatEnums::CONTROL_FID)
    {
      if (event == SatQueue::FIRST_BUFFERED_PKT || event == SatQueue::BUFFERED_PKT)
        {
          NS_LOG_INFO ("Buffered packet event received");

          if (m_randomAccess != NULL)
            {
              NS_LOG_INFO ("Doing Slotted ALOHA");

              DoRandomAccess (SatEnums::RA_TRIGGER_TYPE_SLOTTED_ALOHA);
            }
        }
    }
}

void
SatUtMac::ReceiveQueueEventEssa (SatQueue::QueueEvent_t event, uint8_t rcIndex)
{
  NS_LOG_FUNCTION (this << event << (uint32_t) rcIndex);

  NS_LOG_INFO ("SatUtMac::ReceiveQueueEventEssa - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " Queue: " << (uint32_t)rcIndex);

  // DoRandomAccess only if it is not scheduled yet
  // NOTE: could use m_nextPacketTime to do the check
  if (!m_isRandomAccessScheduled)
    {
      if (event == SatQueue::FIRST_BUFFERED_PKT || event == SatQueue::BUFFERED_PKT)
        {
          NS_LOG_INFO ("SatUtMac::ReceiveQueueEventEssa - Buffered packet event received");

          if (m_randomAccess != NULL)
            {
              NS_LOG_INFO ("SatUtMac::ReceiveQueueEventEssa - Doing ESSA");

              DoRandomAccess (SatEnums::RA_TRIGGER_TYPE_ESSA);
            }
        }
    }
}

void
SatUtMac::SendLogon (Ptr<Packet> packet)
{
  SatPhy::PacketContainer_t packets;
  packets.push_back (packet);

  Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
  uint8_t frameId = superframeConf->GetRaChannelFrameId (m_logonChannel);
  Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);

  Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf (0);

  Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (timeSlotConf->GetWaveFormId ());
  Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

  uint32_t carrierId = m_superframeSeq->GetCarrierId (SatConstVariables::SUPERFRAME_SEQUENCE, frameId, timeSlotConf->GetCarrierId () );

  SatSignalParameters::txInfo_s txInfo;
  txInfo.packetType = SatEnums::PACKET_TYPE_LOGON;
  txInfo.modCod = wf->GetModCod ();
  txInfo.sliceId = 0;
  txInfo.fecBlockSizeInBytes = wf->GetPayloadInBytes ();
  txInfo.frameType = SatEnums::UNDEFINED_FRAME;
  txInfo.waveformId = wf->GetWaveformId ();

  Time waitingTime = Seconds (m_waitingTimeLogonRng->GetValue (0.0, pow (1 + m_sendLogonTries, 2) * m_windowInitLogon.GetSeconds ()));
  m_sendLogonTries++;
  waitingTime = GetRealSendingTime (waitingTime);
  if (waitingTime >= Seconds (0))
    {
      Simulator::Schedule (waitingTime, &SatUtMac::TransmitPackets, this, packets, duration, carrierId, txInfo);
    }

  m_nextLogonTransmissionPossible = Simulator::Now () + m_maxWaitingTimeLogonResponse + waitingTime;
}

void
SatUtMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this << packets.size ());

  // Add packet trace entry:
  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_RECV,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_MAC,
                 SatEnums::LD_FORWARD,
                 SatUtils::GetPacketInfo (packets));

  // Invoke the `Rx` and `RxDelay` trace sources.
  RxTraces (packets);

  m_receptionDates.push (Simulator::Now ());
  if (m_receptionDates.size () > 3)
    {
      m_receptionDates.pop ();
    }

  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      // Remove packet tag
      SatMacTag macTag;
      bool mSuccess = (*i)->PeekPacketTag (macTag);

      if (!mSuccess)
        {
          NS_FATAL_ERROR ("MAC tag was not found from the packet!");
        }

      NS_LOG_INFO ("Packet from " << macTag.GetSourceAddress () << " to " << macTag.GetDestAddress ());
      NS_LOG_INFO ("Receiver " << m_nodeInfo->GetMacAddress ());

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
              m_rxCallback (*i, macTag.GetSourceAddress (), destAddress);
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

        /**
         * Control message NOT found in container anymore! This means, that the
         * SatBeamHelper::CtrlMsgStoreTimeInFwdLink attribute may be set to too short value
         * or there are something wrong in the FWD link RRM.
         */
        if (tbtp == NULL)
          {
            NS_FATAL_ERROR ("TBTP not found, check SatBeamHelper::CtrlMsgStoreTimeInFwdLink attribute is long enough!");
          }

        if (m_handoverState == WAITING_FOR_TBTP)
          {
            m_handoverState = NO_HANDOVER;
            m_firstTransmittableSuperframeId = tbtp->GetSuperframeCounter ();
          }
        ScheduleTimeSlots (tbtp);

        packet->RemovePacketTag (macTag);
        packet->RemovePacketTag (ctrlTag);

        break;
      }
    case SatControlMsgTag::SAT_ARQ_ACK:
      {
        // ARQ ACK messages are forwarded to LLC, since they may be fragmented
        m_rxCallback (packet, macTag.GetSourceAddress (), macTag.GetDestAddress ());
        break;
      }
    case SatControlMsgTag::SAT_RA_CTRL_MSG:
      {
        uint32_t raCtrlId = ctrlTag.GetMsgId ();
        Ptr<SatRaMessage> raMsg = DynamicCast<SatRaMessage> (m_readCtrlCallback (raCtrlId));

        if (raMsg != NULL)
          {
            uint32_t allocationChannelId = raMsg->GetAllocationChannelId ();
            uint16_t backoffProbability = raMsg->GetBackoffProbability ();
            uint16_t backoffTime = raMsg->GetBackoffTime ();

            NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () <<
                         "  Updating RA backoff probability for AC: " << allocationChannelId <<
                         " to: " << backoffProbability);

            m_randomAccess->SetBackoffProbability (allocationChannelId, backoffProbability);
            m_randomAccess->SetBackoffTime (allocationChannelId, backoffTime);

            packet->RemovePacketTag (macTag);
            packet->RemovePacketTag (ctrlTag);
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInFwdLink attribute may be set to too short value
             * or there are something wrong in the FWD link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the FWD link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }
        break;
      }
    case SatControlMsgTag::SAT_TIMU_CTRL_MSG:
      {
        uint32_t timuCtrlId = ctrlTag.GetMsgId ();
        Ptr<SatTimuMessage> timuMsg = DynamicCast<SatTimuMessage> (m_readCtrlCallback (timuCtrlId));

        if (timuMsg != NULL)
          {
            uint32_t beamId = timuMsg->GetAllocatedBeamId ();
            NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () <<
                         " switching from beam " << m_beamId << " to beam " << beamId);
            if (m_beamId != beamId)
              {
                NS_LOG_INFO ("Storing TIM-U information internally for later");
                m_timuInfo = Create<SatTimuInfo> (beamId, timuMsg->GetGwAddress ());
              }
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInFwdLink attribute may be set to too short value
             * or there are something wrong in the FWD link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the FWD link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }
        break;
      }
    case SatControlMsgTag::SAT_SLICE_CTRL_MSG:
      {
        uint32_t sliceCtrlId = ctrlTag.GetMsgId ();
        Ptr<SatSliceSubscriptionMessage> sliceMsg = DynamicCast<SatSliceSubscriptionMessage> (m_readCtrlCallback (sliceCtrlId));

        if (sliceMsg != NULL)
          {
            if (m_nodeInfo->GetMacAddress () == sliceMsg->GetAddress ())
              {
                m_sliceSubscriptionCallback (sliceMsg->GetSliceId ());
              }
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInFwdLink attribute may be set to too short value
             * or there are something wrong in the FWD link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the FWD link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }
        break;
      }
    case SatControlMsgTag::SAT_LOGON_RESPONSE_CTRL_MSG:
      {
        uint32_t logonId = ctrlTag.GetMsgId ();
        Ptr<SatLogonResponseMessage> logonMsg = DynamicCast<SatLogonResponseMessage> (m_readCtrlCallback (logonId));

        if (logonMsg != NULL)
          {
            m_raChannel = logonMsg->GetRaChannel ();
            m_loggedOn = true;
            m_sendLogonTries = 0;
            m_rcstState.SetLogOffCallback (MakeCallback (&SatUtMac::LogOff, this));
            m_rcstState.SwitchToReadyForTdmaSync ();
            m_deltaNcr = Simulator::Now ().GetMicroSeconds ()*m_clockDrift/1000000.0;
            std::cout << "UT " << m_nodeInfo->GetMacAddress () << " has been logged in. Correction is " << m_deltaNcr << std::endl;
          }
        else
          {
            /**
             * Control message NOT found in container anymore! This means, that the
             * SatBeamHelper::CtrlMsgStoreTimeInFwdLink attribute may be set to too short value
             * or there are something wrong in the FWD link RRM.
             */
            std::stringstream msg;
            msg << "Control message " << ctrlTag.GetMsgType () << " is not found from the FWD link control msg container!";
            msg << " at: " << Now ().GetSeconds () << "s";
            Singleton<SatLog>::Get ()->AddToLog (SatLog::LOG_WARNING, "", msg.str ());
          }
        break;
      }
    case SatControlMsgTag::SAT_NCR_CTRL_MSG:
      {
        uint32_t ncrCtrlId = ctrlTag.GetMsgId ();
        Ptr<SatNcrMessage> ncrMsg = DynamicCast<SatNcrMessage> (m_readCtrlCallback (ncrCtrlId));

        m_lastNcrDateReceived = m_ncrV2 ? m_receptionDates.front () : Simulator::Now ();
        m_ncr = ncrMsg->GetNcrDate ();
        m_rcstState.NcrControlMessageReceived ();

        if (m_rcstState.GetState () == SatUtMacState::RcstState_t::NCR_RECOVERY)
          {
            if (ControlMsgTransmissionPossible ())
              {
                m_rcstState.SwitchToReadyForTdmaSync ();
              }
            else
              {
                m_rcstState.SwitchToReadyForLogon ();
              }
          }
        break;
      }
    case SatControlMsgTag::SAT_CMT_CTRL_MSG:
      {
        uint32_t cmtCtrlId = ctrlTag.GetMsgId ();
        Ptr<SatCmtMessage> cmtMsg = DynamicCast<SatCmtMessage> (m_readCtrlCallback (cmtCtrlId));
        int16_t burstTimeCorrection = cmtMsg->GetBurstTimeCorrection ();
        m_deltaNcr -= burstTimeCorrection;
        //std::cout << "CMT message received at " << m_nodeInfo->GetMacAddress () << ", correction is " << burstTimeCorrection << ", total is " << m_deltaNcr << std::endl;
        break;
      }
    case SatControlMsgTag::SAT_LOGOFF_CTRL_MSG:
      {
        uint32_t logoffCtrlId = ctrlTag.GetMsgId ();
        Ptr<SatLogoffMessage> logoffMsg = DynamicCast<SatLogoffMessage> (m_readCtrlCallback (logoffCtrlId));
        LogOff ();
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
  NS_LOG_FUNCTION (this << randomAccessTriggerType);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress ());

  if ((m_rcstState.GetState () != SatUtMacState::RcstState_t::TDMA_SYNC) && m_useLogon)
    {
      return;
    }

  /// reset the isRandomAccessScheduled flag. TODO: should be done only if randomAccessTriggerType is ESSA
  m_isRandomAccessScheduled = false;

  SatRandomAccess::RandomAccessTxOpportunities_s txOpportunities;

  /// select the RA allocation channel
  uint32_t allocationChannel = GetNextRandomAccessAllocationChannel ();

  if (m_useLogon && allocationChannel == m_logonChannel)
    {
      NS_LOG_INFO ("Logon channel cannot be used for RA transmition");
      return;
    }

  /// run random access algorithm
  txOpportunities = m_randomAccess->DoRandomAccess (allocationChannel, randomAccessTriggerType);

  /// process Slotted ALOHA Tx opportunities
  if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_SLOTTED_ALOHA)
    {
      Time txOpportunity = GetRealSendingTime (Time::FromInteger (txOpportunities.slottedAlohaTxOpportunity, Time::MS));

      NS_LOG_INFO ("Processing Slotted ALOHA results, Tx evaluation @: " << (Now () + txOpportunity).GetSeconds () << " seconds");

      /// schedule the check for next available RA slot
      if (txOpportunity >= Seconds (0))
        {
          Simulator::Schedule (txOpportunity, &SatUtMac::ScheduleSlottedAlohaTransmission, this, allocationChannel);
        }
    }
  /// process CRDSA Tx opportunities
  else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_CRDSA)
    {
      NS_LOG_INFO ("Processing CRDSA results");

      /// schedule CRDSA transmission
      ScheduleCrdsaTransmission (allocationChannel, txOpportunities);
    }
  /// process ESSA opportunities
  else if (txOpportunities.txOpportunityType == SatEnums::RA_TX_OPPORTUNITY_ESSA)
    {
      NS_LOG_INFO ("SatUtMac::DoRandomAccess - Processing ESSA results");
      /// set the is RA scheduled
      /// TODO: if there are no Tx opportunities there'll be no scheduling until a new event arrives
      m_isRandomAccessScheduled = true;
      Time txOpportunity = GetRealSendingTime (Time::FromInteger (txOpportunities.slottedAlohaTxOpportunity, Time::MS));

      /// schedule the transmission
      if (txOpportunity >= Seconds (0))
        {
          Simulator::Schedule (txOpportunity, &SatUtMac::ScheduleEssaTransmission, this, allocationChannel);
        }
    }
}

uint32_t
SatUtMac::GetNextRandomAccessAllocationChannel ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " has allocation channel " << m_raChannel);

  /// TODO at the moment only one allocation channel is supported
  return m_raChannel;
}

void
SatUtMac::ScheduleSlottedAlohaTransmission (uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this << allocationChannel);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " AC: " << allocationChannel);

  /// check if we have known DAMA allocations
  /// this functionality checks the current and all known future frames for DAMA allocation
  /// TODO it might be better to check only the current frame or a limited subset of frames
  if ( !m_tbtpContainer->HasScheduledTimeSlots () )
    {
      NS_LOG_INFO ("No known DAMA, selecting a slot for Slotted ALOHA");

      Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
      uint8_t frameId = superframeConf->GetRaChannelFrameId (allocationChannel);
      Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
      uint32_t timeSlotCount = frameConf->GetTimeSlotCount ();

      std::pair<bool, uint32_t> result = std::make_pair (false, 0);
      Time superframeStartTime = GetCurrentSuperFrameStartTime (SatConstVariables::SUPERFRAME_SEQUENCE);

      if ( Now () < superframeStartTime )
        {
          NS_FATAL_ERROR ("SatUtMac::ScheduleSlottedAlohaTransmission - Invalid SF start time");
        }

      uint32_t superFrameId = GetCurrentSuperFrameId (SatConstVariables::SUPERFRAME_SEQUENCE);

      NS_LOG_INFO ("Searching for next available slot");

      /// search for the next available slot
      /// if there is no free slots in the current frame, look for it in the following frames
      while (!result.first)
        {
          NS_LOG_INFO ("SuperFrameId: " << superFrameId << ", superframeStartTime: " << superframeStartTime.GetSeconds ());

          result = SearchFrameForAvailableSlot (superframeStartTime, frameConf, timeSlotCount, superFrameId, allocationChannel);

          if (!result.first)
            {
              NS_LOG_INFO ("Increasing frame offset!");
              superFrameId++;
              superframeStartTime += frameConf->GetDuration ();
            }
        }

      /// time slot configuration
      Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( result.second );

      /// start time
      Time slotStartTime = superframeStartTime + timeSlotConf->GetStartTime ();
      Time offset = slotStartTime - Now ();

      if (offset.IsStrictlyNegative ())
        {
          NS_FATAL_ERROR ("SatUtMac::ScheduleSlottedAlohaTransmission - Invalid transmit time: " << offset.GetSeconds ());
        }

      /// duration
      Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (timeSlotConf->GetWaveFormId ());
      Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

      /// carrier
      uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

      NS_LOG_INFO ("Starting to schedule, SF ID: " << superFrameId <<
                   " slot: " << result.second <<
                   " SF start: " << superframeStartTime.GetSeconds () <<
                   " Tx start: " << (Now () + offset).GetSeconds () <<
                   " duration: " << duration.GetSeconds () <<
                   " carrier ID: " << carrierId <<
                   " payload in bytes: " << wf->GetPayloadInBytes ());

      /// schedule transmission
      offset = GetRealSendingTime (offset);
      if (offset >= Seconds (0))
        {
          Simulator::Schedule (offset, &SatUtMac::DoSlottedAlohaTransmit, this, duration, wf, carrierId, uint8_t (SatEnums::CONTROL_FID), SatUtScheduler::STRICT);
        }
    }
  else
    {
      NS_LOG_INFO ("UT has known DAMA, aborting Slotted ALOHA");
    }
}

std::pair<bool, uint32_t>
SatUtMac::SearchFrameForAvailableSlot (Time superframeStartTime,
                                       Ptr<SatFrameConf> frameConf,
                                       uint32_t timeSlotCount,
                                       uint32_t superFrameId,
                                       uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this << superframeStartTime << timeSlotCount << superFrameId << allocationChannel);

  Time opportunityOffset = Now () - superframeStartTime;

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " offset: " << opportunityOffset.GetSeconds ());

  /// if we can not use the current superframe, e.g., we are in the the middle of the last slot of the frame, we will use offset 0, e.g., the first slot of the next frame
  if (opportunityOffset.IsStrictlyNegative ())
    {
      opportunityOffset = Seconds (0);
    }

  return FindNextAvailableRandomAccessSlot (opportunityOffset, frameConf, timeSlotCount, superFrameId, allocationChannel);
}

std::pair<bool, uint32_t>
SatUtMac::FindNextAvailableRandomAccessSlot (Time opportunityOffset,
                                             Ptr<SatFrameConf> frameConf,
                                             uint32_t timeSlotCount,
                                             uint32_t superFrameId,
                                             uint32_t allocationChannel)
{
  NS_LOG_FUNCTION (this << opportunityOffset << timeSlotCount << superFrameId << allocationChannel);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress ());

  Ptr<SatTimeSlotConf> slotConf;
  uint32_t slotId;
  bool availableSlotFound = false;

  /// iterate through slots in this frame
  for (slotId = 0; slotId < timeSlotCount; slotId++)
    {
      slotConf = frameConf->GetTimeSlotConf (slotId);

      //NS_LOG_INFO ("Slot: " << slotId <<
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

  NS_LOG_INFO ("Success: " << availableSlotFound <<
               " SF: " << superFrameId <<
               " AC: " << allocationChannel <<
               " slot: " << slotId << "/" << timeSlotCount);

  return std::make_pair (availableSlotFound, slotId);
}

void
SatUtMac::ScheduleEssaTransmission (uint32_t allocationChannel)
{
  // TODO: do we really need the allocationChannel ???
  NS_LOG_FUNCTION (this << allocationChannel);

  NS_LOG_INFO ("SatUtMac::ScheduleEssaTransmission - UT: " << m_nodeInfo->GetMacAddress () << " time: " << Now ().GetSeconds () << " AC: " << allocationChannel);

  /// start time
  Time offset = m_nextPacketTime - Now ();

  if (offset.IsStrictlyNegative ())
    {
      /// not transmiting at the moment, transmit now
      offset = Seconds (0);
    }

  /// duration
  Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
  uint8_t frameId = superframeConf->GetRaChannelFrameId (allocationChannel);
  Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
  Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( 0 ); // only one timeslot on ESSA

  Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (timeSlotConf->GetWaveFormId ());
  Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

  NS_LOG_INFO ("SatUtMac::ScheduleEssaTransmission - Starting to schedule @ " << Now ().GetSeconds () <<
               " Tx start: " << (Now () + offset).GetSeconds () <<
               " duration: " << duration.GetSeconds () <<
               " payload in bytes: " << wf->GetPayloadInBytes ());

  /// carrier
  uint32_t carrierId = 0; // TODO: for now we use 0 as we have a single carrier

  /// schedule transmission
  offset = GetRealSendingTime (offset);
  if (offset >= Seconds (0))
    {
      Simulator::Schedule (offset, &SatUtMac::DoEssaTransmit, this, duration, wf, carrierId, uint8_t (SatEnums::CONTROL_FID), SatUtScheduler::LOOSE);
    }
}

void
SatUtMac::ScheduleCrdsaTransmission (uint32_t allocationChannel, SatRandomAccess::RandomAccessTxOpportunities_s txOpportunities)
{
  NS_LOG_FUNCTION (this << allocationChannel);

  // get current superframe ID
  Time now = Simulator::Now ();
  Time superFrameStart = GetCurrentSuperFrameStartTime (SatConstVariables::SUPERFRAME_SEQUENCE);
  uint32_t superFrameId = GetCurrentSuperFrameId (SatConstVariables::SUPERFRAME_SEQUENCE);
  NS_LOG_INFO ("Checking for CRDSA transmission at " <<
               now.GetMilliSeconds () - superFrameStart.GetMilliSeconds () <<
               " milliseconds into superframe " << superFrameId);

  // TODO: check we didn't already scheduled packets for this superframe
  // (because we are moving so fast, for instance, that we are now at the end of the
  // window for this "past" superframe instead of the beginning of the next one)

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () <<
               " AC: " << allocationChannel <<
               ", SF: " << superFrameId <<
               ", num of opportunities: " << txOpportunities.crdsaTxOpportunities.size ());

  std::map<uint32_t, std::set<uint32_t> >::iterator iter;

  /// loop through the unique packets
  for (iter = txOpportunities.crdsaTxOpportunities.begin (); iter != txOpportunities.crdsaTxOpportunities.end (); iter++)
    {
      std::set<uint32_t>::iterator iterSet;

      /// loop through the replicas
      for (iterSet = iter->second.begin (); iterSet != iter->second.end (); iterSet++)
        {
          /// check and update used slots
          if (!UpdateUsedRandomAccessSlots (superFrameId, allocationChannel, *iterSet))
            {
              /// TODO this needs to be handled when multiple allocation channels are implemented
              /// In that case a slot exclusion list should be used when randomizing the Tx slots
              NS_FATAL_ERROR ("SatUtMac::ScheduleCrdsaTransmission - Slot unavailable: " << *iterSet);
            }
        }

      /// create replicas and schedule the packets
      NS_LOG_INFO ("Creating replicas for packet " << (uint32_t)m_crdsaUniquePacketId);
      CreateCrdsaPacketInstances (allocationChannel, iter->second);
    }
}

void
SatUtMac::CreateCrdsaPacketInstances (uint32_t allocationChannel, std::set<uint32_t> slots)
{
  NS_LOG_FUNCTION (this << allocationChannel);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " AC: " << allocationChannel);

  Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
  uint8_t frameId = superframeConf->GetRaChannelFrameId (allocationChannel);
  Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);

  /// CRDSA is evaluated only at the frame start
  /// TODO this has to be changed when a proper mobility model is implemented
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
  ExtractPacketsToSchedule (uniq, payloadBytes, SatTimeSlotConf::SLOT_TYPE_TRC, uint8_t (SatEnums::CONTROL_FID), policy, true);

  NS_LOG_INFO ("Processing the packet container, fragments: " << uniq.size ());

  if ( !uniq.empty () )
    {
      std::vector < std::pair< uint16_t, SatPhy::PacketContainer_t > > replicas;
      std::map <uint16_t, SatCrdsaReplicaTag> tags;
      std::set<uint32_t>::iterator iterSet;

      NS_LOG_INFO ("Creating replicas for a packet");

      /// create replicas
      for (iterSet = slots.begin (); iterSet != slots.end (); iterSet++)
        {
          SatPhy::PacketContainer_t rep;
          SatPhy::PacketContainer_t::const_iterator it = uniq.begin ();

          for (; it != uniq.end (); ++it)
            {
              rep.push_back ((*it)->Copy ());
              NS_LOG_INFO ("Replica in slot: " << (*iterSet)
                                               << ", original (HL packet) fragment UID: " << (*it)->GetUid ()
                                               << ", copied replica fragment (HL packet) UID: " << rep.back ()->GetUid ());
            }

          NS_LOG_INFO ("One replica created");

          replicas.push_back (std::make_pair (*iterSet, rep));
        }

      NS_LOG_INFO ("Creating replica tags");

      /// create replica slot ID tags
      for (uint32_t i = 0; i < replicas.size (); i++)
        {
          SatCrdsaReplicaTag replicaTag;

          replicaTag.AddSlotId (replicas[i].first);

          NS_LOG_INFO ("Own packet tag: " << replicas[i].first);

          for (uint32_t j = 0; j < replicas.size (); j++)
            {
              if (i != j)
                {
                  replicaTag.AddSlotId (replicas[j].first);

                  NS_LOG_INFO ("Other packet tag: " << replicas[j].first);
                }
            }
          tags.insert (std::make_pair (replicas[i].first, replicaTag));
        }

      NS_LOG_INFO ("Scheduling replicas");

      /// loop through the replicas
      for (uint32_t i = 0; i < replicas.size (); i++)
        {
          for (uint32_t j = 0; j < replicas[i].second.size (); j++)
            {
              NS_LOG_INFO ("Replica: " << i <<
                           ", fragment: " << j <<
                           ", key: " << replicas[i].first <<
                           ", tag: " << tags.at (replicas[i].first).GetSlotIds ().at (0) <<
                           ", fragment (HL packet) UID: " << replicas[i].second.at (j)->GetUid ());

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
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (timeSlotConf->GetWaveFormId ());
          Time duration = wf->GetBurstDuration (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

          /// carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (SatConstVariables::SUPERFRAME_SEQUENCE, frameId, timeSlotConf->GetCarrierId () );

          /// create CRDSA Tx params
          SatSignalParameters::txInfo_s txInfo;
          txInfo.packetType = SatEnums::PACKET_TYPE_CRDSA;
          txInfo.modCod = wf->GetModCod ();
          txInfo.sliceId = 0;
          txInfo.fecBlockSizeInBytes = wf->GetPayloadInBytes ();
          txInfo.frameType = SatEnums::UNDEFINED_FRAME;
          txInfo.waveformId = wf->GetWaveformId ();
          txInfo.crdsaUniquePacketId = m_crdsaUniquePacketId;

          /// schedule transmission
          offset = GetRealSendingTime (offset);
          if (offset >= Seconds (0))
            {
              Simulator::Schedule (offset, &SatUtMac::TransmitPackets, this, replicas[i].second, duration, carrierId, txInfo);
            }
          NS_LOG_INFO ("Scheduled a replica in slot " << replicas[i].first << " with offset " << offset.GetSeconds ());
        }
      replicas.clear ();
      tags.clear ();

      m_crdsaUniquePacketId++;
    }
}

bool
SatUtMac::UpdateUsedRandomAccessSlots (uint32_t superFrameId, uint32_t allocationChannelId, uint32_t slotId)
{
  NS_LOG_FUNCTION (this << superFrameId << allocationChannelId << slotId);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " SF: " << superFrameId << " AC: " << allocationChannelId << " slot: " << slotId);

  std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator iter;
  bool isSlotFree = false;

  /// remove past RA Tx opportunity information
  RemovePastRandomAccessSlots (superFrameId);

  std::pair <uint32_t, uint32_t> key = std::make_pair (superFrameId, allocationChannelId);

  iter = m_usedRandomAccessSlots.find (key);

  if (iter == m_usedRandomAccessSlots.end ())
    {
      std::set<uint32_t> txOpportunities {{slotId}};
      std::pair <std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator, bool> result;
      result = m_usedRandomAccessSlots.insert (std::make_pair (key, txOpportunities));

      if (result.second)
        {
          isSlotFree = true;
          NS_LOG_INFO ("No saved SF, slot " << slotId << " saved in SF " << superFrameId);
        }
      else
        {
          NS_LOG_WARN ("No saved SF but unable to create one");
        }
    }
  else
    {
      std::pair<std::set<uint32_t>::iterator, bool> result;
      result = iter->second.insert (slotId);

      if (result.second)
        {
          isSlotFree = true;
          NS_LOG_INFO ("Saved SF exist, slot " << slotId << " saved in SF " << superFrameId);
        }
      else
        {
          NS_LOG_WARN ("Saved SF exist but unable to add slot " << slotId);
        }
    }
  return isSlotFree;
}

void
SatUtMac::RemovePastRandomAccessSlots (uint32_t superFrameId)
{
  NS_LOG_FUNCTION (this << superFrameId);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress () << " SF: " << superFrameId);

  std::map < std::pair <uint32_t, uint32_t>, std::set<uint32_t> >::iterator iter;

  for (iter = m_usedRandomAccessSlots.begin (); iter != m_usedRandomAccessSlots.end (); )
    {
      if (iter->first.first < superFrameId)
        {
          iter = m_usedRandomAccessSlots.erase (iter);
        }
      else
        {
          ++iter;
        }
    }
}

void
SatUtMac::PrintUsedRandomAccessSlots ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress ());
  std::cout << "UT: " << m_nodeInfo->GetMacAddress () << std::endl;

  for (auto& iter : m_usedRandomAccessSlots)
    {
      for (auto& slot : iter.second)
        {
          std::cout << "SF: " << iter.first.first << " AC: " << iter.first.second << " slot: " << slot << std::endl;
        }
    }
}

void
SatUtMac::DoFrameStart ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("UT: " << m_nodeInfo->GetMacAddress ());

  if (m_timuInfo != NULL)
    {
      NS_LOG_INFO ("Applying TIM-U parameters received during the previous frame");

      m_beamId = m_timuInfo->GetBeamId ();
      Address gwAddress = m_timuInfo->GetGwAddress ();
      Mac48Address gwAddress48 = Mac48Address::ConvertFrom (gwAddress);
      if (gwAddress48 != m_gwAddress)
        {
          SetGwAddress (gwAddress48);
          m_routingUpdateCallback (m_nodeInfo->GetMacAddress (), gwAddress);
        }
      m_handoverCallback (m_beamId);

      m_tbtpContainer->Clear ();
      m_handoverState = WAITING_FOR_TBTP;
      m_timuInfo = NULL;
    }
  else if (m_txCheckCallback ())
    {
      NS_LOG_INFO ("Tx is permitted");

      if (m_rcstState.GetState () == SatUtMacState::RcstState_t::HOLD_STANDBY)
        {
          m_rcstState.SwitchToOffStandby ();
        }

      if (m_loggedOn && !m_beamCheckerCallback.IsNull ())
        {
          NS_LOG_INFO ("UT checking for beam handover recommendation");
          if (m_beamCheckerCallback (m_beamId))
            {
              if (m_handoverState == NO_HANDOVER)
                {
                  m_handoverState = HANDOVER_RECOMMENDATION_SENT;
                  m_handoverMessagesCount = 0;
                }
              else if (m_handoverState == HANDOVER_RECOMMENDATION_SENT)
                {
                  ++m_handoverMessagesCount;
                }
              if (m_useLogon && m_handoverMessagesCount > m_maxHandoverMessagesSent)
                {
                  m_handoverMessagesCount = 0;
                  LogOff ();

                  m_beamId = m_askedBeamCallback ();

                  Address gwAddress = m_beamScheculerCallback (m_beamId)->GetGwAddress ();
                  Mac48Address gwAddress48 = Mac48Address::ConvertFrom (gwAddress);
                  if (gwAddress48 != m_gwAddress)
                    {
                      SetGwAddress (gwAddress48);
                      m_updateGwAddressCallback (gwAddress48);
                      m_routingUpdateCallback (m_nodeInfo->GetMacAddress (), gwAddress);
                    }
                  m_handoverCallback (m_beamId);

                  m_tbtpContainer->Clear ();
                  m_handoverState = NO_HANDOVER;
                  m_nextLogonTransmissionPossible = Simulator::Now ();
                }
            }
        }

      if (m_randomAccess != NULL)
        {
          if (m_loggedOn)
            {
              // reset packet ID counter for this frame
              m_crdsaUniquePacketId = 1;

              // execute CRDSA trigger
              DoRandomAccess (SatEnums::RA_TRIGGER_TYPE_CRDSA);
            }
          else if (m_useLogon)
            {
              m_rcstState.SwitchToReadyForLogon ();
              if (Simulator::Now () > m_nextLogonTransmissionPossible)
                {
                  // Do Logon
                  m_sendLogonCallback ();
                }
            }
        }
    }
  else
    {
      NS_LOG_INFO ("Tx is disabled");
      m_rcstState.SwitchToHoldStandby ();
    }

  Time nextSuperFrameTxTime = GetNextSuperFrameTxTime (SatConstVariables::SUPERFRAME_SEQUENCE);
  NS_ASSERT_MSG (Now () < nextSuperFrameTxTime, "Scheduling next superframe start time to the past!");

  Time schedulingDelay = nextSuperFrameTxTime - Now ();
  Time realDelay = GetRealSendingTime (schedulingDelay);
    if (realDelay == Seconds (0)){
      schedulingDelay += m_superframeSeq->GetDuration (SatConstVariables::SUPERFRAME_SEQUENCE);
    }
  Simulator::Schedule (GetRealSendingTime (schedulingDelay), &SatUtMac::DoFrameStart, this);
}

Time
SatUtMac::GetRealSendingTime (Time t)
{
  if (m_deltaNcr == 0) // For some reason returning t-0 is different than returning t...
    {
      return t;
    }

  uint32_t driftTicks = (t + Simulator::Now ()).GetMicroSeconds ()*m_clockDrift/1000000;
  int32_t deltaTicks = m_deltaNcr - driftTicks;
  Time deltaTime = NanoSeconds (deltaTicks*1000/27.0);

  return t - deltaTime;
}

SatUtMac::SatTimuInfo::SatTimuInfo (uint32_t beamId, Address address)
  : m_beamId (beamId),
  m_gwAddress (address)
{
  NS_LOG_FUNCTION (this << beamId << address);
}


uint32_t
SatUtMac::SatTimuInfo::GetBeamId () const
{
  NS_LOG_FUNCTION (this);
  return m_beamId;
}


Address
SatUtMac::SatTimuInfo::GetGwAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_gwAddress;
}

SatUtMacState::RcstState_t
SatUtMac::GetRcstState () const
{
  NS_LOG_FUNCTION (this);
  return m_rcstState.GetState ();
}


void
SatUtMac::ExtractPacketsToSchedule (SatPhy::PacketContainer_t& packets,
                                    uint32_t payloadBytes,
                                    SatTimeSlotConf::SatTimeSlotType_t type,
                                    uint8_t rcIndex,
                                    SatUtScheduler::SatCompliancePolicy_t policy,
                                    bool randomAccessChannel)
{
  NS_LOG_INFO (this << payloadBytes << type << rcIndex << policy << randomAccessChannel);

  uint32_t superFrameId = GetCurrentSuperFrameId (SatConstVariables::SUPERFRAME_SEQUENCE);
  if ((m_handoverState == NO_HANDOVER && (randomAccessChannel || m_firstTransmittableSuperframeId <= superFrameId)) || (randomAccessChannel && m_handoverState != HANDOVER_RECOMMENDATION_SENT))
    {
      NS_LOG_INFO ("Regular scheduling");
      m_utScheduler->DoScheduling (packets, payloadBytes, type, rcIndex, policy);
    }
  else
    {
      NS_LOG_INFO ("Handover recommendation sent, force control packets only");
      m_utScheduler->DoScheduling (packets, payloadBytes, SatTimeSlotConf::SLOT_TYPE_C, SatEnums::CONTROL_FID, SatUtScheduler::STRICT);

      // Remove every control packets except handover requests
      SatPhy::PacketContainer_t::iterator it = packets.begin ();
      while (it != packets.end ())
        {
          SatControlMsgTag ctrlTag;
          bool success = (*it)->PeekPacketTag (ctrlTag);

          if (success && ctrlTag.GetMsgType () == SatControlMsgTag::SAT_HR_CTRL_MSG)
            {
              ++it;
            }
          else
            {
              it = packets.erase (it);
            }
        }
    }
}


} // namespace ns3
