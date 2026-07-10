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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SAT_FWD_LINK_SCHEDULER_SCPC_H
#define SAT_FWD_LINK_SCHEDULER_SCPC_H

#include "satellite-fwd-link-scheduler.h"

#include "ns3/pointer.h"

#include <stdint.h>
#include <utility>
#include <vector>

namespace ns3
{

/**
 * @ingroup satellite
 * @brief SatFwdLinkSchedulerDefault schedules BB frames for forward link. This is the reference
 * case of the scheduler, without separation between slices. It uses only one instance of
 * BbFrameContainer.
 *
 *        SatFwdLinkSchedulerDefault communicated through callback functions to request scheduling
 * objects and notifying TX opportunities.
 *
 *        GW MAC requests frames from scheduler through method GetNextFrame.
 *
 */

class SatScpcScheduler : public SatFwdLinkScheduler
{
  public:
    /**
     * @brief Get the type ID
     * @return the object TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * Construct a SatFwdLinkScheduler
     *
     * This the default constructor for the SatFwdLinkScheduler is not supported.
     *
     */
    SatScpcScheduler();

    /**
     * Actual constructor of a SatFwdLinkScheduler
     *
     * @param conf BB Frame configuration
     * @param address MAC address
     * @param carrierBandwidthInHz Carrier bandwidth where scheduler is associated to [Hz].
     */
    SatScpcScheduler(Ptr<SatBbFrameConf> conf, Mac48Address address, double carrierBandwidthInHz);

    /**
     * Notifier called once the ObjectBase is fully constructed.
     *
     * This method is invoked once all member attributes have been
     * initialized. Subclasses can override this method to be notified
     * of this event but if they do this, they must chain up to their
     * parent's NotifyConstructionCompleted method.
     */
    virtual void NotifyConstructionCompleted() override;

    /**
     * Destroy a SatFwdLinkScheduler
     *
     * This is the destructor for the SatFwdLinkScheduler.
     */
    ~SatScpcScheduler();

    /**
     * Get next frame to be transmitted.
     *
     * @return Pointer to frame
     */
    virtual std::pair<Ptr<SatBbFrame>, const Time> GetNextFrame();

  private:
    /**
     * Do dispose actions.
     */
    void DoDispose(void);

    /**
     * Schedule BB Frames.
     */
    void ScheduleBbFrames();

    /**
     * Send stats and reset all the symbols sent count for each slice to zero.
     */
    void SendAndClearSymbolsSentStat();

    /**
     * Gets scheduling object in sorted order according to configured sorting criteria.
     *
     * @param output reference to a vector which will be filled with pointers to
     *               the scheduling objects available for scheduling.
     */
    void GetSchedulingObjects(std::vector<Ptr<SatSchedulingObject>>& output);

    /**
     * The container for BB Frames.
     */
    Ptr<SatBbFrameContainer> m_bbFrameContainer;

    /**
     * Threshold time of total transmissions in BB Frame container to trigger a scheduling round.
     */
    Time m_schedulingStartThresholdTime;

    /**
     * Threshold time of total transmissions in BB Frame container to stop a scheduling round.
     */
    Time m_schedulingStopThresholdTime;

    /**
     * The number of symbols sent for each slice during an allocation cycle.
     */
    uint32_t m_symbolsSent;
};

} // namespace ns3

#endif /* SAT_FWD_LINK_SCHEDULER_SCPC_H */
