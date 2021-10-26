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

#ifndef SAT_LORA_CONF_H
#define SAT_LORA_CONF_H

#include <ns3/satellite-conf.h>

#include <ns3/lorawan-mac-end-device-class-a.h>
#include <ns3/lorawan-mac-gateway.h>

namespace ns3 {

/**
 * \brief A configuration class for the GEO satellite reference system
 *
 */
class SatLoraConf : public Object
{
public:

  typedef enum
  {
    SATELLITE, //!< For Satellite
    EU863_870, //!< EU863-870
  } PhyLayerStandard_t;

  /**
   * Default constructor.
   */
  SatLoraConf ();
  virtual ~SatLoraConf ()
  {
  }

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  void SetConf (Ptr<LorawanMacGateway> gatewayMac);
  void SetConf (Ptr<LorawanMacEndDeviceClassA> endDeviceMac);

  void setSatConfAttributes (Ptr<SatConf> satConf);

private:
  void SetEu863_870Conf (Ptr<LorawanMacGateway> gatewayMac);
  void SetEu863_870Conf (Ptr<LorawanMacEndDeviceClassA> endDeviceMac);

  void SetSatelliteConf (Ptr<LorawanMacGateway> gatewayMac);
  void SetSatelliteConf (Ptr<LorawanMacEndDeviceClassA> endDeviceMac);

  PhyLayerStandard_t m_phyLayerStandard;
};


} // namespace ns3


#endif /* SAT_LORA_CONF_H */
