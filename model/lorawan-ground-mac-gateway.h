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

#ifndef LORAWAN_GROUND_MAC_GATEWAY_H
#define LORAWAN_GROUND_MAC_GATEWAY_H

#include "lora-tag.h"
#include "lorawan-mac-gateway.h"
#include "lorawan-mac.h"
#include "satellite-bbframe-container.h"

#include <stdint.h>

namespace ns3
{
namespace satellite
{

class LorawanGroundMacGateway : public LorawanMacGateway
{
  public:
    static TypeId GetTypeId(void);

    LorawanGroundMacGateway();
    LorawanGroundMacGateway(uint32_t satId, uint32_t beamId);
    virtual ~LorawanGroundMacGateway();

    // Implementation of the LorawanMac interface
    virtual void Send(Ptr<Packet> packet);

    // Implementation of the LorawanMac interface
    // virtual void Receive (Ptr<Packet const> packet);
    virtual void Receive(SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/);

  private:
    // BB Frame configuration.
  protected:
};

}
} /* namespace ns3 */

#endif /* LORAWAN_GROUND_MAC_GATEWAY_H */
