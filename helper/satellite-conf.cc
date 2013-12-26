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
#include "ns3/enum.h"
#include "ns3/simulator.h"
#include "satellite-conf.h"
#include "satellite-wave-form-conf.h"


NS_LOG_COMPONENT_DEFINE ("SatConf");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatConf);

TypeId
SatConf::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatConf")
      .SetParent<Object> ()
      .AddConstructor<SatConf> ()
      .AddAttribute ("FwdLinkBandwidth", "Bandwidth of the forward link.",
                      DoubleValue (2.0e9),
                      MakeDoubleAccessor (&SatConf::m_fwdFeederLinkBandwidth_hz),
                      MakeDoubleChecker<double> ())
      .AddAttribute ("FwdFeederLinkBaseFrequency", "Base frequency of the forward feeder link band.",
                      DoubleValue (27.5e9),
                      MakeDoubleAccessor (&SatConf::m_fwdFeederLinkFreq_hz),
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
      .AddAttribute ("StaticFrameConfig",
                     "Static frame configuration used for superframes.",
                      EnumValue (SatConf::STATIC_CONFIG_0),
                      MakeEnumAccessor (&SatConf::m_staticFrameConfig),
                      MakeEnumChecker (SatConf::STATIC_CONFIG_0, "Configuration 0",
                                       SatConf::STATIC_CONFIG_1, "Configuration 1",
                                       SatConf::STATIC_CONFIG_2, "Configuration 2",
                                       SatConf::STATIC_CONFIG_3, "Configuration 3"))
      .AddAttribute ("StaticConfTargetDuration",
                     "Target duration of the superframe for static configuration [s].",
                     DoubleValue (0.010),
                     MakeDoubleAccessor (&SatConf::m_frameConfTargetDuration),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("StaticConfAllocatedBandwidth",
                     "The allocated carrier bandwidth for static configuration [Hz].",
                     DoubleValue (1.25e6),
                     MakeDoubleAccessor (&SatConf::m_frameConfAllocatedBandwidth),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("StaticConfRollOff",
                     "The roll-off factor for static configuration.",
                     DoubleValue (0.20),
                     MakeDoubleAccessor (&SatConf::m_frameConfRollOffFactor),
                     MakeDoubleChecker<double> (0.00, 1.00))
      .AddAttribute ("StaticConfSpacing",
                     "The carrier spacing factor for static configuration.",
                     DoubleValue (0.30),
                     MakeDoubleAccessor (&SatConf::m_frameConfSpacingFactor),
                     MakeDoubleChecker<double> (0.00, 1.00))
      .AddAttribute ("FwdCarrierAllocatedBandwidth",
                     "The allocated carrier bandwidth for forward link carriers [Hz].",
                     DoubleValue (0.125e9),
                     MakeDoubleAccessor (&SatConf::m_fwdCarrierAllocatedBandwidth),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("FwdCarrierRollOff",
                     "The roll-off factor for forward link carriers.",
                     DoubleValue (0.20),
                     MakeDoubleAccessor (&SatConf::m_fwdCarrierRollOffFactor),
                     MakeDoubleChecker<double> (0.00, 1.00))
      .AddAttribute ("FwdCarrierSpacing",
                     "The carrier spacing factor for forward link carriers.",
                     DoubleValue (0.00),
                     MakeDoubleAccessor (&SatConf::m_fwdCarrierSpacingFactor),
                     MakeDoubleChecker<double> (0.00, 1.00))
;
    return tid;
}

TypeId
SatConf::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId();
}

SatConf::SatConf()
{
  NS_LOG_FUNCTION (this);

  // Nothing done here
}


void SatConf::Initialize (std::string path, std::string satConf, std::string gwPos, std::string satPos, std::string wfConf)
{
  NS_LOG_FUNCTION (this);

  // Load satellite configuration file
  LoadSatConf (path + satConf);

  // Load GW positions
  LoadGwPos (path + gwPos);

  // Load satellite position
  LoadGeoSatPos (path + satPos);

  Configure (path + wfConf);
}

void
SatConf::Configure (std::string wfConf)
{
  NS_LOG_FUNCTION (this);

  // *** configure forward link ***

  // currently only one carrier in forward link is used.
  double fwdFeederLinkBandwidth = m_fwdFeederLinkBandwidth_hz / m_feederLinkChannelCount;
  double fwdUserLinkBandwidth = m_fwdUserLinkBandwidth_hz / m_userLinkChannelCount;

  // bandwidths of the forward feeder and user links is expected to be equal
    if ( fwdFeederLinkBandwidth != fwdUserLinkBandwidth )
      {
        NS_FATAL_ERROR ( "Bandwidths of forward feeder and user links are not equal!!!");
      }

  Ptr<SatBtuConf> fwdCarrierConf = Create<SatBtuConf> (m_fwdCarrierAllocatedBandwidth, m_fwdCarrierRollOffFactor, m_fwdCarrierSpacingFactor );

  m_forwardLinkCarrierConf.push_back (fwdCarrierConf);

  // *** configure return link ***

  double rtnFeederLinkBandwidth = m_rtnFeederLinkBandwidth_hz / m_feederLinkChannelCount;
  double rtnUserLinkBandwidth = m_rtnUserLinkBandwidth_hz / m_userLinkChannelCount;

  // bandwidths of the return feeder and user links is expected to be equal
  if ( rtnFeederLinkBandwidth != rtnUserLinkBandwidth )
    {
      NS_FATAL_ERROR ( "Bandwidths of return feeder and user links are not equal!!!");
    }

  switch (m_staticFrameConfig)
  {
    case STATIC_CONFIG_0:
      {
        // Create superframe sequence and add later needed elements to it
        m_superframeSeq = CreateObject<SatSuperframeSeq> ();

        // Create BTU conf according to given attributes
        Ptr<SatBtuConf> btuConf = Create<SatBtuConf> ( m_frameConfAllocatedBandwidth, m_frameConfRollOffFactor, m_frameConfSpacingFactor );

        // Create a waveform configuration
        Ptr<SatWaveformConf> waveFormConf = CreateObject<SatWaveformConf> (wfConf);

        // get default waveform
        uint32_t defaultWaveFormId = waveFormConf->GetDefaultWaveformId ();
        Ptr<SatWaveform> defaultWaveForm = waveFormConf->GetWaveform (defaultWaveFormId);

        double timeSlotDuration = defaultWaveForm->GetBurstDurationInSeconds (btuConf->GetSymbolRate_baud ());
        uint32_t slotCount = m_frameConfTargetDuration / timeSlotDuration;

        if ( slotCount == 0 )
          {
            slotCount = 1;
          }

        // Created one frame to be used utilizating earlier created BTU
        Ptr<SatFrameConf> frameConf = Create<SatFrameConf> (rtnUserLinkBandwidth, slotCount * timeSlotDuration,
                                                            btuConf, (SatFrameConf::SatTimeSlotConfList_t *) NULL);

        // Created time slots for every carrier and add them to frame configuration
        for (uint32_t i = 0; i < frameConf->GetCarrierCount (); i++)
          {
            for (uint32_t j = 0; j < slotCount; j++)
              {
                Ptr<SatTimeSlotConf> timeSlot = Create<SatTimeSlotConf> (j * timeSlotDuration, defaultWaveFormId, i);
                frameConf->AddTimeSlotConf (timeSlot);
              }
          }

        // Create superframe configuration without frame first
        Ptr<SatSuperframeConf> superframeConf = Create<SatSuperframeConf> ( frameConf->GetBandwidth_hz (), frameConf->GetDuration_s (),
                                                                            (SatSuperframeConf::SatFrameConfList_t *) NULL);

        // Add earlier created frame to superframe configuration
        superframeConf->AddFrameConf (frameConf);

        // Add earlier created superframe and wave form configurations to it
        m_superframeSeq->AddWaveformConf (waveFormConf);
        m_superframeSeq->AddSuperframe (superframeConf);
      }
      break;

    case STATIC_CONFIG_1:
    case STATIC_CONFIG_2:
    case STATIC_CONFIG_3:
      // TODO: Add other static configuration..
      NS_ASSERT (false); // these are not supported yet
      break;

    default:
      NS_ASSERT (false);
      break;
  }

}

double
SatConf::GetCarrierFrequency( SatEnums::ChannelType_t chType, uint32_t freqId, uint32_t carrierId )
{
  NS_LOG_FUNCTION (this << chType << freqId << carrierId);

  double centerFrequency_hz = 0.0;
  double baseFreq_hz = 0.0;
  double channelBandwidth = 0.0;
  double carrierBandwidth = 0.0;

  switch (chType)
  {
    case SatEnums::FORWARD_FEEDER_CH:
      channelBandwidth = m_fwdFeederLinkBandwidth_hz / m_feederLinkChannelCount;
      carrierBandwidth = m_forwardLinkCarrierConf[0]->GetAllocatedBandwidth_hz ();
      baseFreq_hz = m_fwdFeederLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + (carrierBandwidth * carrierId) + (carrierBandwidth / 2);
      break;

    case SatEnums::FORWARD_USER_CH:
      channelBandwidth = m_fwdUserLinkBandwidth_hz / m_userLinkChannelCount;
      carrierBandwidth = m_forwardLinkCarrierConf[0]->GetAllocatedBandwidth_hz ();
      baseFreq_hz = m_fwdUserLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + (carrierBandwidth * carrierId) + (carrierBandwidth / 2);
      break;

    case SatEnums::RETURN_FEEDER_CH:
      channelBandwidth = m_rtnFeederLinkBandwidth_hz / m_feederLinkChannelCount;
      baseFreq_hz = m_rtnFeederLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + m_superframeSeq->GetCarrierFrequency_hz ( carrierId);
      break;

    case SatEnums::RETURN_USER_CH:
      channelBandwidth = m_rtnUserLinkBandwidth_hz / m_userLinkChannelCount;
      baseFreq_hz = m_rtnUserLinkFreq_hz + ( channelBandwidth * (freqId - 1) );
      centerFrequency_hz = baseFreq_hz + m_superframeSeq->GetCarrierFrequency_hz ( carrierId);
      break;

    default:
      NS_ASSERT (false);
      break;
  }

  return centerFrequency_hz;
}

double
SatConf::GetCarrierBandwidth( SatEnums::ChannelType_t chType, uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidhtType )
{
  NS_LOG_FUNCTION (this << chType << carrierId);

  double carrierBandwidth = 0.0;

  switch (chType)
  {
    case SatEnums::FORWARD_FEEDER_CH:
      carrierBandwidth = GetFwdLinkCarrierFrequencyHz (carrierId, bandwidhtType);
      break;

    case SatEnums::FORWARD_USER_CH:
      carrierBandwidth = GetFwdLinkCarrierFrequencyHz (carrierId, bandwidhtType);
      break;

    case SatEnums::RETURN_FEEDER_CH:
      carrierBandwidth = m_superframeSeq->GetCarrierBandwidth_hz (carrierId, bandwidhtType);
      break;

    case SatEnums::RETURN_USER_CH:
      carrierBandwidth = m_superframeSeq->GetCarrierBandwidth_hz (carrierId, bandwidhtType);
      break;

    default:
      NS_ASSERT (false);
      break;
  }

  return carrierBandwidth;
}

std::ifstream* SatConf::OpenFile (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

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

      // Store the valuesNS_LOG_FUNCTION (this);
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

  return m_superframeSeq->GetCarrierCount();
}

uint32_t
SatConf::GetFwdLinkCarrierCount () const
{
  NS_LOG_FUNCTION (this);

  return m_forwardLinkCarrierConf.size();
}

double
SatConf::GetFwdLinkCarrierFrequencyHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const
{
  NS_LOG_FUNCTION (this);

  double bandwidtHz = 0.0;

  switch (bandwidthType)
  {
    case SatEnums::ALLOCATED_BANDWIDTH:
      bandwidtHz = m_forwardLinkCarrierConf[carrierId]->GetAllocatedBandwidth_hz ();
      break;

    case SatEnums::OCCUPIED_BANDWIDTH:
      bandwidtHz = m_forwardLinkCarrierConf[carrierId]->GetOccupiedBandwidth_hz ();
      break;

    case SatEnums::EFFECTIVE_BANDWIDTH:
      bandwidtHz = m_forwardLinkCarrierConf[carrierId]->GetEffectiveBandwidth_hz ();
      break;

    default:
      NS_FATAL_ERROR ("Invalid bandwidth type");
      break;
  }

  return bandwidtHz;
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

} // namespace ns3



