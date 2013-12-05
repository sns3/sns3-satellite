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
#include "satellite-ut-mac.h"
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
{
  NS_LOG_FUNCTION (this);
  
  // default constructor should not be used
  NS_ASSERT (false);
}

SatUtMac::SatUtMac (Ptr<SatSuperframeSeq> seq)
 : m_superframeSeq (seq)
{
	NS_LOG_FUNCTION (this);
}

SatUtMac::~SatUtMac ()
{
  NS_LOG_FUNCTION (this);
}

void
SatUtMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  SatMac::DoDispose ();
}


void
SatUtMac::SetTimingAdvanceCallback (SatUtMac::TimingAdvanceCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_timingAdvanceCb = cb;
}

void
SatUtMac::ScheduleTimeSlots (SatTbtpHeader * tbtp)
{
  NS_LOG_FUNCTION (this << tbtp);

  std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo > > slots = tbtp->GetTimeslots (m_macAddress);

  if ( !slots.empty ())
    {
      double superframeDuration = m_superframeSeq->GetDuration_s (tbtp->GetSuperframeId ());

      // TODO: start time must be calculated using reference or global clock
      Time startTime = Seconds (superframeDuration * tbtp->GetSuperframeCounter ());

      // schedule time slots
      for ( std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo > >::iterator it = slots.begin (); it != slots.end (); it++ )
        {
          Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (0);
          Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf ((*it)->GetFrameId ());
          Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf ( (*it)->GetTimeSlotId () );

          // Start time
          Time slotStartTime = startTime + Seconds (timeSlotConf->GetStartTime_s ());

          // Duration
          Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf()->GetWaveform (timeSlotConf->GetWaveFormId ());
          double duration = wf->GetBurstDurationInSeconds (frameConf->GetBtuConf ()->GetSymbolRate_baud ());

          // Carrier
          uint32_t carrierId = m_superframeSeq->GetCarrierId (0, (*it)->GetFrameId (), timeSlotConf->GetCarrierId () );

          ScheduleTxOpportunity (slotStartTime, duration, wf->GetPayloadInBytes (), carrierId);
        }
    }
}

void
SatUtMac::ScheduleTxOpportunity(Time transmitTime, double durationInSecs, uint32_t payloadBytes, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << transmitTime << durationInSecs << payloadBytes << carrierId);

  Simulator::Schedule (transmitTime, &SatUtMac::TransmitTime, this, durationInSecs, payloadBytes, carrierId);
}

void
SatUtMac::TransmitTime (double durationInSecs, uint32_t payloadBytes, uint32_t carrierId)
{
  NS_LOG_FUNCTION (this << durationInSecs << payloadBytes << carrierId);

  /**
   * TODO: the TBTP should hold also the RC_index for each time slot. Here, the RC_index
   * should be passed with txOpportunity to higher layer, so that it knows which RC_index
   * (= queue) to serve.
   */
   
   /**
   * Notify LLC of the Tx opportunity; returns a packet.
   * In addition, the function returns the bytes left after txOpportunity in
   * bytesLeft reference variable.
   */
  uint32_t bytesLeft (0);

  Ptr<Packet> p = m_txOpportunityCallback (payloadBytes, m_macAddress, bytesLeft);
  SatPhy::PacketContainer_t packets;
  packets.push_back (p);

  if ( p )
    {
      // Decrease one tick from time slot duration. This evaluates guard period.
      // If more sophisticated guard period is needed, it is needed done before hand and
      // remove this 'one tick decrease' implementation
      Time duration (Time::FromDouble (durationInSecs, Time::S) - Time (1));
      SendPacket (packets, carrierId, duration);
    }
}


void
SatUtMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this);

  // Hit the trace hooks.  All of these hooks are in the same place in this
  // device because it is so simple, but this is not usually the case in
  // more complicated devices.

  for (SatPhy::PacketContainer_t::iterator i = packets.begin(); i != packets.end(); i++ )
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

      NS_LOG_LOGIC("Packet from " << macTag.GetSourceAddress() << " to " << macTag.GetDestAddress());
      NS_LOG_LOGIC("Receiver " << m_macAddress );

      Mac48Address destAddress = Mac48Address::ConvertFrom(macTag.GetDestAddress());
      if (destAddress == m_macAddress || destAddress.IsBroadcast())
        {
          // Remove control msg tag
          SatControlMsgTag ctrlTag;
          bool cSuccess = (*i)->RemovePacketTag (ctrlTag);

          if (cSuccess)
            {
              SatControlMsgTag::SatControlMsgType_t cType = ctrlTag.GetMsgType ();

              if ( cType != SatControlMsgTag::SAT_NON_CTRL_MSG )
                {
                  // Remove the ctrl tag
                  (*i)->RemovePacketTag (ctrlTag);
                  ReceiveSignalingPacket (*i, cType);
                }
              else
                {
                  NS_FATAL_ERROR ("A control message received with not valid msg type!");
                }
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
SatUtMac::ReceiveSignalingPacket (Ptr<Packet> packet, SatControlMsgTag::SatControlMsgType_t cType)
{
  switch (cType)
  {
    case SatControlMsgTag::SAT_TBTP_CTRL_MSG:
      {
        SatTbtpHeader tbtp;
        if ( packet->RemoveHeader (tbtp) > 0 )
          {
            ScheduleTimeSlots (&tbtp);
          }
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
