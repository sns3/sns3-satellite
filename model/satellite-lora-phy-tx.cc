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
 */

#include "satellite-lora-phy-tx.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SatLoraPhyTx");

NS_OBJECT_ENSURE_REGISTERED(SatLoraPhyTx);

TypeId
SatLoraPhyTx::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatLoraPhyTx").SetParent<SatPhyTx>();
    return tid;
}

SatLoraPhyTx::SatLoraPhyTx()
    : m_isTransmitting(false)
{
}

SatLoraPhyTx::~SatLoraPhyTx()
{
}

void
SatLoraPhyTx::SetTxFinishedCallback(TxFinishedCallback callback)
{
    m_txFinishedCallback = callback;
}

void
SatLoraPhyTx::StartTx(Ptr<SatSignalParameters> txParams)
{
    NS_LOG_FUNCTION(this << txParams);

    m_isTransmitting = true;

    SatPhyTx::StartTx(txParams);
}

void
SatLoraPhyTx::EndTx()
{
    NS_LOG_FUNCTION(this);

    m_isTransmitting = false;

    m_txFinishedCallback();

    SatPhyTx::EndTx();
}

bool
SatLoraPhyTx::IsTransmitting()
{
    NS_LOG_FUNCTION(this);

    return m_isTransmitting && SatPhyTx::IsTransmitting();
}

} // namespace ns3
