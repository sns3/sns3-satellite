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
 *
 * Modified by: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include <ns3/log.h>

#include "satellite-bbframe-conf.h"
#include "satellite-lorawan-net-device.h"
#include "lorawan-mac-gateway.h"
#include "lorawan-mac-header.h"
#include "lora-frame-header.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LorawanMacGateway");

NS_OBJECT_ENSURE_REGISTERED (LorawanMacGateway);

TypeId
LorawanMacGateway::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LorawanMacGateway")
    .SetParent<LorawanMac> ()
    .AddConstructor<LorawanMacGateway> ()
    ;
  return tid;
}

LorawanMacGateway::LorawanMacGateway ()
{
  NS_FATAL_ERROR ("Default constructor not in use");
}

LorawanMacGateway::LorawanMacGateway (uint32_t beamId)
  : LorawanMac (beamId)
{
  NS_LOG_FUNCTION (this);
}

LorawanMacGateway::~LorawanMacGateway ()
{
  NS_LOG_FUNCTION (this);
}

void
LorawanMacGateway::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Get DataRate to send this packet with
  LoraTag tag;
  packet->RemovePacketTag (tag);
  uint8_t modcod = tag.GetModcod ();
  uint8_t dataRate = tag.GetDataRate ();
  double frequency = tag.GetFrequency ();
  NS_LOG_DEBUG ("DR: " << (uint32_t) unsigned (dataRate));
  NS_LOG_DEBUG ("SF: " << (uint32_t) unsigned (GetSfFromDataRate (dataRate)));
  NS_LOG_DEBUG ("BW: " << GetBandwidthFromDataRate (dataRate));
  NS_LOG_DEBUG ("Freq: " << frequency << " MHz");
  packet->AddPacketTag (tag);

  if (m_isStatisticsTagsEnabled)
    {
      packet->AddPacketTag (SatMacTimeTag (Simulator::Now ()));

      // Add a SatAddressTag tag with this device's address as the source address.
      packet->AddByteTag (SatAddressTag (m_nodeInfo->GetMacAddress ()));

      // Add a SatDevTimeTag tag for packet delay computation at the receiver end.
      SatDevTimeTag satDevTag;
      packet->RemovePacketTag (satDevTag);
      packet->AddPacketTag (SatDevTimeTag (Simulator::Now ()));
    }

  // Make sure we can transmit this packet
  if (m_channelHelper.GetWaitingTime(CreateObject<LoraLogicalChannel> (frequency)) > Time(0))
    {
      // We cannot send now!
      NS_LOG_WARN ("Trying to send a packet but Duty Cycle won't allow it. Aborting.");
      return;
    }

  if (IsTransmitting ())
    {
      // Gateway already transmitting!
      NS_LOG_WARN ("Gateway is already transmitting. Aborting.");
      return;
    }

  LoraTxParameters params;
  params.sf = GetSfFromDataRate (dataRate);
  params.headerDisabled = false;
  params.codingRate = SatUtils::GetCodingRate ((SatEnums::SatModcod_t) modcod);
  params.bandwidthHz = GetBandwidthFromDataRate (dataRate);
  params.nPreamble = 8;
  params.crcEnabled = 1;
  params.lowDataRateOptimizationEnabled = 0;

  SatSignalParameters::txInfo_s txInfo;
  txInfo.packetType = SatEnums::PACKET_TYPE_DEDICATED_ACCESS;
  txInfo.frameType = SatEnums::NORMAL_FRAME;
  txInfo.modCod = (SatEnums::SatModcod_t) modcod;
  txInfo.sliceId = 0;
  txInfo.waveformId = 0;

  // Get the duration
  Time duration = GetOnAirTime (packet, params);

  NS_LOG_DEBUG ("Duration: " << duration.GetSeconds ());

  // Add the event to the channelHelper to keep track of duty cycle
  m_channelHelper.AddEvent (duration, CreateObject<LoraLogicalChannel> (frequency));

  SatMacTag mTag;
  mTag.SetDestAddress (Mac48Address::GetBroadcast ());
  mTag.SetSourceAddress (Mac48Address::ConvertFrom (m_device->GetAddress ()));
  packet->AddPacketTag (mTag);

  SatAddressE2ETag addressE2ETag;
  addressE2ETag.SetE2EDestAddress (Mac48Address::GetBroadcast ());
  addressE2ETag.SetE2ESourceAddress (Mac48Address::ConvertFrom (m_device->GetAddress ()));
  packet->AddPacketTag (addressE2ETag);

  SatPhy::PacketContainer_t packets;
  packets.push_back (packet);
  uint32_t carrierId = 0;

  // Send the packet to the PHY layer to send it on the channel
  m_phy->SendPdu (packets, carrierId, duration, txInfo);

  m_sentNewPacket (packet);
}

bool
LorawanMacGateway::IsTransmitting (void)
{
  return DynamicCast<SatLoraPhyTx> (m_phy->GetPhyTx ())->IsTransmitting ();
}

void
LorawanMacGateway::Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/)
{
  NS_LOG_FUNCTION (this << packets);

  // Invoke the `Rx` and `RxDelay` trace sources.
  RxTraces (packets);

  Ptr<Packet> packet;
  for (SatPhy::PacketContainer_t::iterator i = packets.begin (); i != packets.end (); i++ )
    {
      packet = *i;
      // Make a copy of the packet to work on
      Ptr<Packet> packetCopy = packet->Copy ();

      SatMacTag mTag;
      packetCopy->RemovePacketTag (mTag);

      // Only forward the packet if it's uplink
      LorawanMacHeader macHdr;
      packetCopy->PeekHeader (macHdr);

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
LorawanMacGateway::FailedReception (Ptr<Packet const> packet)
{
  NS_LOG_FUNCTION (this << packet);
}

void
LorawanMacGateway::TxFinished ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

Time
LorawanMacGateway::GetWaitingTime (double frequency)
{
  NS_LOG_FUNCTION (this << frequency);

  return m_channelHelper.GetWaitingTime (CreateObject<LoraLogicalChannel> (frequency));
}
}
