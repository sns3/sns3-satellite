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

#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-channel.h"
#include "../model/satellite-gw-mac.h"
#include "../model/satellite-net-device.h"
#include "../model/satellite-geo-net-device.h"
#include "../model/satellite-gw-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/virtual-channel.h"
#include "../model/satellite-phy-rx-carrier-conf.h"
#include "../model/satellite-link-results.h"
#include "ns3/satellite-gw-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGwHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGwHelper);

TypeId
SatGwHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatGwHelper")
      .SetParent<Object> ()
      .AddConstructor<SatGwHelper> ()
      .AddAttribute ("RtnLinkErrorModel",
                     "Return link error model",
                      EnumValue (SatPhyRxCarrierConf::EM_AVI),
                      MakeEnumAccessor (&SatGwHelper::m_errorModel),
                      MakeEnumChecker (SatPhyRxCarrierConf::EM_NONE, "None",
                                      SatPhyRxCarrierConf::EM_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::EM_AVI, "AVI"))
      .AddAttribute ("RtnLinkInterferenceModel",
                     "Return link interference model",
                      EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET),
                      MakeEnumAccessor (&SatGwHelper::m_interferenceModel),
                      MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute( "RxTemperatureDbK",
                     "RX noise temperature in GW.",
                      DoubleValue(24.62),  // ~290K
                      MakeDoubleAccessor(&SatGwHelper::m_rxTemperature_dbK),
                      MakeDoubleChecker<double>())
      .AddAttribute( "RxOtherSysNoiseDbHz",
                     "Other system noise of RX in GW.",
                      DoubleValue (SatUtils::MinDb<double> ()),
                      MakeDoubleAccessor(&SatGwHelper::m_otherSysNoise_dbHz),
                      MakeDoubleChecker<double>(SatUtils::MinDb<double> (), SatUtils::MaxDb<double> ()))
      .AddAttribute( "RxOtherSysIfDb",
                     "Other system interference of RX in GW.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor(&SatGwHelper::m_otherSysInterference_db),
                      MakeDoubleChecker<double>())
      .AddAttribute( "RxImIfDb",
                     "Intermodultation interference of RX in GW.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor(&SatGwHelper::m_imInterference_db),
                      MakeDoubleChecker<double>())
      .AddAttribute( "RxAciIfDb",
                     "Adjacent channel interference of RX in GW.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor(&SatGwHelper::m_aciInterference_db),
                      MakeDoubleChecker<double>())
      .AddAttribute( "RxAciIfWrtNoise",
                     "Adjacent channel interference wrt noise in percents.",
                      DoubleValue (0.0),
                      MakeDoubleAccessor(&SatGwHelper::m_aciIfWrtNoise),
                      MakeDoubleChecker<double>())
      .AddTraceSource ("Creation", "Creation traces",
                        MakeTraceSourceAccessor (&SatGwHelper::m_creation))
    ;
    return tid;
}

TypeId
SatGwHelper::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatGwHelper::SatGwHelper ()
{
  // this default constructor should be never called
  NS_ASSERT (false);
}

SatGwHelper::SatGwHelper (CarrierBandwidthConverter carrierBandwidthConverter, uint32_t rtnLinkCarrierCount)
 : m_carrierBandwidthConverter (carrierBandwidthConverter),
   m_rtnLinkCarrierCount (rtnLinkCarrierCount)
{
  NS_LOG_FUNCTION (this << rtnLinkCarrierCount);

  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  //LogComponentEnable ("SatGwHelper", LOG_LEVEL_INFO);
}

void
SatGwHelper::Initialize ()
{
  NS_LOG_FUNCTION (this );

  /*
   * Return channel link results (DVB-RCS2) are created for GWs.
   */
  if (m_errorModel == SatPhyRxCarrierConf::EM_AVI)
    {
      m_linkResults = CreateObject<SatLinkResultsDvbRcs2> ();
      m_linkResults->Initialize ();
    }
}

void 
SatGwHelper::SetQueue (std::string type,
                              std::string n1, const AttributeValue &v1,
                              std::string n2, const AttributeValue &v2,
                              std::string n3, const AttributeValue &v3,
                              std::string n4, const AttributeValue &v4)
{
  NS_LOG_FUNCTION (this << type << n1 << n2 << n3 <<  n4  );

  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void 
SatGwHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void 
SatGwHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_channelFactory.Set (n1, v1);
}

void
SatGwHelper::SetPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGwPhy::" + n1, v1);
}

NetDeviceContainer 
SatGwHelper::Install (NodeContainer c, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << beamId << fCh << rCh );

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add (Install (*i, beamId, fCh, rCh, ncc));
  }

  return devs;
}

Ptr<NetDevice>
SatGwHelper::Install (Ptr<Node> n, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << n << beamId << fCh << rCh );

  NetDeviceContainer container;

  // Create SatNetDevice
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  // Attach the SatNetDevices to nodes
  n->AddDevice (dev);

  Ptr<SatGwPhy> phy = CreateObject<SatGwPhy> (dev, fCh, rCh, beamId);

  // Configure the SatPhyRxCarrier instances
  Ptr<SatPhyRxCarrierConf> carrierConf = CreateObject<SatPhyRxCarrierConf> (m_rxTemperature_dbK,
                                                                            m_errorModel,
                                                                            m_interferenceModel,
                                                                            SatPhyRxCarrierConf::NORMAL,
                                                                            SatEnums::RETURN_FEEDER_CH,
                                                                            m_carrierBandwidthConverter,
                                                                            m_rtnLinkCarrierCount);

  carrierConf->SetAttribute ("ExtNoiseDensityDbWHz", DoubleValue (m_otherSysNoise_dbHz) );
  carrierConf->SetAttribute ("RxOtherSysIfDb", DoubleValue (m_otherSysInterference_db) );
  carrierConf->SetAttribute ("RxImIfDb", DoubleValue (m_imInterference_db) );
  carrierConf->SetAttribute ("RxAciIfDb", DoubleValue (m_aciInterference_db) );
  carrierConf->SetAttribute ("RxAciIfWrtNoise", DoubleValue (m_aciIfWrtNoise) );

  // If the link results are created, we pass those
  // to SatPhyRxCarrier for error modeling
  if (m_linkResults)
    {
      carrierConf->SetLinkResults (m_linkResults);
    }

  phy->ConfigureRxCarriers (carrierConf);

  // Set fading
  phy->SetTxFadingContainer (n->GetObject<SatBaseFading> ());
  phy->SetRxFadingContainer (n->GetObject<SatBaseFading> ());

  Ptr<SatGwMac> mac = CreateObject<SatGwMac> ();

  // Create and set queues for Mac modules
  Ptr<Queue> queue = m_queueFactory.Create<Queue> ();
  mac->SetQueue (queue);

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback recCb = MakeCallback (&SatGwMac::Receive, mac);

  // Attach the NCC C/N0 update to Phy
  SatPhy::CnoCallback cnoCb = MakeCallback (&SatNcc::UtCnoUpdated, ncc);

  phy->SetAttribute ("ReceiveCb", CallbackValue(recCb));
 phy->SetAttribute ("CnoCb", CallbackValue(cnoCb));

  //Ptr<SatPhy> phy = DynamicCast<SatPhy> ( m_phyFactory.Create<SatGwPhy>() );
  //phy->Initialize();

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the PHY layer to SatMac
  mac->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac (mac);

  // Set the device address and pass it to MAC as well
  dev->SetAddress (Mac48Address::Allocate ());
  phy->SetAddress (Mac48Address::ConvertFrom (dev->GetAddress ()));

  // Attach the device receive callback to SatMac
  mac->SetReceiveCallback (MakeCallback (&SatNetDevice::ReceiveMac, dev));

  mac->StartScheduling ();

  phy->Initialize();

  return dev;
}

Ptr<NetDevice>
SatGwHelper::Install (std::string aName, uint32_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh, Ptr<SatNcc> ncc )
{
  NS_LOG_FUNCTION (this << aName << beamId << fCh << rCh );

  Ptr<Node> a = Names::Find<Node> (aName);

  return Install (a, beamId, fCh, rCh, ncc);
}

void
SatGwHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this );

  TraceConnect ("Creation", "SatGwHelper", cb);
}

} // namespace ns3
