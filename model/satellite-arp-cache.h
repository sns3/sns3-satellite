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
 * \brief An ARP cache interface for satellite module. In satellite module,
 * the ARP cache entries are pre-filled by the helpers and n "infinite"
 * timeout is set for all ARP cache entries. Thus, ARP is enabled but the
 * ARP messages do not need to be actively sent.
 */
class SatArpCache : public ArpCache
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor.
   */
  SatArpCache ();

  /**
   * Destructor for SatArpCache
   */
  ~SatArpCache ();

  /**
   * \brief Add an Ipv4Address - MAC address entry to this ARP cache
   * \param to IP address
   * \param macAddress MAC address
   * \return ArpCache entry
   */
  ArpCache::Entry * Add (Ipv4Address to, Address macAddress);
};


} // namespace ns3

#endif /* SATELLITE_ARP_CACHE_H */
