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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "satellite-conf.h"


NS_LOG_COMPONENT_DEFINE ("SatConf");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatConf);

TypeId
SatConf::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatConf")
      .SetParent<Object> ()
      .AddConstructor<SatConf> ()
      .AddAttribute ("FwdFeederLinkBaseFrequency", "Base frequency of the forward feeder link band.",
                      DoubleValue (27.5e9),
                      MakeDoubleAccessor (&SatConf::m_fwdFeederLinkFreq_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("FwdFeederLinkBandwidth", "Bandwidth of the forward feeder link band.",
                      DoubleValue (2.0e9),
                      MakeDoubleAccessor (&SatConf::m_fwdFeederLinkBandwidth_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("RtnFeederLinkBaseFrequency", "Base frequency of the return feeder link band.",
                      DoubleValue (17.7e9),
                      MakeDoubleAccessor (&SatConf::m_rtnFeederLinkFreq_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("RtnFeederLinkBandwidth", "Bandwidth of the return feeder link band.",
                      DoubleValue (2.0e9),
                      MakeDoubleAccessor (&SatConf::m_rtnFeederLinkBandwidth_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("FwdUserLinkBaseFrequency", "Base frequency of the forward user link band.",
                      DoubleValue (19.7e9),
                      MakeDoubleAccessor (&SatConf::m_fwdUserLinkFreq_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("FwdUserLinkBandwidth", "Bandwidth of the forward user link band.",
                      DoubleValue (0.5e9),
                      MakeDoubleAccessor (&SatConf::m_fwdUserLinkBandwidth_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("RtnUserLinkBaseFrequency", "Base frequency of the return user link band.",
                      DoubleValue (29.5e9),
                      MakeDoubleAccessor (&SatConf::m_rtnUserLinkFreq_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("RtnUserLinkBandwidth", "Bandwidth of the return user link band.",
                      DoubleValue (0.5e9),
                      MakeDoubleAccessor (&SatConf::m_rtnUserLinkBandwidth_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("UserLinkChannels", "The number of channels in user link",
                      UintegerValue (4),
                      MakeUintegerAccessor (&SatConf::m_userLinkChannelCount),
                      MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("FeederLinkChannels", "The number of channels in feeder link",
                      UintegerValue (16),
                      MakeUintegerAccessor (&SatConf::m_feederLinkChannelCount),
                      MakeUintegerChecker<uint32_t> (1))
    ;
    return tid;
}

TypeId
SatConf::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

SatConf::SatConf()
{
  // Nothing done here
}


void SatConf::Initialize (std::string path, std::string satConf, std::string gwPos, std::string satPos)
{
  // Load satellite configuration file
  LoadSatConf (path + satConf);

  // Load GW positions
  LoadGwPos (path + gwPos);

  // Load satellite position
  LoadGeoSatPos (path + satPos);

  Configure ();
}

void
SatConf::Configure()
{
  //TODO: Now we just create some carriers with one frame, needed to do proper configuration later

  Ptr<SatBtuConf> btuConf = Create<SatBtuConf> (125.0e6/100.0, 0.100/10.0, 7.69e6);

  Ptr<SatFrameConf> frameConf = Create<SatFrameConf> (125.0e6, btuConf->GetLength_s()*10, btuConf, (std::vector<Ptr<SatTimelSlotConf> > *) NULL);

  for (uint32_t i = 0; i < frameConf->GetCarrierCount(); i++)
    {
      Ptr<SatTimelSlotConf> timeSlot = Create<SatTimelSlotConf> (0, btuConf->GetLength_s()*5, 0, i);
      frameConf->AddTimeSlotConf(timeSlot);

      timeSlot = Create<SatTimelSlotConf> (btuConf->GetLength_s()*5, btuConf->GetLength_s()*5, 0, i);
      frameConf->AddTimeSlotConf(timeSlot);
    }

  Ptr<SatSuperFrameConf> superframeConf = Create<SatSuperFrameConf> (frameConf->GetBandwidth_hz(), frameConf->GetDuration_s(), (std::vector<Ptr<SatFrameConf> > *) NULL);

  superframeConf->AddFrameConf (frameConf);

  m_sctTable.push_back (superframeConf);

  m_forwardLinkCarrierConf.push_back(1);
}

double
SatConf::GetCarrierFrequency( SatChannel::ChannelType_t chType, uint32_t freqId, uint32_t carrierId )
{
  double centerFrequency_hz = 0.0;
  double baseFreq_hz = 0.0;
  double channelBandwidth = 0.0;
  double carrierBandwidth = 0.0;

  switch (chType)
  {
    case SatChannel::FORWARD_FEEDER_CH:
      channelBandwidth = m_fwdFeederLinkBandwidth_hz / m_feederLinkChannelCount;
      carrierBandwidth = channelBandwidth / m_forwardLinkCarrierConf[carrierId];
      baseFreq_hz = m_fwdFeederLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + (carrierBandwidth * carrierId) + (carrierBandwidth / 2);
      break;

    case SatChannel::FORWARD_USER_CH:
      channelBandwidth = m_fwdUserLinkBandwidth_hz / m_userLinkChannelCount;
      carrierBandwidth = channelBandwidth / m_forwardLinkCarrierConf[carrierId];
      baseFreq_hz = m_fwdUserLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + (carrierBandwidth * carrierId) + (carrierBandwidth / 2);
      break;

    case SatChannel::RETURN_FEEDER_CH:
      channelBandwidth = m_rtnFeederLinkBandwidth_hz / m_feederLinkChannelCount;
      baseFreq_hz = m_rtnFeederLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + m_sctTable[0]->GetCarrierFrequency (carrierId);
      break;

    case SatChannel::RETURN_USER_CH:
      channelBandwidth = m_rtnUserLinkBandwidth_hz / m_userLinkChannelCount;
      baseFreq_hz = m_rtnUserLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + m_sctTable[0]->GetCarrierFrequency (carrierId);
      break;

    default:
      NS_ASSERT (false);
      break;
  }

  return centerFrequency_hz;
}

double
SatConf::GetCarrierBandwidth( SatChannel::ChannelType_t chType, uint32_t carrierId )
{
  double channelBandwidth = 0.0;
  double carrierBandwidth = 0.0;

  uint32_t seqId = 0;
  uint32_t superFrameCarrierId = 0;

  switch (chType)
  {
    case SatChannel::FORWARD_FEEDER_CH:
      channelBandwidth = m_fwdFeederLinkBandwidth_hz / m_feederLinkChannelCount;
      carrierBandwidth = channelBandwidth / m_forwardLinkCarrierConf[carrierId];
      break;

    case SatChannel::FORWARD_USER_CH:
      channelBandwidth = m_fwdUserLinkBandwidth_hz / m_userLinkChannelCount;
      carrierBandwidth = channelBandwidth / m_forwardLinkCarrierConf[carrierId];
      break;

    case SatChannel::RETURN_FEEDER_CH:
      channelBandwidth = m_rtnFeederLinkBandwidth_hz / m_feederLinkChannelCount;
      superFrameCarrierId = GetSuperFrameCarrierId(carrierId, &seqId );
      carrierBandwidth = m_sctTable[seqId]->GetCarrierBandwidth (superFrameCarrierId);
      break;

    case SatChannel::RETURN_USER_CH:
      channelBandwidth = m_rtnUserLinkBandwidth_hz / m_userLinkChannelCount;
      superFrameCarrierId = GetSuperFrameCarrierId(carrierId, &seqId );
      carrierBandwidth = m_sctTable[seqId]->GetCarrierBandwidth (superFrameCarrierId);
      break;

    default:
      NS_ASSERT (false);
      break;
  }

  return carrierBandwidth;
}

std::ifstream* SatConf::OpenFile (std::string filePathName)
{
  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

  if (!ifs->is_open ())
    {
      // script might be launched by test.py, try a different base path
      delete ifs;
      filePathName = "../../" + filePathName;
      ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

      if (!ifs->is_open ())
        {
          NS_FATAL_ERROR ("The file " << filePathName << " is not found.");
        }
    }
    return ifs;
}

void
SatConf::LoadSatConf (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = OpenFile (filePathName);

  uint32_t beamId, userChannelId, gwId, feederChannelId;
  *ifs >> beamId >> userChannelId >> gwId >> feederChannelId;

  while (ifs->good ())
    {
      NS_LOG_DEBUG (this <<
                    " beamId = " << beamId <<
                    ", userChannelId = " << userChannelId <<
                    ", gwId = " << gwId <<
                    ", feederChannelId = " << feederChannelId);

      // Store the values
      std::vector <uint32_t> beamConf;

      beamConf.push_back (beamId);
      beamConf.push_back (userChannelId);
      beamConf.push_back (gwId);
      beamConf.push_back (feederChannelId);

      m_conf.push_back(beamConf);

      // get next row
      *ifs >> beamId >> userChannelId >> gwId >> feederChannelId;
    }

  m_beamCount = m_conf.size ();

  ifs->close ();
  delete ifs;
}

void
SatConf::LoadGwPos (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = OpenFile (filePathName);

  double lat, lon, alt;
  *ifs >> lat >> lon >> alt;

  while (ifs->good ())
    {
      NS_LOG_DEBUG (this <<
                    " latitude [deg] = " << lat <<
                    ", longitude [deg] = " << lon <<
                    ", altitude [m] = ");

      // Store the values
      GeoCoordinate coord(lat, lon, alt);
      m_gwPositions.push_back (coord);

      // get next row
      *ifs >> lat >> lon >> alt;
    }

  m_gwCount = m_gwPositions.size ();

  ifs->close ();
  delete ifs;
}

void
SatConf::LoadGeoSatPos (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = OpenFile (filePathName);

  double lat, lon, alt;
  *ifs >> lat >> lon >> alt;

  if (ifs->good ())
    {
      NS_LOG_DEBUG (this <<
                    " latitude [deg] = " << lat <<
                    ", longitude [deg] = " << lon <<
                    ", altitude [m] = ");

      m_geoSatPosition = GeoCoordinate (lat,lon,alt);
    }

  ifs->close ();
  delete ifs;
}

uint32_t
SatConf::GetBeamCount() const
{
  NS_LOG_FUNCTION (this);

  return m_beamCount;
}

uint32_t
SatConf::GetGwCount() const
{
  NS_LOG_FUNCTION (this);

  return m_gwCount;
}

std::vector <uint32_t>
SatConf::GetBeamConfiguration (uint32_t beamId) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT((beamId > 0) && (beamId <=  m_beamCount));

  return m_conf[beamId - 1];
}

uint32_t
SatConf::GetRtnLinkCarrierCount () const
{
  NS_LOG_FUNCTION (this);

  uint32_t carrierCount = 0;

  for (std::vector< Ptr<SatSuperFrameConf> >::const_iterator it = m_sctTable.begin(); it != m_sctTable.end(); it++ )
    {
      carrierCount += (*it)->GetCarrierCount();
    }

  return carrierCount;
}

uint32_t
SatConf::GetFwdLinkCarrierCount () const
{
  NS_LOG_FUNCTION (this);

  return m_forwardLinkCarrierConf.size();
}

GeoCoordinate
SatConf::GetGwPosition (uint32_t gwId) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT((gwId > 0) && (gwId <=  m_gwCount));

  return m_gwPositions[gwId - 1];
}

GeoCoordinate
SatConf::GetGeoSatPosition () const
{
  NS_LOG_FUNCTION (this);

  return m_geoSatPosition;
}

uint32_t
SatConf::GetSuperFrameCarrierId (uint32_t carrierId, uint32_t * seqId)
{
  uint32_t superFrameId = carrierId;
  uint32_t superFrameSeq = 0;
  uint32_t lastIdInSuperFrame = m_sctTable[0]->GetCarrierCount() - 1;

  while ( carrierId > lastIdInSuperFrame)
    {
      superFrameId -= m_sctTable[superFrameSeq]->GetCarrierCount();
      superFrameSeq++;
      lastIdInSuperFrame += m_sctTable[superFrameSeq]->GetCarrierCount();
    }

  *seqId = superFrameSeq;

  return superFrameId;
}

uint32_t
SatConf::GetGlobalCarrierId (uint32_t seqId, uint32_t carrierId)
{
  uint32_t globalCarrierId = carrierId;

  for ( uint32_t i = 0; i < seqId; i++)
    {
      NS_ASSERT ( i < m_sctTable.size());

      globalCarrierId += m_sctTable[i]->GetCarrierCount();
    }

  return globalCarrierId;
}

} // namespace ns3



