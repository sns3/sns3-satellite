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
 */

#include "ns3/gateway-lorawan-mac.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/satellite-lorawan-net-device.h"
#include "ns3/lora-frame-header.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GatewayLorawanMac");

NS_OBJECT_ENSURE_REGISTERED (GatewayLorawanMac);

TypeId
GatewayLorawanMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GatewayLorawanMac")
    .SetParent<LorawanMac> ()
    .AddConstructor<GatewayLorawanMac> ()
    ;
  return tid;
}

GatewayLorawanMac::GatewayLorawanMac ()
{
  NS_FATAL_ERROR ("Default constructor not in use");
}

GatewayLorawanMac::GatewayLorawanMac (uint32_t beamId)
  : LorawanMac (beamId)
{
  NS_LOG_FUNCTION (this);
}

GatewayLorawanMac::~GatewayLorawanMac ()
{
  NS_LOG_FUNCTION (this);
}

void
GatewayLorawanMac::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet);

  // Get DataRate to send this packet with
  // TODO who add it ???? -> GetReplyForDevice from "network status" And add it again here
  // TODO change values in tag
  // TODO only some values of datarate (and BW) -> say it in phy, or change in MAC
  LoraTag tag;
  packet->AddPacketTag (tag);
  tag.SetDataRate (0);
  tag.SetFrequency (1000000000);
  packet->RemovePacketTag (tag);
  uint8_t dataRate = tag.GetDataRate ();
  double frequency = tag.GetFrequency ();
  NS_LOG_DEBUG ("DR: " << (uint32_t) unsigned (dataRate));
  NS_LOG_DEBUG ("SF: " << (uint32_t) unsigned (GetSfFromDataRate (dataRate)));
  NS_LOG_DEBUG ("BW: " << GetBandwidthFromDataRate (dataRate));
  NS_LOG_DEBUG ("Freq: " << frequency << " MHz");
  packet->AddPacketTag (tag);

  // Make sure we can transmit this packet
  // TODO
  /*if (m_channelHelper.GetWaitingTime(CreateObject<LogicalLoraChannel> (frequency)) > Time(0))
    {
      // We cannot send now!
      NS_LOG_WARN ("Trying to send a packet but Duty Cycle won't allow it. Aborting.");
      return;
    }
    */

  LoraTxParameters params;
  params.sf = GetSfFromDataRate (dataRate);
  params.headerDisabled = false;
  params.codingRate = 1;
  params.bandwidthHz = GetBandwidthFromDataRate (dataRate);
  params.nPreamble = 8;
  params.crcEnabled = 1;
  params.lowDataRateOptimizationEnabled = 0;


  // TODO put correct stuff here
  //Ptr<SatWaveform> wf = m_waveformConf->GetWaveform (1);
  SatSignalParameters::txInfo_s txInfo;
  txInfo.modCod = SatEnums::SAT_MODCOD_QPSK_1_TO_2;
  //txInfo.fecBlockSizeInBytes = waveform->GetPayloadInBytes ();
  //txInfo.frameType = SatEnums::UNDEFINED_FRAME;
  //txInfo.waveformId = wf->GetWaveformId ();
  //txInfo.crdsaUniquePacketId = m_crdsaUniquePacketId; // reuse the crdsaUniquePacketId to identify ESSA frames
  txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
  txInfo.sliceId = 0;

  // Get the duration
  Time duration = GetOnAirTime (packet, params);

  // NS_LOG_DEBUG ("Duration: " << duration.GetSeconds ());

  // Find the channel with the desired frequency
  //TODO
  /*double sendingPower = m_channelHelper.GetTxPowerForChannel
      (CreateObject<LogicalLoraChannel> (frequency));

  // Add the event to the channelHelper to keep track of duty cycle
  m_channelHelper.AddEvent (duration, CreateObject<LogicalLoraChannel>
                              (frequency));
  */

  SatMacTag mTag;
  mTag.SetDestAddress (Mac48Address::ConvertFrom (dest));
  mTag.SetSourceAddress (Mac48Address::ConvertFrom (m_device->GetAddress ()));
  packet->AddPacketTag (mTag);

  SatPhy::PacketContainer_t packets;
  packets.push_back (packet);
  uint32_t carrierId = 0;

  // Send the packet to the PHY layer to send it on the channel
  //m_phy->Send (packet, params, frequency, sendingPower);
  m_phy->SendPdu (packets, carrierId, duration, txInfo);

  m_sentNewPacket (packet);
}

bool
GatewayLorawanMac::IsTransmitting (void)
{
  // TODO
  return false;
  // return m_phy->IsTransmitting ();
}

void
GatewayLorawanMac::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this << packets);

  Ptr<Packet> packet;
  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      packet = *i;
      // Make a copy of the packet to work on
      Ptr<Packet> packetCopy = packet->Copy ();

      // Only forward the packet if it's uplink
      LorawanMacHeader macHdr;
      packetCopy->RemoveHeader (macHdr);

      // Add the Lora Frame Header to the packet
      LoraFrameHeader frameHdr;
      packetCopy->RemoveHeader (frameHdr);

      SatMacTag mTag;
      packetCopy->PeekPacketTag (mTag);

      if (macHdr.IsUplink ())
        {
          m_device->GetObject<SatLorawanNetDevice> ()->Receive (packetCopy);

          NS_LOG_DEBUG ("Received packet: " << packet);

          m_receivedPacket (packet);
        }
      else
        {
          NS_LOG_DEBUG ("Not forwarding downlink message to NetDevice");
        }
    }
}

void
GatewayLorawanMac::FailedReception (Ptr<Packet const> packet)
{
  NS_LOG_FUNCTION (this << packet);
}

void
GatewayLorawanMac::TxFinished (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION_NOARGS ();
}

Time
GatewayLorawanMac::GetWaitingTime (double frequency)
{
  NS_LOG_FUNCTION_NOARGS ();

  // TODO
  /*
  return m_channelHelper.GetWaitingTime (CreateObject<LogicalLoraChannel>
                                           (frequency));
                                           */
  return Seconds(0);
}
}
