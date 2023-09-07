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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#include "satellite-uplink-info-tag.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(SatUplinkInfoTag);

SatUplinkInfoTag::SatUplinkInfoTag()
    : m_satelliteReceptionTime(Seconds(0)),
      m_sinr(0.0),
      m_additionalInterference(0.0),
      m_sinrComputed(false),
      m_satId(0),
      m_beamId(0),
      m_isControl(false)
{
    // Nothing to do here
}

SatUplinkInfoTag::SatUplinkInfoTag(Time satelliteReceptionTime,
                                   double sinr,
                                   double additionalInterference,
                                   uint32_t satId,
                                   uint32_t beamId,
                                   bool isControl)
    : m_satelliteReceptionTime(satelliteReceptionTime),
      m_sinr(sinr),
      m_additionalInterference(additionalInterference),
      m_sinrComputed(true),
      m_satId(satId),
      m_beamId(beamId),
      m_isControl(isControl)
{
    // Nothing to do here
}

TypeId
SatUplinkInfoTag::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::SatUplinkInfoTag").SetParent<Tag>().AddConstructor<SatUplinkInfoTag>();
    return tid;
}

TypeId
SatUplinkInfoTag::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

uint32_t
SatUplinkInfoTag::GetSerializedSize(void) const
{
    return sizeof(Time) + 2 * sizeof(double) + 2 * sizeof(bool) + 2 * sizeof(uint32_t);
}

void
SatUplinkInfoTag::Serialize(TagBuffer i) const
{
    int64_t satelliteReceptionTime = m_satelliteReceptionTime.GetNanoSeconds();
    i.Write((const uint8_t*)&satelliteReceptionTime, sizeof(int64_t));

    i.WriteDouble(m_sinr);
    i.WriteDouble(m_additionalInterference);
    i.WriteU8(m_sinrComputed);
    i.WriteU32(m_satId);
    i.WriteU32(m_beamId);
    i.WriteU8(m_isControl);
}

void
SatUplinkInfoTag::Deserialize(TagBuffer i)
{
    int64_t satelliteReceptionTime;
    i.Read((uint8_t*)&satelliteReceptionTime, 8);
    m_satelliteReceptionTime = NanoSeconds(satelliteReceptionTime);

    m_sinr = i.ReadDouble();
    m_additionalInterference = i.ReadDouble();
    m_sinrComputed = i.ReadU8();
    m_satId = i.ReadU32();
    m_beamId = i.ReadU32();
    m_isControl = i.ReadU8();
}

void
SatUplinkInfoTag::Print(std::ostream& os) const
{
    os << m_satelliteReceptionTime << " " << m_sinr << " " << m_satId << " " << m_beamId << " "
       << m_isControl;
}

Time
SatUplinkInfoTag::GetSatelliteReceptionTime(void) const
{
    return m_satelliteReceptionTime;
}

void
SatUplinkInfoTag::SetSatelliteReceptionTime(Time satelliteReceptionTime)
{
    m_satelliteReceptionTime = satelliteReceptionTime;
}

double
SatUplinkInfoTag::GetSinr(void) const
{
    return m_sinr;
}

void
SatUplinkInfoTag::SetSinr(double sinr, double additionalInterference)
{
    m_sinr = sinr;
    m_additionalInterference = additionalInterference;
}

double
SatUplinkInfoTag::GetAdditionalInterference(void) const
{
    return m_additionalInterference;
}

bool
SatUplinkInfoTag::IsSinrComputed(void) const
{
    return m_sinrComputed;
}

uint32_t
SatUplinkInfoTag::GetSatId(void) const
{
    return m_satId;
}

void
SatUplinkInfoTag::SetSatId(uint32_t satId)
{
    m_satId = satId;
}

uint32_t
SatUplinkInfoTag::GetBeamId(void) const
{
    return m_beamId;
}

void
SatUplinkInfoTag::SetBeamId(uint32_t beamId)
{
    m_beamId = beamId;
}

bool
SatUplinkInfoTag::IsControl(void) const
{
    return m_isControl;
}

void
SatUplinkInfoTag::SetIsControl(bool isControl)
{
    m_isControl = isControl;
}

} // namespace ns3
