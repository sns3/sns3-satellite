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


#ifndef VIRTUAL_CHANNEL_H
#define VIRTUAL_CHANNEL_H

#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/channel.h>

namespace ns3 {

class SatNetDevice;

/**
 * \ingroup satellite
 *
 * Virtual channel implementation
 *
 */


class VirtualChannel : public Channel
{
public:
  static TypeId GetTypeId (void);
  VirtualChannel ();

  void Add (Ptr<SatNetDevice> device);

  // inherited from ns3::Channel
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

private:
  std::vector<Ptr<SatNetDevice> > m_devices;
};

}


#endif /* VIRTUAL_CHANNEL_H */




