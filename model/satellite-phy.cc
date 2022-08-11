/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include <ns3/node.h>

#include "satellite-phy.h"
#include "satellite-utils.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-lora-phy-rx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"
#include "satellite-node-info.h"
#include "satellite-enums.h"
#include "satellite-address-tag.h"
#include "satellite-time-tag.h"
#include "satellite-typedefs.h"


NS_LOG_COMPONENT_DEFINE ("SatPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhy);

SatPhy::SatPhy (void)
  : m_beamId (0),
  m_eirpWoGainW (0),
  m_isStatisticsTagsEnabled (false),
  m_lastDelay (0),
  m_lastLinkDelay (0),
  m_rxNoiseTemperatureDbk (0),
  m_rxMaxAntennaGainDb (0),
  m_rxAntennaLossDb (0),
  m_txMaxAntennaGainDb (0),
  m_txMaxPowerDbw (0),
  m_txOutputLossDb (0),
  m_txPointingLossDb (0),
  m_txOboLossDb (0),
  m_txAntennaLossDb (0),
  m_defaultFadingValue (1.0)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatPhy default constructor is not allowed to use");
}

SatPhy::SatPhy (CreateParam_t & params)
  : m_beamId (0),
  m_eirpWoGainW (0),
  m_isStatisticsTagsEnabled (false),
  m_lastDelay (0),
  m_lastLinkDelay (0),
  m_rxNoiseTemperatureDbk (0),
  m_rxMaxAntennaGainDb (0),
  m_rxAntennaLossDb (0),
  m_txMaxAntennaGainDb (0),
  m_txMaxPowerDbw (0),
  m_txOutputLossDb (0),
  m_txPointingLossDb (0),
  m_txOboLossDb (0),
  m_txAntennaLossDb (0),
  m_defaultFadingValue (1.0)
{
  NS_LOG_FUNCTION (this << params.m_beamId);
  ObjectBase::ConstructSelf (AttributeConstructionList ());

  Ptr<MobilityModel> mobility = params.m_device->GetNode ()->GetObject<MobilityModel> ();

  switch (params.m_standard)
    {
      case SatEnums::GEO:
      case SatEnums::DVB_UT:
      case SatEnums::DVB_GW:
      {
        m_phyTx = CreateObject<SatPhyTx> ();
        m_phyRx = CreateObject<SatPhyRx> ();
        break;
      }
      case SatEnums::LORA_UT:
      {
        m_phyTx = CreateObject<SatLoraPhyTx> ();
        m_phyRx = CreateObject<SatLoraPhyRx> ();
        break;
      }
      case SatEnums::LORA_GW:
      {
        m_phyTx = CreateObject<SatLoraPhyTx> ();
        m_phyRx = CreateObject<SatPhyRx> ();
        break;
      }
      default:
        NS_FATAL_ERROR ("Standard not implemented yet: " << params.m_standard);
    }

  m_phyTx->SetChannel (params.m_txCh);
  m_beamId = params.m_beamId;

  params.m_rxCh->AddRx (m_phyRx);
  m_phyRx->SetDevice (params.m_device);
  m_phyTx->SetMobility (mobility);
  m_phyRx->SetMobility (mobility);
}

TypeId
SatPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhy")
    .SetParent<Object> ()
    .AddAttribute ("ReceiveCb", "The receive callback for this phy.",
                   CallbackValue (),
                   MakeCallbackAccessor (&SatPhy::m_rxCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("CnoCb", "The C/N0 info callback for this phy.",
                   CallbackValue (),
                   MakeCallbackAccessor (&SatPhy::m_cnoCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("AverageNormalizedOfferedLoadCallback", "The average offered random access load callback for this phy.",
                   CallbackValue (),
                   MakeCallbackAccessor (&SatPhy::m_avgNormalizedOfferedLoadCallback),
                   MakeCallbackChecker ())
    .AddAttribute ("EnableStatisticsTags",
                   "If true, some tags will be added to each transmitted packet to assist with statistics computation",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatPhy::m_isStatisticsTagsEnabled),
                   MakeBooleanChecker ())
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatPhy::m_packetTrace),
                     "ns3::SatTypedefs::PacketTraceCallback")
    .AddTraceSource ("Rx",
                     "A packet received",
                     MakeTraceSourceAccessor (&SatPhy::m_rxTrace),
                     "ns3::SatTypedefs::PacketSourceAddressCallback")
    .AddTraceSource ("RxDelay",
                     "A packet is received with delay information",
                     MakeTraceSourceAccessor (&SatPhy::m_rxDelayTrace),
                     "ns3::SatTypedefs::PacketDelayAddressCallback")
    .AddTraceSource ("RxLinkDelay",
                     "A packet is received with link delay information",
                     MakeTraceSourceAccessor (&SatPhy::m_rxLinkDelayTrace),
                     "ns3::SatTypedefs::PacketDelayAddressCallback")
    .AddTraceSource ("RxJitter",
                     "A packet is received with jitter information",
                     MakeTraceSourceAccessor (&SatPhy::m_rxJitterTrace),
                     "ns3::SatTypedefs::PacketJitterAddressCallback")
    .AddTraceSource ("RxLinkJitter",
                     "A packet is received with link jitter information",
                     MakeTraceSourceAccessor (&SatPhy::m_rxLinkJitterTrace),
                     "ns3::SatTypedefs::PacketJitterAddressCallback")
  ;
  return tid;
}

TypeId
SatPhy::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
SatPhy::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
SatPhy::Initialize ()
{
  NS_LOG_FUNCTION (this);

  // calculate EIRP without Gain (maximum)
  double eirpWoGainDbw = m_txMaxPowerDbw - m_txOutputLossDb - m_txPointingLossDb - m_txOboLossDb - m_txAntennaLossDb;

  m_eirpWoGainW = SatUtils::DbWToW ( eirpWoGainDbw );

  m_phyTx->SetBeamId (m_beamId);
  m_phyRx->SetBeamId (m_beamId);

  m_phyRx->SetReceiveCallback ( MakeCallback (&SatPhy::Receive, this) );

  if ( m_cnoCallback.IsNull () == false )
    {
      m_phyRx->SetCnoCallback ( MakeCallback (&SatPhy::CnoInfo, this) );
    }

  if ( m_avgNormalizedOfferedLoadCallback.IsNull () == false )
    {
      std::cout << "m_phyRx->SetAverageNormalizedOfferedLoadCallback " << std::endl;
      m_phyRx->SetAverageNormalizedOfferedLoadCallback ( MakeCallback (&SatPhy::AverageNormalizedOfferedRandomAccessLoadInfo, this) );
    }

  m_phyTx->SetMaxAntennaGain_Db (m_txMaxAntennaGainDb);
  m_phyRx->SetMaxAntennaGain_Db (m_rxMaxAntennaGainDb);

  m_phyTx->SetDefaultFadingValue (m_defaultFadingValue);
  m_phyRx->SetDefaultFadingValue (m_defaultFadingValue);

  m_phyRx->SetAntennaLoss_Db (m_rxAntennaLossDb);
}

SatPhy::~SatPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phyTx->DoDispose ();
  m_phyTx = 0;
  m_phyRx->DoDispose ();
  m_phyRx = 0;

  Object::DoDispose ();
}

void
SatPhy::SetTxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  NS_LOG_FUNCTION (this);
  m_phyTx->SetAntennaGainPattern (agp);
}

void
SatPhy::SetRxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp)
{
  NS_LOG_FUNCTION (this);
  m_phyRx->SetAntennaGainPattern (agp);
}

void
SatPhy::ConfigureRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf, Ptr<SatSuperframeConf> superFrameConf)
{
  NS_LOG_FUNCTION (this);
  m_phyRx->ConfigurePhyRxCarriers (carrierConf, superFrameConf);
}

void
SatPhy::SetRxFadingContainer (Ptr<SatBaseFading> fadingContainer)
{
  NS_LOG_FUNCTION (this);

  m_phyRx->SetFadingContainer (fadingContainer);
}

void
SatPhy::SetTxFadingContainer (Ptr<SatBaseFading> fadingContainer)
{
  NS_LOG_FUNCTION (this);

  m_phyTx->SetFadingContainer (fadingContainer);
}

void
SatPhy::SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo)
{
  NS_LOG_FUNCTION (this << nodeInfo);
  m_nodeInfo = nodeInfo;
  m_phyRx->SetNodeInfo (nodeInfo);
}

void
SatPhy::BeginEndScheduling ()
{
  NS_LOG_FUNCTION (this);
  m_phyRx->BeginEndScheduling ();
}

Ptr<SatPhyTx>
SatPhy::GetPhyTx () const
{
  NS_LOG_FUNCTION (this);
  return m_phyTx;
}

Ptr<SatPhyRx>
SatPhy::GetPhyRx () const
{
  NS_LOG_FUNCTION (this);
  return m_phyRx;
}

void
SatPhy::SetPhyTx (Ptr<SatPhyTx> phyTx)
{
  NS_LOG_FUNCTION (this << phyTx);
  m_phyTx = phyTx;
}

void
SatPhy::SetPhyRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);
  m_phyRx = phyRx;
}

Ptr<SatChannel>
SatPhy::GetTxChannel ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phyTx);

  return m_phyTx->GetChannel ();
}

void
SatPhy::SendPdu (PacketContainer_t p, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s txInfo)
{
  NS_LOG_FUNCTION (this << carrierId << duration);
  NS_LOG_INFO ("Sending a packet with carrierId: " << carrierId << " duration: " << duration);

  // Add a SatPhyTimeTag tag for packet delay computation at the receiver end.
  SetTimeTag (p);

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld = GetSatLinkTxDir ();

  m_packetTrace (Simulator::Now (),
                 SatEnums::PACKET_SENT,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 ld,
                 SatUtils::GetPacketInfo (p));

  // Create a new SatSignalParameters related to this packet transmission
  Ptr<SatSignalParameters> txParams = Create<SatSignalParameters> ();
  txParams->m_duration = duration;
  txParams->m_phyTx = m_phyTx;
  txParams->m_packetsInBurst = p;
  txParams->m_beamId = m_beamId;
  txParams->m_carrierId = carrierId;
  txParams->m_txPower_W = m_eirpWoGainW;
  txParams->m_txInfo.modCod = txInfo.modCod;
  txParams->m_txInfo.sliceId = txInfo.sliceId;
  txParams->m_txInfo.fecBlockSizeInBytes = txInfo.fecBlockSizeInBytes;
  txParams->m_txInfo.frameType = txInfo.frameType;
  txParams->m_txInfo.waveformId = txInfo.waveformId;
  txParams->m_txInfo.packetType = txInfo.packetType;
  txParams->m_txInfo.crdsaUniquePacketId = txInfo.crdsaUniquePacketId;

  m_phyTx->StartTx (txParams);
}

void
SatPhy::SendPduWithParams (Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << txParams);
  NS_ASSERT (false);

  /**
   * This method is not meant to be used in this class. It
   * is overriden in the inherited classes.
   */
}

void
SatPhy::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  m_beamId = beamId;
  m_phyTx->SetBeamId (beamId);
  m_phyRx->SetBeamId (beamId);
}

void
SatPhy::SetTimeTag (SatPhy::PacketContainer_t packets)
{
  if (m_isStatisticsTagsEnabled)
    {
      for (PacketContainer_t::const_iterator it = packets.begin (); it != packets.end (); ++it)
        {
          SatPhyTimeTag timeTag;
          if (!(*it)->PeekPacketTag (timeTag))
            {
              (*it)->AddPacketTag (SatPhyTimeTag (Simulator::Now ()));
            }

          (*it)->AddPacketTag (SatPhyLinkTimeTag (Simulator::Now ()));
        }
    }
}

SatEnums::SatLinkDir_t
SatPhy::GetSatLinkTxDir ()
{
  return SatEnums::LD_UNDEFINED;
}

SatEnums::SatLinkDir_t
SatPhy::GetSatLinkRxDir ()
{
  return SatEnums::LD_UNDEFINED;
}

void
SatPhy::RxTraces (SatPhy::PacketContainer_t packets)
{
  NS_LOG_FUNCTION (this);

  if (m_isStatisticsTagsEnabled)
    {
      SatSignalParameters::PacketsInBurst_t::iterator it1;
      for (it1 = packets.begin ();
           it1 != packets.end (); ++it1)
        {
          Address addr; // invalid address.
          bool isTaggedWithAddress = false;
          ByteTagIterator it2 = (*it1)->GetByteTagIterator ();

          while (!isTaggedWithAddress && it2.HasNext ())
            {
              ByteTagIterator::Item item = it2.Next ();

              if (item.GetTypeId () == SatAddressTag::GetTypeId ())
                {
                  NS_LOG_DEBUG (this << " contains a SatAddressTag tag:"
                                     << " start=" << item.GetStart ()
                                     << " end=" << item.GetEnd ());
                  SatAddressTag addrTag;
                  item.GetTag (addrTag);
                  addr = addrTag.GetSourceAddress ();
                  isTaggedWithAddress = true; // this will exit the while loop.
                }
            }

          m_rxTrace (*it1, addr);

          SatPhyLinkTimeTag linkTimeTag;
          if ((*it1)->RemovePacketTag (linkTimeTag))
            {
              NS_LOG_DEBUG (this << " contains a SatPhyLinkTimeTag tag");
              Time delay = Simulator::Now () - linkTimeTag.GetSenderLinkTimestamp ();
              m_rxLinkDelayTrace (delay, addr);
              if (m_lastLinkDelay.IsZero() == false)
                {
                  Time jitter = Abs (delay - m_lastLinkDelay);
                  m_rxLinkJitterTrace (jitter, addr);
                }
              m_lastLinkDelay = delay;
            }

          SatPhyTimeTag timeTag;
          if ((*it1)->RemovePacketTag (timeTag))
            {
              NS_LOG_DEBUG (this << " contains a SatPhyTimeTag tag");
              Time delay = Simulator::Now () - timeTag.GetSenderTimestamp ();
              m_rxDelayTrace (delay, addr);
              if (m_lastDelay.IsZero() == false)
                {
                  Time jitter = Abs (delay - m_lastDelay);
                  m_rxJitterTrace (jitter, addr);
                }
              m_lastDelay = delay;
            }

        } // end of `for (it1 = rxParams->m_packetsInBurst)`

    } // end of `if (m_isStatisticsTagsEnabled)`
}

void
SatPhy::Receive (Ptr<SatSignalParameters> rxParams, bool phyError)
{
  NS_LOG_FUNCTION (this << rxParams << phyError);

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld = GetSatLinkRxDir ();

  SatEnums::SatPacketEvent_t event = (phyError) ? SatEnums::PACKET_DROP : SatEnums::PACKET_RECV;

  m_packetTrace (Simulator::Now (),
                 event,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 ld,
                 SatUtils::GetPacketInfo (rxParams->m_packetsInBurst));

  if (phyError)
    {
      // If there was a PHY error, the packet is dropped here.
      NS_LOG_INFO (this << " dropped " << rxParams->m_packetsInBurst.size ()
                        << " packets because of PHY error.");
    }
  else
    {
      // Invoke the `Rx` and `RxDelay` trace sources.
      RxTraces (rxParams->m_packetsInBurst);

      // Pass the packet to the upper layer.
      m_rxCallback (rxParams->m_packetsInBurst, rxParams);

    } // end of else of `if (phyError)`

}

void
SatPhy::CnoInfo (uint32_t beamId, Address source, Address dest, double cno)
{
  NS_LOG_FUNCTION (this << beamId << source << cno);
  m_cnoCallback ( beamId, source, dest, cno);
}

void
SatPhy::AverageNormalizedOfferedRandomAccessLoadInfo (uint32_t beamId, uint32_t carrierId, uint8_t allocationChannelId, double averageNormalizedOfferedLoad)
{
  NS_LOG_FUNCTION (this << beamId << carrierId << allocationChannelId << averageNormalizedOfferedLoad);
  m_avgNormalizedOfferedLoadCallback (beamId, carrierId, allocationChannelId, averageNormalizedOfferedLoad);
}


void
SatPhy::SetChannelPairGetterCallback (SatPhy::ChannelPairGetterCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_retrieveChannelPair = cb;
}


} // namespace ns3
