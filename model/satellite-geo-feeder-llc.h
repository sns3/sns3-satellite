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
 * Author: Bastien TAURAN <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_GEO_FEEDER_LLC_H_
#define SATELLITE_GEO_FEEDER_LLC_H_

#include "satellite-geo-llc.h"


namespace ns3 {

/**
 * \ingroup satellite TODO
 */
class SatGeoFeederLlc : public SatGeoLlc
{
public:
  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Construct a SatGeoFeederLlc, should not be used
   */
  SatGeoFeederLlc ();

  /**
   * Construct a SatGeoFeederLlc
   * \param forwardLinkRegenerationMode Forward link regeneration model
   * \param returnLinkRegenerationMode Return link regeneration model
   */
  SatGeoFeederLlc (SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
                   SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /**
   * Destroy a SatGeoFeederLlc
   *
   * This is the destructor for the SatGeoFeederLlc.
   */
  virtual ~SatGeoFeederLlc ();

protected:
  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * \brief Method to create a new encapsulator 'on-a-need-basis' dynamically.
   * \param key Encapsulator key class
   */
  virtual void CreateEncap (Ptr<EncapKey> key);

  /**
   * \brief Virtual method to create a new decapsulator 'on-a-need-basis' dynamically.
   * \param key Encapsulator key class
   */
  virtual void CreateDecap (Ptr<EncapKey> key);

};

} // namespace ns3


#endif /* SATELLITE_GEO_FEEDER_LLC_H_ */
