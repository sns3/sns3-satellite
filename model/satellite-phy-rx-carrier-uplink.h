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


#ifndef SATELLITE_PHY_RX_CARRIER_UPLINK_H
#define SATELLITE_PHY_RX_CARRIER_UPLINK__H

#include <vector>
#include <map>
#include <list>
#include <deque>
#include <ns3/satellite-phy-rx-carrier.h>

namespace ns3 {

class SatPhyRxCarrierConf;
class SatPhyRxCarrier;

/**
 * \ingroup satellite
 * \brief Inherited functionality of the base SatPhyRxCarrier for satellite node.
 */
class SatPhyRxCarrierUplink : public SatPhyRxCarrier
{
public:

	/**
	 * Constructor.
	 * \param carrierId ID of the carrier
	 * \param carrierConf Carrier configuration
	 * \param randomAccessEnabled Is this a RA carrier
	 */
	SatPhyRxCarrierUplink (uint32_t carrierId,
	                       Ptr<SatPhyRxCarrierConf> carrierConf,
	                       bool randomAccessEnabled);

  /**
   * \brief Destructor
   */
  virtual ~SatPhyRxCarrierUplink ();

  /**
   * Get the TypeId of the class.
   * \return TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Method for querying the type of the carrier
   */
  inline virtual CarrierType GetCarrierType () { return CarrierType::DEDICATED_ACCESS; }

protected:

  /**
   * Get the default receive mode. In satellite node always true.
   */
  inline virtual const bool GetDefaultReceiveMode () { return true; };

  /**
   * \brief Function for ending the packet reception from the SatChannel
   * \param key Key for Rx params map
   */
	virtual void EndRxData (uint32_t key);

  /**
   * \brief Create an interference event based on Rx parameters and address.
   *
   * \return Pointer to the interference event.
   */
	virtual Ptr<SatInterference::InterferenceChangeEvent> CreateInterference (Ptr<SatSignalParameters> rxParams, Address rxAddress);
};

}

#endif /* SATELLITE_PHY_RX_CARRIER_UPLINK_H */
