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

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/callback.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-channel.h"
#include "../model/satellite-mobility-observer.h"
#include "../model/satellite-ut-mac.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-phy-rx-carrier-conf.h"

#include "satellite-ut-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatUtHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtHelper);

TypeId
SatUtHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatUtHelper")
      .SetParent<Object> ()
      .AddConstructor<SatUtHelper> ()
      .AddAttribute ("FwdLinkErrorModel",
                     "Forward link error model",
                     EnumValue (SatPhyRxCarrierConf::EM_AVI),
                     MakeEnumAccessor (&SatUtHelper::m_errorModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::EM_NONE, "None",
                                      SatPhyRxCarrierConf::EM_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::EM_AVI, "AVI"))
      .AddAttribute ("FwdLinkInterferenceModel",
                     "Forward link interference model",
                     EnumValue (SatPhyRxCarrierConf::IF_CONSTANT),
                     MakeEnumAccessor (&SatUtHelper::m_interferenceModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute( "RxTemperatureDbK",
                     "The forward link RX noise temperature in UT.",
                     DoubleValue(24.62),  // ~290K
                     MakeDoubleAccessor (&SatUtHelper::m_rxTemperature_dbK),
                     MakeDoubleChecker<double> ())
      .AddAttribute( "RxOtherSysNoiseDbHz",
                     "Other system noise of RX in UT.",
                     DoubleValue (SatUtils::MinDb<double> ()),
                     MakeDoubleAccessor (&SatUtHelper::m_otherSysNoise_dbHz),
                     MakeDoubleChecker<double> (SatUtils::MinDb<double> (), SatUtils::MaxDb<double> ()))
      .AddAttribute( "RxOtherSysIfDb",
                     "Other system interference of RX in UT.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor (&SatUtHelper::m_otherSysInterference_db),
                      MakeDoubleChecker<double> ())
      .AddAttribute( "RxImIfDb",
                     "Intermodulation interference of RX in UT.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor(&SatUtHelper::m_imInterference_db),
                      MakeDoubleChecker<double> ())
      .AddAttribute( "RxAciIfDb",
                     "Adjacent channel interference of RX in UT.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor(&SatUtHelper::m_aciInterference_db),
                      MakeDoubleChecker<double> ())
      .AddAttribute( "RxAciIfWrtNoise",
                     "Adjacent channel interference wrt noise in percents.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor(&SatUtHelper::m_aciIfWrtNoise),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("CraAllocMode",
                     "Intial Constant Rate Assignment (CRA) allocation mode used for UTs.",
                      EnumValue (SatUtHelper::CONSTANT_CRA),
                      MakeEnumAccessor (&SatUtHelper::m_craAllocMode),
                      MakeEnumChecker (SatUtHelper::CONSTANT_CRA, "Constant CRA used for UTs. (not set by helper, UT's attribute defines value.",
                                       SatUtHelper::RANDOM_CRA, "Random CRA value (128 is set by helper for every UT."))
      .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatUtHelper::m_creation))
    ;
    return tid;
}

TypeId
SatUtHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId();
}

SatUtHelper::SatUtHelper ()
{
  NS_LOG_FUNCTION (this);

  // this default constructor should be never called
  NS_ASSERT (false);
}

SatUtHelper::SatUtHelper (CarrierBandwidthConverter carrierBandwidthConverter, uint32_t fwdLinkCarrierCount, Ptr<SatSuperframeSeq> seq)
 : m_carrierBandwidthConverter (carrierBandwidthConverter),
   m_fwdLinkCarrierCount (fwdLinkCarrierCount),
   m_superframeSeq (seq)
{
  NS_LOG_FUNCTION (this << fwdLinkCarrierCount << seq );

  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");
  m_phyFactory.SetTypeId ("ns3::SatPhy");

  m_phyFactory.Set ("RxMaxAntennaGainDb", DoubleValue(44.60));
  m_phyFactory.Set ("RxAntennaLossDb", DoubleValue(0.00));
  m_phyFactory.Set ("TxMaxAntennaGainDb", DoubleValue(45.20));
  m_phyFactory.Set ("TxMaxPowerDbW", DoubleValue(4.00));
  m_phyFactory.Set ("TxOutputLossDb", DoubleValue(0.50));
  m_phyFactory.Set ("TxPointingLossDb", DoubleValue(1.00));
  m_phyFactory.Set ("TxOboLossDb", DoubleValue(0.50));
  m_phyFactory.Set ("TxAntennaLossDb", DoubleValue(0.00));

  //LogComponentEnable ("SatUtHelper", LOG_LEVEL_INFO);
}

void
SatUtHelper::Initialize ()
{
  NS_LOG_FUNCTION (this);
  /*
   * Forward channel link results (DVB-S2) are created for UTs.
   */
  if (m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_linkResults = CreateObject<SatLinkResultsDvbS2> ();
      m_linkResults->Initialize ();
    }
}

void 
SatUtHelper::SetQueue (std::string type,
                              std::string n1, const AttributeValue &v1,
                              std::string n2, const AttributeValue &v2,
                              std::string n3, const AttributeValue &v3,
                              std::string n4, const AttributeValue &v4)
{
  NS_LOG_FUNCTION (this << type << n1 << n2 << n3 << n4 );

  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void 
SatUtHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void 
SatUtHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_channelFactory.Set (n1, v1);
}

void
SatUtHelper::SetPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_phyFactory.Set (n1, v1);
}

NetDeviceContainer 
SatUtHelper::Install (NodeContainer c, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  NS_LOG_FUNCTION (this << beamId << fCh << rCh );

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add (Install (*i, beamId, fCh, rCh));
  }

  return devs;
}

Ptr<NetDevice>
SatUtHelper::Install (Ptr<Node> n, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  NS_LOG_FUNCTION (this << n << beamId << fCh << rCh );

  NetDeviceContainer container;

  // Create SatNetDevice
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  // Create the SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> phyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> phyRx = CreateObject<SatPhyRx> ();

  // Set SatChannels to SatPhyTx/SatPhyRx
  phyTx->SetChannel (rCh);
  fCh->AddRx (phyRx);
  phyRx->SetDevice (dev);
  phyRx->SetMobility (n->GetObject<MobilityModel> ());
  phyTx->SetMobility (n->GetObject<MobilityModel> ());

  // Configure the SatPhyRxCarrier instances
  // \todo We should pass the whole carrier configuration to the SatPhyRxCarrier,
  // instead of just the number of carriers, since it should hold information about
  // the number of carriers, carrier center frequencies and carrier bandwidths, etc.
  Ptr<SatPhyRxCarrierConf> carrierConf =
        CreateObject<SatPhyRxCarrierConf> (m_rxTemperature_dbK,
                                           m_otherSysNoise_dbHz,
                                           m_errorModel,
                                           m_interferenceModel,
                                           SatPhyRxCarrierConf::NORMAL);

  carrierConf->SetAttribute ("RxOtherSysIfDb", DoubleValue (m_otherSysInterference_db) );
  carrierConf->SetAttribute ("RxImIfDb", DoubleValue (m_imInterference_db) );
  carrierConf->SetAttribute ("RxAciIfDb", DoubleValue (m_aciInterference_db) );
  carrierConf->SetAttribute ("RxAciIfWrtNoise", DoubleValue (m_aciIfWrtNoise) );
  carrierConf->SetAttribute ("ChannelType", EnumValue (SatEnums::FORWARD_USER_CH));
  carrierConf->SetAttribute ("CarrierBandwidhtConverter", CallbackValue (m_carrierBandwidthConverter));
  carrierConf->SetAttribute ("CarrierCount", UintegerValue (m_fwdLinkCarrierCount));

  // If the link results are created, we pass those
  // to SatPhyRxCarrier for error modeling.
  if (m_linkResults)
    {
      carrierConf->SetLinkResults (m_linkResults);
    }

  phyRx->ConfigurePhyRxCarriers (carrierConf);

  // Set fading
  phyTx->SetFadingContainer (n->GetObject<SatFading> ());
  phyRx->SetFadingContainer (n->GetObject<SatFading> ());

  Ptr<SatUtMac> mac = CreateObject<SatUtMac> (m_superframeSeq);
  mac->SetAttribute ("Interval", StringValue ("0s"));

  // Create and set queues for Mac modules
  Ptr<Queue> queue = m_queueFactory.Create<Queue> ();
  mac->SetQueue (queue);

  // Set timing advance callback to mac.
  Ptr<SatMobilityObserver> observer = n->GetObject<SatMobilityObserver> ();
  NS_ASSERT (observer != NULL);
  SatUtMac::TimingAdvanceCallback timingCb = MakeCallback (&SatMobilityObserver::GetTimingAdvance, observer);
  mac->SetTimingAdvanceCallback (timingCb);

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback cb = MakeCallback (&SatUtMac::Receive, mac);

  // Create SatPhy modules
  m_phyFactory.Set ("PhyRx", PointerValue(phyRx));
  m_phyFactory.Set ("PhyTx", PointerValue(phyTx));
  m_phyFactory.Set ("BeamId",UintegerValue(beamId));
  m_phyFactory.Set ("ReceiveCb", CallbackValue(cb));

  Ptr<SatPhy> phy = m_phyFactory.Create<SatPhy>();
  phy->Initialize();

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the PHY layer to SatMac
  mac->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac (mac);

  // Set the device address and pass it to MAC as well
  dev->SetAddress (Mac48Address::Allocate ());
  phyRx->SetAddress (Mac48Address::ConvertFrom (dev->GetAddress ()));

  // Attach the device receive callback to SatMac
  mac->SetReceiveCallback (MakeCallback (&SatNetDevice::ReceiveMac, dev));

  // Attach the SatNetDevices to nodes
  n->AddDevice (dev);

  return DynamicCast <NetDevice> (dev);
}

Ptr<NetDevice>
SatUtHelper::Install (std::string aName, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  NS_LOG_FUNCTION (this << aName << beamId << fCh << rCh );

  Ptr<Node> a = Names::Find<Node> (aName);

  return Install (a, beamId, fCh, rCh);
}

void
SatUtHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect("Creation", "SatUtHelper", cb);
}

} // namespace ns3
