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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 * Author: Mathias Ettinger <mettinger@viveris.toulouse.fr>
 */

#ifndef SAT_ORBITER_HELPER_DVB_H
#define SAT_ORBITER_HELPER_DVB_H

#include "satellite-orbiter-helper.h"

#include "ns3/error-model.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/pointer.h"
#include "ns3/satellite-bbframe-conf.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-fwd-link-scheduler.h"
#include "ns3/satellite-ncc.h"
#include "ns3/satellite-orbiter-feeder-mac.h"
#include "ns3/satellite-orbiter-net-device.h"
#include "ns3/satellite-phy.h"
#include "ns3/satellite-scpc-scheduler.h"
#include "ns3/satellite-superframe-sequence.h"
#include "ns3/satellite-typedefs.h"
#include "ns3/traced-callback.h"

#include <map>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

namespace ns3
{

/**
 * @brief Creates needed objects for Satellite node like SatorbiterNetDevice objects.
 *        Handles needed configuration for the Satellite node.
 *
 */
class SatOrbiterHelperDvb : public SatOrbiterHelper
{
  public:
    /**
     * @brief Get the type ID
     * @return the object TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * Default constructor.
     */
    SatOrbiterHelperDvb();

    /**
     * Create a SatOrbiterHelperDvb to make life easier when creating Satellite point to
     * point network connections.
     */
    SatOrbiterHelperDvb(SatTypedefs::CarrierBandwidthConverter_t bandwidthConverterCb,
                        uint32_t rtnLinkCarrierCount,
                        uint32_t fwdLinkCarrierCount,
                        Ptr<SatSuperframeSeq> seq,
                        SatMac::ReadCtrlMsgCallback fwdReadCb,
                        SatMac::ReadCtrlMsgCallback rtnReadCb,
                        RandomAccessSettings_s randomAccessSettings);

    virtual ~SatOrbiterHelperDvb()
    {
    }

    /**
     * Create a SatOrbiterNetDevice instance, with correct type infered from child classes.
     *
     * @return SatOrbiterNetDevice instance
     */
    virtual Ptr<SatOrbiterNetDevice> CreateOrbiterNetDevice();

    /*
     * Attach the SatChannels for the beam to NetDevice
     * @param dev NetDevice to attach channels
     * @param uf user forward channel
     * @param uf user return channel
     * @param userAgp user beam antenna gain pattern
     * @param ncc NCC (Network Control Center)
     * @param satId ID of satellite associated to this channel
     * @param userBeamId Id of the beam
     */
    virtual void AttachChannelsUser(Ptr<SatOrbiterNetDevice> dev,
                                    Ptr<SatChannel> uf,
                                    Ptr<SatChannel> ur,
                                    Ptr<SatAntennaGainPattern> userAgp,
                                    Ptr<SatNcc> ncc,
                                    uint32_t satId,
                                    uint32_t userBeamId);
};

} // namespace ns3

#endif /* SAT_ORBITER_HELPER_DVB_H */
