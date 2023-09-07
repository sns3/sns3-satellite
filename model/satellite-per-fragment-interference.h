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

#ifndef SATELLITE_PER_FRAGMENT_INTERFERENCE_H
#define SATELLITE_PER_FRAGMENT_INTERFERENCE_H

#include "satellite-per-packet-interference.h"

namespace ns3
{

/**
 * \ingroup satellite
 * \brief Packet fragment by packet fragment interference. Interference
 * is calculated separately for each fragment of a packet depending of
 * how other interferent packets overlaps.
 */
class SatPerFragmentInterference : public SatPerPacketInterference
{
  public:
    /**
     * Inherited from ObjectBase base class.
     */
    static TypeId GetTypeId();

    /**
     * Inherited from ObjectBase base class.
     */
    TypeId GetInstanceTypeId(void) const;

    /**
     * Default constructor
     */
    SatPerFragmentInterference();

    /**
     * Constructor to listen on a specific SatChannel.
     */
    SatPerFragmentInterference(SatEnums::ChannelType_t channelType, double rxBandwidthHz);

    /**
     * Destructor
     */
    ~SatPerFragmentInterference();

  protected:
    /**
     * Calculates interference power for the given reference
     * Sets final power at end time to finalPower.
     *
     * \param event Reference event which for interference is calculated.
     *
     * \return Final calculated power value at end of receiving
     */
    std::vector<std::pair<double, double>> DoCalculate(
        Ptr<SatInterference::InterferenceChangeEvent> event);

    /**
     * Helper function meant for subclasses to override.
     * Called during DoCalculate when the start of the event whose
     * interferences are being calculated has been reached.
     */
    void onOwnStartReached(double ifPowerW);

    /**
     * Helper function meant for subclasses to override.
     * Called during DoCalculate after the start of the event whose
     * interferences are being calculated has been reached.
     *
     * Update the current interference power by adding the power of
     * the event being processed proportional to the remaining time
     * until the end of the event whose interferences are being calculated.
     *
     * \param timeRatio ratio of time compared to an event duration of the
     * distance between the event being processed and the end of the event
     * whose interferences are being calculated.
     * \param interferenceValue the interference value of the event being
     * processed.
     * \param ifPowerW the current value of the interference for the event
     * whose interferences are being calculated; which will be updated.
     */
    void onInterferentEvent(long double timeRatio, double interferenceValue, double& ifPowerW);

  private:
    std::vector<std::pair<double, double>> m_ifPowerAtEventChangeW;

    uint32_t m_maxFragmentsCount;
};

} // namespace ns3

#endif // SATELLITE_PER_FRAGMENT_INTERFERENCE_H
