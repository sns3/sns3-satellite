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

#include "satellite-phy-rx-carrier-conf.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SatPhyRxCarrierConf");

namespace ns3 {

SatPhyRxCarrierConf::SatPhyRxCarrierConf()
{
  m_numCarriers = 1;
  m_errorModel = EM_CONSTANT;
  m_ifModel = IF_CONSTANT;
}

SatPhyRxCarrierConf::SatPhyRxCarrierConf ( uint32_t numCarriers, double rxTemperature_K,
                                           double rxBandwidth_Hz, ErrorModel errorModel, InterferenceModel ifModel)
{
  m_numCarriers = numCarriers;
  m_errorModel = errorModel;
  m_ifModel = ifModel;
}

TypeId
SatPhyRxCarrierConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhyRxCarrierConf")
    .SetParent<Object> ()
    .AddConstructor<SatPhyRxCarrierConf> ()
  ;
  return tid;
}


void
SatPhyRxCarrierConf::SetLinkResults (Ptr<SatLinkResults> linkResults)
{
  m_linkResults = linkResults;
}

uint32_t SatPhyRxCarrierConf::GetCarriersN () const
{
  return m_numCarriers;
}


SatPhyRxCarrierConf::ErrorModel SatPhyRxCarrierConf::GetErrorModel () const
{
  return m_errorModel;
}

SatPhyRxCarrierConf::InterferenceModel SatPhyRxCarrierConf::GetInterferenceModel () const
{
  return m_ifModel;
}

Ptr<SatLinkResults> SatPhyRxCarrierConf::GetLinkResults () const
{
  return m_linkResults;
}

double SatPhyRxCarrierConf::GetBandwidth_Hz () const
{
  return m_bandwidth_Hz;
}

double SatPhyRxCarrierConf::GetRxTemperature_K () const
{
  return m_rxTemperature_K;
}

} // namespace ns3
