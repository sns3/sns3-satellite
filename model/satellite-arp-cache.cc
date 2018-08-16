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

#include "ns3/log.h"
#include "ns3/assert.h"

#include "satellite-arp-cache.h"


NS_LOG_COMPONENT_DEFINE ("SatArpCache");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatArpCache);

TypeId
SatArpCache::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatArpCache")
    .SetParent<ArpCache> ()
  ;
  return tid;
}

SatArpCache::SatArpCache ()
  : ArpCache ()
{
  NS_LOG_FUNCTION (this);
}

SatArpCache::~SatArpCache ()
{
  NS_LOG_FUNCTION (this);
}


ArpCache::Entry *
SatArpCache::Add (Ipv4Address to, Address macAddress)
{
  NS_LOG_FUNCTION (this << to << macAddress);

  // Add a new entry
  ArpCache::Entry *entry = ArpCache::Add (to);
  entry->SetMacAddress (macAddress);
  entry->MarkPermanent ();

  NS_LOG_INFO ( "IP: " << to << ", MAC: " << macAddress );

  return entry;
}



} // namespace ns3

