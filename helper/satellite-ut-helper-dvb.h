/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_UT_HELPER_DVB_H
#define SATELLITE_UT_HELPER_DVB_H

#include <ns3/satellite-channel.h>
#include <ns3/satellite-ncc.h>
#include <ns3/satellite-superframe-sequence.h>
#include <ns3/satellite-typedefs.h>
#include <ns3/satellite-ut-helper.h>
#include <ns3/satellite-ut-mac.h>

namespace ns3
{

/**
 * \brief Creates needed objects for DVB UT nodes like SatGeoNetDevice objects.
 *        Handles needed configuration for the UT nodes.
 *
 */
class SatUtHelperDvb : public SatUtHelper
{
  public:
    /**
     * Derived from Object
     */
    static TypeId GetTypeId(void);

    /**
     * Derived from Object
     */
    TypeId GetInstanceTypeId(void) const;

    /**
     * Default constructor
     */
    SatUtHelperDvb();

    /**
     * Create a SatUtHelperDvb to make life easier when creating Satellite point to
     * point network connections.
     */
    SatUtHelperDvb(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                   uint32_t rtnLinkCarrierCount,
                   Ptr<SatSuperframeSeq> seq,
                   SatMac::ReadCtrlMsgCallback readCb,
                   SatMac::ReserveCtrlMsgCallback reserveCb,
                   SatMac::SendCtrlMsgCallback sendCb,
                   RandomAccessSettings_s randomAccessSettings);

    /**
     * Destructor
     */
    virtual ~SatUtHelperDvb()
    {
    }

    /**
     * \param n node
     * \param satId  id of the satellite
     * \param beamId  id of the beam
     * \param fCh forward channel
     * \param rCh return channel
     * \param gwNd satellite netdevice of the GW
     * \param ncc NCC (Network Control Center)
     * \param satUserAddress MAC address of satellite user link
     * \param forwardLinkRegenerationMode The regeneration mode on forward link
     * \param returnLinkRegenerationMode The regeneration mode on return link
     * \return Net device installed to node
     *
     * This method creates a ns3::SatChannel with the
     * attributes configured by SatUtHelper::SetChannelAttribute,
     * then, for each node in the input container, we create a
     * ns3::SatNetDevice with the requested attributes,
     * a queue for this ns3::NetDevice, and associate the resulting
     * ns3::NetDevice with the ns3::Node and ns3::SatChannel.
     */
    virtual Ptr<NetDevice> Install(Ptr<Node> n,
                                   uint32_t satId,
                                   uint32_t beamId,
                                   Ptr<SatChannel> fCh,
                                   Ptr<SatChannel> rCh,
                                   Ptr<SatNetDevice> gwNd,
                                   Ptr<SatNcc> ncc,
                                   Address satUserAddress,
                                   SatPhy::ChannelPairGetterCallback cbChannel,
                                   SatMac::RoutingUpdateCallback cbRouting,
                                   SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                   SatEnums::RegenerationMode_t returnLinkRegenerationMode);
};

} // namespace ns3

#endif /* SATELLITE_UT_HELPER_DVB_H */
