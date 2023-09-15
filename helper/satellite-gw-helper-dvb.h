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

#ifndef SATELLITE_GW_HELPER_DVB_H
#define SATELLITE_GW_HELPER_DVB_H

#include <ns3/satellite-gw-helper.h>

#include <ns3/satellite-channel.h>
#include <ns3/satellite-mac.h>
#include <ns3/satellite-ncc.h>
#include <ns3/satellite-superframe-sequence.h>
#include <ns3/satellite-typedefs.h>

#include <string>

namespace ns3
{

/**
 * \brief Creates needed objects for DVB GW nodes like SatGeoNetDevice objects.
 *        Handles needed configuration for the GW nodes.
 *
 */
class SatGwHelperDvb : public SatGwHelper
{
  public:
    /**
     * \brief Get the type ID
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const;

    /**
     * Default constructor.
     */
    SatGwHelperDvb();

    /**
     * Create a SatGwHelperDvb to make life easier when creating Satellite point to
     * point network connections.
     */
    SatGwHelperDvb(SatTypedefs::CarrierBandwidthConverter_t carrierBandwidthConverter,
                   uint32_t fwdLinkCarrierCount,
                   Ptr<SatSuperframeSeq> seq,
                   SatMac::ReadCtrlMsgCallback readCb,
                   SatMac::ReserveCtrlMsgCallback reserveCb,
                   SatMac::SendCtrlMsgCallback sendCb,
                   RandomAccessSettings_s randomAccessSettings);

    virtual ~SatGwHelperDvb()
    {
    }

    /**
     * \param n node
     * \param gwId  id of the gw
     * \param satId  id of the satellite
     * \param beamId  id of the beam
     * \param fCh forward channel
     * \param rCh return channel
     * \param ncc NCC (Network Control Center)
     * \param forwardLinkRegenerationMode The regeneration mode on forward link
     * \param returnLinkRegenerationMode The regeneration mode on return link
     *
     * This method creates a ns3::SatChannel with the
     * attributes configured by SatGwHelper::SetChannelAttribute,
     * then, for each node in the input container, we create a
     * ns3::SatNetDevice with the requested attributes,
     * a queue for this ns3::SatNetDevice, and associate the resulting
     * ns3::SatNetDevice with the ns3::Node and ns3::SatChannel.
     */
    virtual Ptr<NetDevice> Install(Ptr<Node> n,
                                   uint32_t gwId,
                                   uint32_t satId,
                                   uint32_t beamId,
                                   Ptr<SatChannel> fCh,
                                   Ptr<SatChannel> rCh,
                                   Ptr<SatNcc> ncc,
                                   Ptr<SatLowerLayerServiceConf> llsConf,
                                   SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                                   SatEnums::RegenerationMode_t returnLinkRegenerationMode);
};

} // namespace ns3

#endif /* SATELLITE_GW_HELPER_DVB_H */
