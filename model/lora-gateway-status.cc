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

#include "lora-gateway-status.h"

#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE("LoraGatewayStatus");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(LoraGatewayStatus);

TypeId
LoraGatewayStatus::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::LoraGatewayStatus").AddConstructor<LoraGatewayStatus>();
    return tid;
}

LoraGatewayStatus::LoraGatewayStatus()
{
    NS_LOG_FUNCTION(this);
}

LoraGatewayStatus::~LoraGatewayStatus()
{
    NS_LOG_FUNCTION(this);
}

LoraGatewayStatus::LoraGatewayStatus(Address address,
                                     Ptr<NetDevice> netDevice,
                                     Ptr<LorawanMacGateway> gwMac)
    : m_address(address),
      m_netDevice(netDevice),
      m_gatewayMac(gwMac),
      m_nextTransmissionTime(Seconds(0))
{
    NS_LOG_FUNCTION(this);
}

Address
LoraGatewayStatus::GetAddress()
{
    NS_LOG_FUNCTION(this);

    return m_address;
}

void
LoraGatewayStatus::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this);

    m_address = address;
}

Ptr<NetDevice>
LoraGatewayStatus::GetNetDevice()
{
    return m_netDevice;
}

void
LoraGatewayStatus::SetNetDevice(Ptr<NetDevice> netDevice)
{
    m_netDevice = netDevice;
}

Ptr<LorawanMacGateway>
LoraGatewayStatus::GetGatewayMac(void)
{
    return m_gatewayMac;
}

bool
LoraGatewayStatus::IsAvailableForTransmission(double frequency)
{
    NS_LOG_FUNCTION(this << frequency);

    // We can't send multiple packets at once, see SX1301 V2.01 page 29

    // Check that the gateway was not already "booked"
    if (m_nextTransmissionTime > Simulator::Now() - MilliSeconds(1))
    {
        NS_LOG_INFO("This gateway is already booked for a transmission");
        return false;
    }

    // Check that the gateway is not already in TX mode
    if (m_gatewayMac->IsTransmitting())
    {
        NS_LOG_INFO("This gateway is currently transmitting");
        return false;
    }

    // Check that the gateway is not constrained by the duty cycle
    Time waitingTime = m_gatewayMac->GetWaitingTime(frequency);
    if (waitingTime > Seconds(0))
    {
        NS_LOG_INFO("Gateway cannot be used because of duty cycle");
        NS_LOG_INFO("Waiting time at current GW: " << waitingTime.GetSeconds() << " seconds");

        return false;
    }

    return true;
}

void
LoraGatewayStatus::SetNextTransmissionTime(Time nextTransmissionTime)
{
    NS_LOG_FUNCTION(this << nextTransmissionTime);

    m_nextTransmissionTime = nextTransmissionTime;
}
} // namespace ns3
