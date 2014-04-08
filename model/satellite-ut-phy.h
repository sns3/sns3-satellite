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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_UT_PHY_H
#define SATELLITE_UT_PHY_H

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel-estimation-error-container.h"

namespace ns3 {


/**
 * \ingroup satellite
 *
 * The SatUtPhy models the physical layer of the User Terminal of the satellite system.
 */
class SatUtPhy : public SatPhy
{
public:

  /**
   * Default constructor
   */
  SatUtPhy (void);

  SatUtPhy (SatPhy::CreateParam_t & params,
            Ptr<SatLinkResults> linkResults,
            SatPhyRxCarrierConf::RxCarrierCreateParams_s parameters);

  virtual ~SatUtPhy ();

  // inherited from Object
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

  /**
   * UT specific SINR calculator.
   * Calculate SINR with UT PHY specific parameters and given SINR.
   *
   * \param sinr Calculated (C/NI)
   */
  virtual double CalculateSinr (double sinr);

private:
  /**
   * Configured other system interference in dB.
   */
  double m_otherSysInterferenceCOverIDb;

  /**
   * Other system interference in linear.
   */
  double m_otherSysInterferenceCOverI;
};

}

#endif /* SATELLITE_UT_PHY_H */
