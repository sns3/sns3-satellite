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
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-geo-net-device.h"
#include "../model/satellite-geo-feeder-phy.h"
#include "../model/satellite-geo-user-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-phy-rx-carrier-conf.h"

#include "satellite-geo-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGeoHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoHelper);

TypeId
SatGeoHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatGeoHelper")
      .SetParent<Object> ()
      .AddConstructor<SatGeoHelper> ()
      .AddAttribute ("FwdLinkInterferenceModel",
                     "Forward link interference model",
                     EnumValue (SatPhyRxCarrierConf::IF_CONSTANT),
                     MakeEnumAccessor (&SatGeoHelper::m_fwdLinkInterferenceModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute ("RtnLinkInterferenceModel",
                     "Return link interference model",
                     EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET),
                     MakeEnumAccessor (&SatGeoHelper::m_rtnLinkInterferenceModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute( "FwdLinkRxTemperatureDbK",
                     "The forward link RX noise temperature in Geo satellite.",
                     DoubleValue(28.4),
                     MakeDoubleAccessor(&SatGeoHelper::m_fwdLinkRxTemperature_dbK),
                     MakeDoubleChecker<double>())
      .AddAttribute( "RtnLinkRxTemperatureDbK",
                     "The return link RX noise temperature in Geo satellite.",
                     DoubleValue(28.4),
                     MakeDoubleAccessor(&SatGeoHelper::m_rtnLinkRxTemperature_dbK),
                     MakeDoubleChecker<double>())
      .AddAttribute( "FwdLinkOtherSysNoiseDbHz",
                     "Other system noise of the forward link in Geo satellite.",
                     DoubleValue (SatUtils::MinDb<double> ()),
                     MakeDoubleAccessor(&SatGeoHelper::m_fwdLinkOtherSysNoise_dbHz),
                     MakeDoubleChecker<double>())
      .AddAttribute( "RtnLinkOtherSysNoiseDbHz",
                     "Other system noise of the return link in Geo satellite.",
                     DoubleValue (SatUtils::MinDb<double> ()),
                     MakeDoubleAccessor(&SatGeoHelper::m_rtnLinkOtherSysNoise_dbHz),
                     MakeDoubleChecker<double>(SatUtils::MinDb<double> (), SatUtils::MaxDb<double> ()))
     .AddAttribute( "FwdRxOtherSysIfDb",
                    "Other system interference of the forward link in Geo satellite.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_fwdOtherSysInterference_db),
                     MakeDoubleChecker<double>())
     .AddAttribute( "RtnRxOtherSysIfDb",
                    "Other system interference of the return link in Geo satellite.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_rtnOtherSysInterference_db),
                     MakeDoubleChecker<double>())
     .AddAttribute( "FwdRxImIfDb",
                    "Intermodultation interference of the forward link in Geo satellite.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_fwdImInterference_db),
                     MakeDoubleChecker<double>())
     .AddAttribute( "RtnRxImIfDb",
                    "Intermodultation interference of the retun link in Geo satellite.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_rtnImInterference_db),
                     MakeDoubleChecker<double>())
     .AddAttribute( "FwdRxAciIfDb",
                    "Adjacent channel interference of the forward link in Geo satellite.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_fwdAciInterference_db),
                     MakeDoubleChecker<double>())
     .AddAttribute( "RtnRxAciIfDb",
                    "Adjacent channel interference of the return link in Geo satellite.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_rtnAciInterference_db),
                     MakeDoubleChecker<double>())
     .AddAttribute( "FwdRxAciIfWrtNoise",
                    "Adjacent channel interference wrt noise in percents for the forward link.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_fwdAciIfWrtNoise),
                     MakeDoubleChecker<double>())
     .AddAttribute( "RtnRxAciIfWrtNoise",
                    "Adjacent channel interference wrt noise in percents for the return link.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor(&SatGeoHelper::m_rtnAciIfWrtNoise),
                     MakeDoubleChecker<double>())
     .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatGeoHelper::m_creation))
    ;
    return tid;
}

TypeId
SatGeoHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this );

  return GetTypeId();
}

SatGeoHelper::SatGeoHelper()
{
  NS_LOG_FUNCTION (this );

  // this default constructor should be never called
  NS_ASSERT (false);
}

SatGeoHelper::SatGeoHelper (CarrierBandwidthConverter bandwidthConverterCb, uint32_t rtnLinkCarrierCount, uint32_t fwdLinkCarrierCount)
  : m_carrierBandwidthConverter (bandwidthConverterCb),
    m_fwdLinkCarrierCount (fwdLinkCarrierCount),
    m_rtnLinkCarrierCount (rtnLinkCarrierCount),
    m_deviceCount(0)
{
  NS_LOG_FUNCTION (this << rtnLinkCarrierCount << fwdLinkCarrierCount );

  m_deviceFactory.SetTypeId ("ns3::SatGeoNetDevice");
}

void 
SatGeoHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void
SatGeoHelper::SetUserPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGeoUserPhy" + n1, v1);
}

void
SatGeoHelper::SetFeederPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGeoFeederPhy" + n1, v1);
}

NetDeviceContainer 
SatGeoHelper::Install (NodeContainer c)
{
  NS_LOG_FUNCTION (this );

  // currently only one node supported by helper
  NS_ASSERT (c.GetN () == 1);

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add(Install(*i));
  }

  return devs;
}

Ptr<NetDevice>
SatGeoHelper::Install (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this << n);

  NS_ASSERT (m_deviceCount == 0);

  // Create SatGeoNetDevice
  Ptr<SatGeoNetDevice> satDev = m_deviceFactory.Create<SatGeoNetDevice> ();

  satDev->SetAddress (Mac48Address::Allocate ());
  n->AddDevice(satDev);
  m_deviceCount++;

  return satDev;
}

Ptr<NetDevice>
SatGeoHelper::Install (std::string aName)
{
  NS_LOG_FUNCTION (this << aName );

  Ptr<Node> n = Names::Find<Node> (aName);

  return Install (n);
}

void
SatGeoHelper::AttachChannels (Ptr<NetDevice> d, Ptr<SatChannel> ff, Ptr<SatChannel> fr, Ptr<SatChannel> uf, Ptr<SatChannel> ur, Ptr<SatAntennaGainPattern> userAgp, Ptr<SatAntennaGainPattern> feederAgp, uint32_t userBeamId )
{
  NS_LOG_FUNCTION (this << d << ff << fr << uf << ur << userAgp << feederAgp << userBeamId);

  Ptr<SatGeoNetDevice> dev = DynamicCast<SatGeoNetDevice> (d);
  //Ptr<MobilityModel> mobility = dev->GetNode()->GetObject<MobilityModel>();

  Ptr<SatGeoUserPhy> uPhy = CreateObject<SatGeoUserPhy> (d, uf, ur, userBeamId);
  Ptr<SatGeoFeederPhy> fPhy = CreateObject<SatGeoFeederPhy> (d, fr, ff, userBeamId);

  SatPhy::ReceiveCallback uCb = MakeCallback (&SatGeoNetDevice::ReceiveUser, dev);
  SatPhy::ReceiveCallback fCb = MakeCallback (&SatGeoNetDevice::ReceiveFeeder, dev);

  uPhy->SetAttribute ("ReceiveCb", CallbackValue (uCb));
  fPhy->SetAttribute ("ReceiveCb", CallbackValue (fCb));

  // Note, that currently we have only one set of antenna patterns,
  // which are utilized in both in user link and feeder link, and
  // in both uplink and downlink directions.
  uPhy->SetTxAntennaGainPattern (userAgp);
  uPhy->SetRxAntennaGainPattern (userAgp);

  // Configure the SatPhyRxCarrier instances
  // \todo We should pass the whole carrier configuration to the SatPhyRxCarrier,
  // instead of just the number of carriers, since it should hold information about
  // the number of carriers, carrier center frequencies and carrier bandwidths, etc.
  // Note, that in GEO satellite, there is no need for error modeling.

  Ptr<SatPhyRxCarrierConf> rtnCarrierConf =
        CreateObject<SatPhyRxCarrierConf> (m_rtnLinkRxTemperature_dbK,
                                           SatPhyRxCarrierConf::EM_NONE,
                                           m_rtnLinkInterferenceModel,
                                           SatPhyRxCarrierConf::TRANSPARENT,
                                           SatEnums::RETURN_USER_CH,
                                           m_carrierBandwidthConverter,
                                           m_rtnLinkCarrierCount);

  rtnCarrierConf->SetAttribute ("ExtNoiseDensityDbWHz", DoubleValue (m_rtnLinkOtherSysNoise_dbHz) );
  rtnCarrierConf->SetAttribute ("RxOtherSysIfDb", DoubleValue (m_rtnOtherSysInterference_db) );
  rtnCarrierConf->SetAttribute ("RxImIfDb", DoubleValue (m_rtnImInterference_db) );
  rtnCarrierConf->SetAttribute ("RxAciIfDb", DoubleValue (m_rtnAciInterference_db) );
  rtnCarrierConf->SetAttribute ("RxAciIfWrtNoise", DoubleValue (m_rtnAciIfWrtNoise) );

  uPhy->ConfigureRxCarriers (rtnCarrierConf);

  // Note, that currently we have only one set of antenna patterns,
  // which are utilized in both in user link and feeder link, and
  // in both uplink and downlink directions.
  fPhy->SetTxAntennaGainPattern (feederAgp);
  fPhy->SetRxAntennaGainPattern (feederAgp);

  // Configure the SatPhyRxCarrier instances
  // Note, that in GEO satellite, there is no need for error modeling.

  Ptr<SatPhyRxCarrierConf> fwdCarrierConf =
        CreateObject<SatPhyRxCarrierConf> (m_fwdLinkRxTemperature_dbK,
                                           SatPhyRxCarrierConf::EM_NONE,
                                           m_fwdLinkInterferenceModel,
                                           SatPhyRxCarrierConf::TRANSPARENT,
                                           SatEnums::FORWARD_FEEDER_CH,
                                           m_carrierBandwidthConverter,
                                           m_fwdLinkCarrierCount);


  fwdCarrierConf->SetAttribute ("ExtNoiseDensityDbWHz", DoubleValue (m_fwdLinkOtherSysNoise_dbHz) );
  fwdCarrierConf->SetAttribute ("RxOtherSysIfDb", DoubleValue (m_fwdOtherSysInterference_db) );
  fwdCarrierConf->SetAttribute ("RxImIfDb", DoubleValue (m_fwdImInterference_db) );
  fwdCarrierConf->SetAttribute ("RxAciIfDb", DoubleValue (m_fwdAciInterference_db) );
  fwdCarrierConf->SetAttribute ("RxAciIfWrtNoise", DoubleValue (m_fwdAciIfWrtNoise) );

  fPhy->ConfigureRxCarriers (fwdCarrierConf);

  dev->AddUserPhy(uPhy, userBeamId);
  dev->AddFeederPhy(fPhy, userBeamId);

  uPhy->Initialize();
  fPhy->Initialize();
}

void
SatGeoHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect ("Creation", "SatGeoHelper",  cb);
}

} // namespace ns3
