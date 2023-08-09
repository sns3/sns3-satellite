/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_RESIDUAL_INTERFERENCE_ELIMINATION_H
#define SATELLITE_RESIDUAL_INTERFERENCE_ELIMINATION_H

#include "satellite-interference-elimination.h"
#include "satellite-enums.h"
#include "satellite-wave-form-conf.h"


namespace ns3 {

/**
 * \ingroup satellite
 * \brief Satellite interference elimination with residual power.
 */
class SatResidualInterferenceElimination : public SatInterferenceElimination
{
public:
  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor. Should not be used.
   */
  SatResidualInterferenceElimination ();

  /**
   * Constructor
   */
  SatResidualInterferenceElimination (Ptr<SatWaveformConf> waveformConf);

  /**
   * Destructor
   */
  ~SatResidualInterferenceElimination ();

  /**
   * Eliminate the residual power of interference for the given packets
   * \param packetInterferedWith Parameters of the packet whose interference level should be lowered
   * \param processedPacket Parameters of the packet we want to remove interference from
   * \param isRegenerative Tell if we are in regenerative configuration. Depending on this value, the computations are made on the GW or on the satellite
   */
  void EliminateInterferences (Ptr<SatSignalParameters> packetInterferedWith, Ptr<SatSignalParameters> processedPacket, double EsNo, bool isRegenerative);

  /**
   * Calculate residual power of interference for the given packets
   * \param packetInterferedWith Parameters of the packet whose interference level should be lowered
   * \param processedPacket Parameters of the packet we want to remove interference from
   * \param isRegenerative Tell if we are in regenerative configuration. Depending on this value, the computations are made on the GW or on the satellite
   * \param startTime Normalized start time of the interference, with respect to packetInterferedWith
   * \param endTime Normalized end time of the interference, with respect to packetInterferedWith
   */
  void EliminateInterferences (Ptr<SatSignalParameters> packetInterferedWith, Ptr<SatSignalParameters> processedPacket, double EsNo, bool isRegenerative, double startTime, double endTime);

  inline uint32_t GetBurstLengthInSymbols (uint32_t waveformId) const
  {
    return m_waveformConf->GetWaveform (waveformId)->GetBurstLengthInSymbols ();
  }

  /**
   * \brief Calculate the residual power of interference for a given packet
   */
  double GetResidualPower (Ptr<SatSignalParameters> processedPacket, double EsNo);

private:
  Ptr<SatWaveformConf> m_waveformConf;
  double m_samplingError;
};

}  // namespace ns3

#endif /* SATELLITE_RESIDUAL_INTERFERENCE_ELIMINATION_H */
