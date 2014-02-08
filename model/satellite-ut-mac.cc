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
#include "../helper/satellite-wave-form-conf.h"

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
    .AddAttribute ("Cra",
                   "Constant Rate Assignment value for this UT Mac.",
                   DoubleValue (128),
                   MakeDoubleAccessor (&SatUtMac::m_cra),
                   MakeDoubleChecker<double> (0.0))
    .AddAttribute ("CrUpdatePeriod",
                   "Capacity request update period.",
                   TimeValue (MilliSeconds (250)),
                   MakeTimeAccessor (&SatUtMac::m_crInterval),
                   MakeTimeChecker ())
    .AddAttribute( "FramePduHeaderSize",
                   "Frame PDU header size in bytes",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SatUtMac::m_framePduHeaderSizeInBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("GuardTime",
                   "Guard time in return link",
                   TimeValue (MicroSeconds (1)),
                   MakeTimeAccessor (&SatUtMac::m_guardTime),
                   MakeTimeChecker ())
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
  m_timingAdvanceCb (),
  m_txCallback (),
  m_cra (),
  m_lastCno (),
  m_framePduHeaderSizeInBytes ()
{
  NS_LOG_FUNCTION (this);
  
  // default constructor should not be used
  NS_FATAL_ERROR ("SatUtMac::SatUtMac - Constructor not in use");
}

SatUtMac::SatUtMac (Ptr<SatSuperframeSeq> seq, uint32_t beamId, Ptr<SatRandomAccessConf> randomAccessConf, SatRandomAccess::RandomAccessModel_t randomAccessModel)
 : SatMac (beamId),
   m_superframeSeq (seq),
   m_timingAdvanceCb (0),
   m_txCallback (0),
   m_cra (),
   m_lastCno (NAN),
   m_framePduHeaderSizeInBytes ()
{
	NS_LOG_FUNCTION (this);

	if (randomAccessConf != NULL && randomAccessModel != SatRandomAccess::RA_OFF)
	  {
	    m_randomAccess = CreateObject<SatRandomAccess> (randomAccessConf, randomAccessModel);
	  }

  Simulator::Schedule (m_crInterval, &SatUtMac::SendCapacityReq, this);
}

SatUtMac::~SatUtMac ()
{
  NS_LOG_FUNCTION (this);

  m_randomAccess = NULL;
}

void
SatUtMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_timingAdvanceCb.Nullify ();

  SatMac::DoDispose ();
}

void SatUtMac::SetGwAddress (Mac48Address gwAddress)
{
  NS_LOG_FUNCTION (this);

  m_gwAddress = gwAddress;
}

void
SatUtMac::SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_timingAdvanceCb = cb;
}

void
SatUtMac::SetTxCallback (SatUtMac::SendCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_txCallback = cb;
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
  double superframeDuration = m_superframeSeq->GetDurationInSeconds (tbtp->GetSuperframeSeqId ());
  Time expectedReceiveTime = Seconds (superframeDuration * tbtp->GetSuperframeCounter ());
  Time twoWayPropagationDelay = m_timingAdvanceCb ();
  Time startTime = expectedReceiveTime - twoWayPropagationDelay;

  // The delay compared to Now when to start the transmission of this superframe
  Time startDelay = startTime - Simulator::Now ();

  // if the calculated start time of the superframe is already in the past
  if (startTime < Simulator::Now ())
    {
      NS_FATAL_ERROR ("UT: " << m_nodeInfo->GetMacAddress () << " received TBTP " << tbtp->GetSuperframeCounter () << ", which should have been sent already in the past");
    }

  // Schedule superframe start
  Simulator::Schedule (startDelay, &SatUtMac::SuperFrameStart, this, tbtp->GetSuperframeSeqId ());

  NS_LOG_LOGIC ("Calculated propagation delay between UT and GW: " << twoWayPropagationDelay.GetSeconds ());
  NS_LOG_LOGIC ("Time to start sending the superframe for this UT: " << startTime.GetSeconds ());

  //tbtp->Dump ();

  SatTbtpMessage::TimeSlotInfoContainer_t slots = tbtp->GetTimeslots (m_nodeInfo->GetMacAddress ());

  if ( !slots.empty ())
    {
      NS_LOG_LOGIC ("TBTP contains " << slots.size () << " timeslots for UT: " << m_nodeInfo->GetMacAddress ());

      uint8_t frameId = 0;

      // schedule time slots
      for ( SatTbtpMessage::TimeSlotInfoContainer_t::iterator it = slots.begin (); it != slots.end (); it++ )
        {
          // Store frame id from first slot and check later that frame id is same
          // If frame id changes in TBTP for same UT, raise error.
          if ( it == slots.begin () )
            {
              frameId = (*it)->GetFrameId ();
            }
          else if ( frameId != (*it)->GetFrameId ())
            {
              NS_FATAL_ERROR ("Error in TBTP: slot allocate from different frames for same UT!!!");
            }

          Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
          Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
          Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( (*it)->GetTimeSlotId () );

          // Start time
          Time slotStartTime = startDelay + Seconds (timeSlotConf->GetStartTimeInSeconds ());

          NS_LOG_LOGIC ("Slot start time " << slotStartTime.GetSeconds() << " for UT: " << m_nodeInfo->GetMacAddress ());

          // Duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
          double duration = wf->GetBurstDurationInSeconds (frameConf->GetBtuConf ()->GetSymbolRateInBauds ());

          // Carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, frameId, timeSlotConf->GetCarrierId () );

          ScheduleTxOpportunity (slotStartTime, duration, wf->GetPayloadInBytes (), carrierId);
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
SatUtMac::ScheduleTxOpportunity(Time transmitTime, double durationInSecs, uint32_t payloadBytes, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << transmitTime.GetSeconds() << durationInSecs << payloadBytes << carrierId);

  Simulator::Schedule (transmitTime, &SatUtMac::TransmitTime, this, durationInSecs, payloadBytes, carrierId);
}

void
SatUtMac::TransmitTime (double durationInSecs, uint32_t payloadBytes, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << durationInSecs << payloadBytes << carrierId);
  NS_LOG_LOGIC ("Tx opportunity for UT: " << m_nodeInfo->GetMacAddress () << " at time: " << Simulator::Now ().GetSeconds () << ": duration: " << durationInSecs << ", payload: " << payloadBytes << ", carrier: " << carrierId);

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
  uint32_t bytesLeftInBuffer (0);

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

      // TxOpportunity
      Ptr<Packet> p = m_txOpportunityCallback (payloadLeft, m_nodeInfo->GetMacAddress (), bytesLeftInBuffer);

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

  // If there are packets to send
  if (!packets.empty ())
    {
      // Decrease a guard time from time slot duration.
      Time duration (Time::FromDouble(durationInSecs, Time::S) - m_guardTime);

      NS_LOG_LOGIC ("UT: " << m_nodeInfo->GetMacAddress () << " send packet at time: " << Simulator::Now ().GetSeconds ());

      SendPacket (packets, carrierId, duration);

    }
}

void
SatUtMac::CnoUpdated (uint32_t beamId, Address /*utId*/, Address /*gwId*/, double cno)
{
  NS_LOG_FUNCTION (this << beamId << cno);

  // TODO: Some estimation algorithm needed to use, now we just save the latest received C/N0 info.
  m_lastCno = cno;
}

void
SatUtMac::SendCapacityReq ()
{

  if ( m_txCallback.IsNull () == false )
    {
      Ptr<Packet> packet = Create<Packet> ();

      // add tag to message
      SatControlMsgTag tag;
      tag.SetMsgType (SatControlMsgTag::SAT_CR_CTRL_MSG);
      packet->AddPacketTag (tag);

      // add TBTP specific header to message
      SatCapacityReqHeader header;
      header.SetReqType (SatCapacityReqHeader::SAT_RBDC_CR);

      // TODO: estimated value of C/N0 must be used instead of last received value
      header.SetCnoEstimate (m_lastCno);

      packet->AddHeader (header);

      m_txCallback (packet, m_gwAddress, Ipv4L3Protocol::PROT_NUMBER);

      Simulator::Schedule (m_crInterval, &SatUtMac::SendCapacityReq, this);
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

  // Hit the trace hooks.  All of these hooks are in the same place in this
  // device because it is so simple, but this is not usually the case in
  // more complicated devices.

  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      m_snifferTrace (*i);
      m_promiscSnifferTrace (*i);
      m_macRxTrace (*i);

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

        Ptr<SatTbtpMessage> tbtp = m_superframeSeq->GetTbtpMessage (m_beamId, tbtpId);

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

} // namespace ns3
