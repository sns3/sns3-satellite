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

#ifndef SATELLITE_ARP_CACHE_H
#define SATELLITE_ARP_CACHE_H

#include "ns3/arp-cache.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief An ARP cache interface for satellite module
 *
 * A cached lookup table for translating layer 3 addresses to layer 2.
 * This implementation does lookups from IPv4 to a MAC address
 */
class SatArpCache : public ArpCache
{
public:
  static TypeId GetTypeId (void);
  SatArpCache ();
  ~SatArpCache ();

  /**
   * \brief Add an Ipv4Address - MAC address entry to this ARP cache
   */
  ArpCache::Entry *Add (Ipv4Address to, Address macAddress);
};


} // namespace ns3

#endif /* SATELLITE_ARP_CACHE_H */
