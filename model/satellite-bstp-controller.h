/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Magister Solutions Ltd.
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

#ifndef SATELLITE_BSTP_CONTROLLER_H_
#define SATELLITE_BSTP_CONTROLLER_H_

#include <deque>
#include "ns3/object.h"
#include "ns3/callback.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatBstpController
 */
class SatBstpController : public Object
{
public:

  typedef enum
  {
    BH_UNKNOWN = 0,
    BH_STATIC = 1,
    BH_DYNAMIC = 2,
  } BeamHoppingType_t;

  /**
   * Default constructor.
   */
	SatBstpController ();

  /**
   * Destructor for SatRequestManager
   */
  virtual ~SatBstpController ();

  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * Callback to fetch queue statistics
   */
  typedef Callback<void, bool> ToggleCallback;

  /**
   * \brief Add a callback to the SatNetDevice of GW matching
   * to a certain beam id.
   * \param beamId Beam id
   * \param userFreqId User frequency id
   * \param feederFreqId Feeder frequency id
   * \param gwId Gateway id
   * \param cb Callback to the toggle method of ND
   */
  void AddNetDeviceCallback (uint32_t beamId,
                             uint32_t userFreqId,
                             uint32_t feederFreqId,
                             uint32_t gwId,
                             SatBstpController::ToggleCallback cb);

protected:

  /**
   * \brief Periodical method to enable/disable certain beam
   * ids related to the scheduling and transmission of BB frames.
   */
  void DoBstpConfiguration ();

private:

  typedef std::map<uint32_t, ToggleCallback> CallbackContainer_t;

  CallbackContainer_t m_gwNdCallbacks;
  BeamHoppingType_t m_bhMode;
  std::string m_configFileName;

};

} // namespace

#endif /* SATELLITE_BSTP_CONTROLLER_H_ */
