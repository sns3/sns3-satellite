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
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include <ns3/node.h>

#include "satellite-phy.h"
#include <ns3/satellite-utils.h>
#include <ns3/satellite-phy-rx.h>
#include <ns3/satellite-phy-tx.h>
#include <ns3/satellite-channel.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-signal-parameters.h>
#include <ns3/satellite-node-info.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-address-tag.h>


NS_LOG_COMPONENT_DEFINE ("SatPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhy);

SatPhy::SatPhy (void)
  : m_eirpWoGainW (0),
    m_rxMaxAntennaGainDb (0),
    m_rxAntennaLossDb (0),
    m_txMaxAntennaGainDb (0),
    m_txMaxPowerDbw (0),
    m_txOutputLossDb (0),
    m_txPointingLossDb (0),
    m_txOboLossDb (0),
    m_txAntennaLossDb (0),
    m_defaultFadingValue (1)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatPhy default constructor is not allowed to use");
}

SatPhy::SatPhy (CreateParam_t & params)
 : m_eirpWoGainW (0),
   m_rxMaxAntennaGainDb (0),
   m_rxAntennaLossDb (0),
   m_txMaxAntennaGainDb (0),
   m_txMaxPowerDbw (0),
   m_txOutputLossDb (0),
   m_txPointingLossDb (0),
   m_txOboLossDb (0),
   m_txAntennaLossDb (0),
   m_defaultFadingValue (1)
{
  NS_LOG_FUNCTION (this << params.m_beamId);
  ObjectBase::ConstructSelf (AttributeConstructionList ());

  Ptr<MobilityModel> mobility = params.m_device->GetNode ()->GetObject<MobilityModel> ();

  m_phyTx = CreateObject<SatPhyTx> ();
  m_phyTx->SetChannel (params.m_txCh);
  m_phyRx = CreateObject<SatPhyRx> ();
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
    .AddTraceSource ("PacketTrace",
                     "Packet event trace",
                     MakeTraceSourceAccessor (&SatPhy::m_packetTrace))
    .AddTraceSource ("Rx",
                     "A packet received",
                     MakeTraceSourceAccessor (&SatPhy::m_rxTrace))
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
SatPhy::ConfigureRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf)
{
  NS_LOG_FUNCTION (this);
  m_phyRx->ConfigurePhyRxCarriers (carrierConf);
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
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << carrierId << " duration: " << duration);

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld =
      (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_RETURN : SatEnums::LD_FORWARD;

  m_packetTrace (Simulator::Now(),
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
  txParams->m_sinr = 0;
  txParams->m_txPower_W = m_eirpWoGainW;
  txParams->m_txInfo.modCod = txInfo.modCod;
  txParams->m_txInfo.waveformId = txInfo.waveformId;
  txParams->m_txInfo.packetType = txInfo.packetType;

  m_phyTx->StartTx (txParams);
}

void
SatPhy::SendPduWithParams (Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << txParams);
  NS_ASSERT (true);
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
SatPhy::Receive (Ptr<SatSignalParameters> rxParams, bool phyError)
{
  NS_LOG_FUNCTION (this << rxParams);

  // Add packet trace entry:
  SatEnums::SatLinkDir_t ld =
      (m_nodeInfo->GetNodeType () == SatEnums::NT_UT) ? SatEnums::LD_FORWARD : SatEnums::LD_RETURN;

  SatEnums::SatPacketEvent_t event = (phyError) ? SatEnums::PACKET_DROP : SatEnums::PACKET_RECV;

  m_packetTrace (Simulator::Now(),
                 event,
                 m_nodeInfo->GetNodeType (),
                 m_nodeInfo->GetNodeId (),
                 m_nodeInfo->GetMacAddress (),
                 SatEnums::LL_PHY,
                 ld,
                 SatUtils::GetPacketInfo (rxParams->m_packetsInBurst));

  // If there was a PHY error, packet dropped here
  if (!phyError)
    {
      m_rxCallback ( rxParams->m_packetsInBurst, rxParams);

      // Invoke the `Rx` trace source.
      SatSignalParameters::TransmitBuffer_t::const_iterator it1;
      for (it1 = rxParams->m_packetsInBurst.begin ();
           it1 != rxParams->m_packetsInBurst.end (); ++it1)
        {
          SatAddressTag tag;
          bool isTagged = false;
          ByteTagIterator it2 = (*it1)->GetByteTagIterator ();

          while (!isTagged && it2.HasNext ())
            {
              ByteTagIterator::Item item = it2.Next ();
              if (item.GetTypeId () == SatAddressTag::GetTypeId ())
                {
                  NS_LOG_DEBUG (this << " contains a SatAddressTag tag:"
                                     << " start=" << item.GetStart ()
                                     << " end=" << item.GetEnd ());
                  item.GetTag (tag);
                  isTagged = true;
                }
            }

          if (isTagged)
            {
              m_rxTrace (*it1, tag.GetSourceAddress ());
            }
          else
            {
              m_rxTrace (*it1, Address ()); // provide an invalid source address.
            }

        } // end of `for (it1 = rxParams->m_packetsInBurst)`

    } // end of `if (!phyError)`

}

void
SatPhy::CnoInfo (uint32_t beamId, Address source, Address dest, double cno)
{
  NS_LOG_FUNCTION (this << beamId << source << cno);
  m_cnoCallback ( beamId, source, dest, cno);
}



} // namespace ns3
