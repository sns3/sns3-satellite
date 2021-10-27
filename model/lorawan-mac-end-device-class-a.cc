/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 *              qiuyukang <b612n@qq.com>
 *              Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <algorithm>

#include <ns3/log.h>
#include <ns3/pointer.h>

#include <ns3/satellite-phy.h>
#include <ns3/satellite-lorawan-net-device.h>

#include "ns3/lora-tag.h"
#include <ns3/lorawan-mac-end-device-class-a.h>
#include <ns3/lorawan-mac-end-device.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LorawanMacEndDeviceClassA");

NS_OBJECT_ENSURE_REGISTERED (LorawanMacEndDeviceClassA);

TypeId
LorawanMacEndDeviceClassA::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::LorawanMacEndDeviceClassA")
  .SetParent<LorawanMacEndDevice> ()
    .AddAttribute ("FirstWindowDelay",
                   "Time to wait between end of message transmission and opening of first reception window",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&LorawanMacEndDeviceClassA::m_firstWindowDelay),
                   MakeTimeChecker ())
    .AddAttribute ("SecondWindowDelay",
                   "Time to wait between end of message transmission and opening of second reception window",
                   TimeValue (Seconds (2)),
                   MakeTimeAccessor (&LorawanMacEndDeviceClassA::m_secondWindowDelay),
                   MakeTimeChecker ())
    .AddAttribute ("FirstWindowDuration",
                   "Duration of first reception window",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&LorawanMacEndDeviceClassA::m_firstWindowDuration),
                   MakeTimeChecker ())
    .AddAttribute ("SecondWindowDuration",
                   "Duration of second reception window",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&LorawanMacEndDeviceClassA::m_secondWindowDuration),
                   MakeTimeChecker ())
  .AddConstructor<LorawanMacEndDeviceClassA> ();
return tid;
}

LorawanMacEndDeviceClassA::LorawanMacEndDeviceClassA ()
{
  NS_FATAL_ERROR ("Default constructor not in use");
}

LorawanMacEndDeviceClassA::LorawanMacEndDeviceClassA (uint32_t beamId, Ptr<SatSuperframeSeq> seq)
  : LorawanMacEndDevice (beamId),
    m_superframeSeq (seq),
    m_firstWindowDelay (Seconds (1)),
    m_secondWindowDelay (Seconds (2)),
    m_firstWindowDuration (MilliSeconds (100)),
    m_secondWindowDuration (MilliSeconds (100)),
    m_rx1DrOffset (0)
{
  NS_LOG_FUNCTION (this);

  // Void the two receiveWindow events
  m_closeFirstWindow = EventId ();
  m_closeFirstWindow.Cancel ();
  m_closeSecondWindow = EventId ();
  m_closeSecondWindow.Cancel ();
  m_secondReceiveWindow = EventId ();
  m_secondReceiveWindow.Cancel ();
}

LorawanMacEndDeviceClassA::~LorawanMacEndDeviceClassA ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

/////////////////////
// Sending methods //
/////////////////////

void
LorawanMacEndDeviceClassA::SendToPhy (Ptr<Packet> packetToSend)
{
  /////////////////////////////////////////////////////////
  // Add headers, prepare TX parameters and send the packet
  /////////////////////////////////////////////////////////
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("PacketToSend: " << packetToSend);

  // Data Rate Adaptation as in LoRaWAN specification, V1.0.2 (2016)
  if (m_enableDRAdapt && (m_dataRate > 0)
      && (m_retxParams.retxLeft < m_maxNumbTx)
      && (m_retxParams.retxLeft % 2 == 0) )
    {
      m_txPower = 14; // Reset transmission power
      m_dataRate = m_dataRate - 1;
    }

  if (m_isStatisticsTagsEnabled)
    {
      // Add a SatAddressTag tag with this device's address as the source address.
      packetToSend->AddByteTag (SatAddressTag (m_nodeInfo->GetMacAddress ()));

      // Add a SatDevTimeTag tag for packet delay computation at the receiver end.
      SatDevTimeTag satDevTag;
      packetToSend->RemovePacketTag (satDevTag);
      packetToSend->AddPacketTag (SatDevTimeTag (Simulator::Now ()));
    }

  m_phyRx->SwitchToTx ();

  uint32_t allocationChannel = 0; // TODO is really zero here ?
  Ptr<SatSuperframeConf> superframeConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);
  uint8_t frameId = superframeConf->GetRaChannelFrameId (allocationChannel);
  Ptr<SatFrameConf> frameConf = superframeConf->GetFrameConf (frameId);
  Ptr<SatTimeSlotConf> timeSlotConf = frameConf->GetTimeSlotConf (0);
  Ptr<SatWaveform> wf = m_superframeSeq->GetWaveformConf ()->GetWaveform (timeSlotConf->GetWaveFormId ());

  SatSignalParameters::txInfo_s txInfo;
  txInfo.modCod = wf->GetModCod ();
  txInfo.fecBlockSizeInBytes = wf->GetPayloadInBytes ();
  txInfo.packetType = m_packetType;
  txInfo.frameType = SatEnums::UNDEFINED_FRAME;
  txInfo.waveformId = wf->GetWaveformId ();
  //txInfo.crdsaUniquePacketId = m_crdsaUniquePacketId; // reuse the crdsaUniquePacketId to identify ESSA frames

  // Craft LoraTxParameters object
  LoraTxParameters params;
  params.sf = GetSfFromDataRate (m_dataRate);
  params.headerDisabled = m_headerDisabled;
  params.codingRate = SatUtils::GetCodingRate (wf->GetModCod ());
  params.bandwidthHz = GetBandwidthFromDataRate (m_dataRate);
  params.nPreamble = m_nPreambleSymbols;
  params.crcEnabled = 1;
  params.lowDataRateOptimizationEnabled = 0;

  // Pick a channel on which to transmit the packet
  Ptr<LoraLogicalChannel> txChannel = GetChannelForTx ();
  double frequency = txChannel->GetFrequency ();
  LoraTag tag;
  packetToSend->RemovePacketTag (tag);
  tag.SetFrequency (frequency);
  tag.SetDataRate (m_dataRate);
  tag.SetModcod (wf->GetModCod ());
  packetToSend->AddPacketTag (tag);

  SatMacTag mTag;
  packetToSend->RemovePacketTag (mTag);
  mTag.SetDestAddress (m_gwAddress);
  mTag.SetSourceAddress (Mac48Address::ConvertFrom (m_device->GetAddress ()));
  packetToSend->AddPacketTag (mTag);

  SatPhy::PacketContainer_t packets;
  packets.push_back (packetToSend);

  uint32_t carrierId = 0;

  // Compute packet duration
  Time duration = GetOnAirTime (packetToSend, params);

  m_phy->SendPdu (packets, carrierId, duration, txInfo);

  NS_LOG_DEBUG ("PacketToSend: " << packetToSend);

  //////////////////////////////////////////////
  // Register packet transmission for duty cycle
  //////////////////////////////////////////////

  // Register the sent packet into the DutyCycleHelper
  m_channelHelper.AddEvent (duration, txChannel);

  //////////////////////////////
  // Prepare for the downlink //
  //////////////////////////////

  // Switch the PHY to the channel so that it will listen here for downlink
  m_phyRx->SetFrequency (txChannel->GetFrequency ());

  // Instruct the PHY on the right Spreading Factor to listen for during the window
  // create a SetReplyDataRate function?
  uint8_t replyDataRate = GetFirstReceiveWindowDataRate ();
  NS_LOG_DEBUG ("m_dataRate: " << unsigned (m_dataRate) <<
                ", m_rx1DrOffset: " << unsigned (m_rx1DrOffset) <<
                ", replyDataRate: " << unsigned (replyDataRate) << ".");

  m_phyRx->SetSpreadingFactor (GetSfFromDataRate (replyDataRate));
}

//////////////////////////
//  Receiving methods   //
//////////////////////////

void
LorawanMacEndDeviceClassA::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // We add good address and not broadcast for traces
  SatMacTag macTag;
  packet->RemovePacketTag (macTag);
  macTag.SetDestAddress (m_nodeInfo->GetMacAddress ());
  packet->AddPacketTag (macTag);

  SatPhy::PacketContainer_t packets;
  packets.push_back (packet);
  RxTraces (packets);

  m_phyRx->SwitchToStandby ();

  // Work on a copy of the packet
  Ptr<Packet> packetCopy = packet->Copy ();

  SatMacTag mTag;
  packetCopy->RemovePacketTag (mTag);

  // Remove the Mac Header to get some information
  LorawanMacHeader mHdr;
  packetCopy->RemoveHeader (mHdr);

  NS_LOG_DEBUG ("Mac Header: " << mHdr);

  // Only keep analyzing the packet if it's downlink
  if (!mHdr.IsUplink ())
    {
      NS_LOG_INFO ("Found a downlink packet.");

      // Remove the Frame Header
      LoraFrameHeader fHdr;
      fHdr.SetAsDownlink ();
      packetCopy->RemoveHeader (fHdr);

      NS_LOG_DEBUG ("Frame Header: " << fHdr);

      // Determine whether this packet is for us
      bool messageForUs = (m_address == fHdr.GetAddress ());

      if (messageForUs)
        {
          NS_LOG_INFO ("The message is for us!");

          // If it exists, cancel the second receive window event
          // THIS WILL BE GetReceiveWindow()
          Simulator::Cancel (m_secondReceiveWindow);

          // Parse the MAC commands
          ParseCommands (fHdr);

          //m_device->GetObject<SatLorawanNetDevice> ()->Receive (packetCopy);

          // Call the trace source
          m_receivedPacket (packet);
        }
      else
        {
          NS_LOG_DEBUG ("The message is intended for another recipient.");

          // In this case, we are either receiving in the first receive window
          // and finishing reception inside the second one, or receiving a
          // packet in the second receive window and finding out, after the
          // fact, that the packet is not for us. In either case, if we no
          // longer have any retransmissions left, we declare failure.
          if (m_retxParams.waitingAck && m_secondReceiveWindow.IsExpired ())
            {
              if (m_retxParams.retxLeft == 0)
                {
                  uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
                  m_requiredTxCallback (txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
                  NS_LOG_DEBUG ("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");

                  // Reset retransmission parameters
                  resetRetransmissionParameters ();
                }
              else       // Reschedule
                {
                  this->Send (m_retxParams.packet);
                  NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
                }
            }
        }
    }
  else if (m_retxParams.waitingAck && m_secondReceiveWindow.IsExpired ())
    {
      NS_LOG_INFO ("The packet we are receiving is in uplink.");
      if (m_retxParams.retxLeft > 0)
        {
          this->Send (m_retxParams.packet);
          NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
        }
      else
        {
          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback (txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
          NS_LOG_DEBUG ("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");

          // Reset retransmission parameters
          resetRetransmissionParameters ();
        }
    }

  m_phyRx->SwitchToSleep ();
}

void
LorawanMacEndDeviceClassA::FailedReception (Ptr<Packet const> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Switch to sleep after a failed reception
  m_phyRx->SwitchToSleep ();

  if (m_secondReceiveWindow.IsExpired () && m_retxParams.waitingAck)
    {
      if (m_retxParams.retxLeft > 0)
        {
          this->Send (m_retxParams.packet);
          NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
        }
      else
        {
          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback (txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
          NS_LOG_DEBUG ("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");

          // Reset retransmission parameters
          resetRetransmissionParameters ();
        }
    }
}

void
LorawanMacEndDeviceClassA::TxFinished ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_phyRx->SwitchToStandby ();

  // Schedule the opening of the first receive window
  Simulator::Schedule (m_firstWindowDelay, &LorawanMacEndDeviceClassA::OpenFirstReceiveWindow, this);

  // Schedule the opening of the second receive window
  m_secondReceiveWindow = Simulator::Schedule (m_secondWindowDelay, &LorawanMacEndDeviceClassA::OpenSecondReceiveWindow, this);

  // Switch the PHY to sleep
  m_phyRx->SwitchToSleep ();
}

void
LorawanMacEndDeviceClassA::SetRaModel (SatEnums::RandomAccessModel_t randomAccessModel)
{
  switch(randomAccessModel)
  {
    case SatEnums::RA_MODEL_OFF:
    {
      m_packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
      break;
    }
    case SatEnums::RA_MODEL_SLOTTED_ALOHA:
    {
      m_packetType = SatEnums::PACKET_TYPE_SLOTTED_ALOHA;
      break;
    }
    case SatEnums::RA_MODEL_CRDSA:
    case SatEnums::RA_MODEL_RCS2_SPECIFICATION:
    case SatEnums::RA_MODEL_MARSALA:
    {
      m_packetType = SatEnums::PACKET_TYPE_CRDSA;
      break;
    }
    case SatEnums::RA_MODEL_ESSA:
    {
      m_packetType = SatEnums::PACKET_TYPE_ESSA;
      break;
    }
  }
}

void
LorawanMacEndDeviceClassA::OpenFirstReceiveWindow (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Set Phy in Standby mode
  m_phyRx->SwitchToStandby ();

  // Schedule return to sleep after "at least the time required by the end
  // device's radio transceiver to effectively detect a downlink preamble"
  // (LoraWAN specification)
  m_closeFirstWindow = Simulator::Schedule (m_firstWindowDuration, &LorawanMacEndDeviceClassA::CloseFirstReceiveWindow, this);

}

void
LorawanMacEndDeviceClassA::CloseFirstReceiveWindow (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Check the Phy layer's state:
  // - RX -> We are receiving a preamble.
  // - STANDBY -> Nothing was received.
  // - SLEEP -> We have received a packet.
  // We should never be in TX or SLEEP mode at this point
  switch (m_phyRx->GetState ())
    {
    case SatLoraPhyRx::TX:
      NS_ABORT_MSG ("PHY was in TX mode when attempting to " <<
                    "close a receive window.");
      break;
    case SatLoraPhyRx::RX:
      // PHY is receiving: let it finish. The Receive method will switch it back to SLEEP.
      break;
    case SatLoraPhyRx::SLEEP:
      // PHY has received, and the MAC's Receive already put the device to sleep
      break;
    case SatLoraPhyRx::STANDBY:
      // Turn PHY layer to SLEEP
      m_phyRx->SwitchToSleep ();
      break;
    }
}

void
LorawanMacEndDeviceClassA::OpenSecondReceiveWindow (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Check for receiver status: if it's locked on a packet, don't open this
  // window at all.
  if (m_phyRx->GetState () == SatLoraPhyRx::RX)
    {
      NS_LOG_INFO ("Won't open second receive window since we are in RX mode.");
      return;
    }

  // Set Phy in Standby mode
  m_phyRx->SwitchToStandby ();

  // Switch to appropriate channel and data rate
  NS_LOG_INFO ("Using parameters: " << m_secondReceiveWindowFrequency << "Hz, DR" << unsigned(m_secondReceiveWindowDataRate));

  m_phyRx->SetFrequency (m_secondReceiveWindowFrequency);
  m_phyRx->SetSpreadingFactor (GetSfFromDataRate (m_secondReceiveWindowDataRate));

  // Schedule return to sleep after "at least the time required by the end
  // device's radio transceiver to effectively detect a downlink preamble"
  // (LoraWAN specification)
  m_closeSecondWindow = Simulator::Schedule (m_secondWindowDuration, &LorawanMacEndDeviceClassA::CloseSecondReceiveWindow, this);
}

void
LorawanMacEndDeviceClassA::CloseSecondReceiveWindow (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // NS_ASSERT (m_phyRx->m_state != SatLoraPhyRx::TX &&
  // m_phyRx->m_state != SatLoraPhyRx::SLEEP);

  // Check the Phy layer's state:
  // - RX -> We have received a preamble.
  // - STANDBY -> Nothing was detected.
  switch (m_phyRx->GetState ())
    {
    case SatLoraPhyRx::TX:
      break;
    case SatLoraPhyRx::SLEEP:
      break;
    case SatLoraPhyRx::RX:
      // PHY is receiving: let it finish
      NS_LOG_DEBUG ("PHY is receiving: Receive will handle the result.");
      return;
    case SatLoraPhyRx::STANDBY:
      // Turn PHY layer to sleep
      m_phyRx->SwitchToSleep ();
      break;
    }

  if (m_retxParams.waitingAck)
    {
      NS_LOG_DEBUG ("No reception initiated by PHY: rescheduling transmission.");
      if (m_retxParams.retxLeft > 0 )
        {
          NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) << " retransmissions left: rescheduling transmission.");
          this->Send (m_retxParams.packet);
        }

      else if (m_retxParams.retxLeft == 0 && m_phyRx->GetState () != SatLoraPhyRx::RX)
        {
          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback (txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
          NS_LOG_DEBUG ("Failure: no more retransmissions left. Used " << unsigned(txs) << " transmissions.");

          // Reset retransmission parameters
          resetRetransmissionParameters ();
        }

      else
        {
          NS_ABORT_MSG ("The number of retransmissions left is negative ! ");
        }
    }
  else
    {
      uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft );
      m_requiredTxCallback (txs, true, m_retxParams.firstAttempt, m_retxParams.packet);
      NS_LOG_INFO ("We have " << unsigned(m_retxParams.retxLeft) <<
                   " transmissions left. We were not transmitting confirmed messages.");

      // Reset retransmission parameters
      resetRetransmissionParameters ();
    }
}

/////////////////////////
// Getters and Setters //
/////////////////////////

Time
LorawanMacEndDeviceClassA::GetNextClassTransmissionDelay (Time waitingTime)
{
  NS_LOG_FUNCTION_NOARGS ();

  // This is a new packet from APP; it can not be sent until the end of the
  // second receive window (if the second recieve window has not closed yet)
  if (!m_retxParams.waitingAck)
    {
      if (!m_closeFirstWindow.IsExpired () ||
          !m_closeSecondWindow.IsExpired () ||
          !m_secondReceiveWindow.IsExpired () )
        {
          NS_LOG_WARN ("Attempting to send when there are receive windows:" <<
                       " Transmission postponed.");
          // Compute the duration of a single symbol for the second receive window DR
          double tSym = pow (2, GetSfFromDataRate (GetSecondReceiveWindowDataRate ())) / GetBandwidthFromDataRate (GetSecondReceiveWindowDataRate ());
          // Compute the closing time of the second receive window
          Time endSecondRxWindow = Time(m_secondReceiveWindow.GetTs()) + Seconds (m_receiveWindowDurationInSymbols*tSym);

          NS_LOG_DEBUG("Duration until endSecondRxWindow for new transmission:" << (endSecondRxWindow - Simulator::Now()).GetSeconds());
          waitingTime = std::max (waitingTime, endSecondRxWindow - Simulator::Now());
        }
    }
  // This is a retransmitted packet, it can not be sent until the end of
  // ACK_TIMEOUT (this timer starts when the second receive window was open)
  else
    {
      double ack_timeout = m_uniformRV->GetValue (1,3);
      // Compute the duration until ACK_TIMEOUT (It may be a negative number, but it doesn't matter.)
      Time retransmitWaitingTime = Time(m_secondReceiveWindow.GetTs()) - Simulator::Now() + Seconds (ack_timeout);

      NS_LOG_DEBUG("ack_timeout:" << ack_timeout <<
                   " retransmitWaitingTime:" << retransmitWaitingTime.GetSeconds());
      waitingTime = std::max (waitingTime, retransmitWaitingTime);
    }

  return waitingTime;
}

uint8_t
LorawanMacEndDeviceClassA::GetFirstReceiveWindowDataRate (void)
{
  return m_replyDataRateMatrix.at (m_dataRate).at (m_rx1DrOffset);
}

void
LorawanMacEndDeviceClassA::SetSecondReceiveWindowDataRate (uint8_t dataRate)
{
  m_secondReceiveWindowDataRate = dataRate;
}

uint8_t
LorawanMacEndDeviceClassA::GetSecondReceiveWindowDataRate (void)
{
  return m_secondReceiveWindowDataRate;
}

void
LorawanMacEndDeviceClassA::SetSecondReceiveWindowFrequency (double frequencyMHz)
{
  m_secondReceiveWindowFrequency = frequencyMHz;
}

double
LorawanMacEndDeviceClassA::GetSecondReceiveWindowFrequency (void)
{
  return m_secondReceiveWindowFrequency;
}

/////////////////////////
// MAC command methods //
/////////////////////////

void
LorawanMacEndDeviceClassA::OnRxClassParamSetupReq (Ptr<RxParamSetupReq> rxParamSetupReq)
{
  NS_LOG_FUNCTION (this << rxParamSetupReq);

  bool offsetOk = true;
  bool dataRateOk = true;

  uint8_t rx1DrOffset = rxParamSetupReq->GetRx1DrOffset ();
  uint8_t rx2DataRate = rxParamSetupReq->GetRx2DataRate ();
  double frequency = rxParamSetupReq->GetFrequency ();

  NS_LOG_FUNCTION (this << unsigned (rx1DrOffset) << unsigned (rx2DataRate) << frequency);

  // Check that the desired offset is valid
  if ( !(0 <= rx1DrOffset && rx1DrOffset <= 5))
    {
      offsetOk = false;
    }

  // Check that the desired data rate is valid
  if (GetSfFromDataRate (rx2DataRate) == 0 || GetBandwidthFromDataRate (rx2DataRate) == 0)
    {
      dataRateOk = false;
    }

  // For now, don't check for validity of frequency
  m_secondReceiveWindowDataRate = rx2DataRate;
  m_rx1DrOffset = rx1DrOffset;
  m_secondReceiveWindowFrequency = frequency;

  // Craft a RxParamSetupAns as response
  NS_LOG_INFO ("Adding RxParamSetupAns reply");
  m_macCommandList.push_back (CreateObject<RxParamSetupAns> (offsetOk, dataRateOk, true));

}

} /* namespace ns3 */
