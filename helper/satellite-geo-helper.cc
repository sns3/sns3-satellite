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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-phy.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-phy-rx.h"


#include "ns3/trace-helper.h"
#include "satellite-geo-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGeoHelper");

namespace ns3 {

SatGeoHelper::SatGeoHelper ()
  :m_deviceCount(0)
{
  m_deviceFactory.SetTypeId ("ns3::SatGeoNetDevice");
}

void 
SatGeoHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
SatGeoHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type SatGeoNetDevice.
  //
  Ptr<SatGeoNetDevice> device = nd->GetObject<SatGeoNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("SatGeoHelper::EnablePcapInternal(): Device " << device << " not of type ns3::SatGeoNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out,
                                                     PcapHelper::DLT_RAW);
}

void 
SatGeoHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type SatGeoNetDevice.
  //

  Ptr<SatGeoNetDevice> device = nd->GetObject<SatGeoNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("SatGeoHelper::EnableAsciiInternal(): Device " << device <<
                   " not of type ns3::SatGeoNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to 
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create 
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy 
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      //
      // The MacRx trace source provides our "r" event.
      //
      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<SatGeoNetDevice> (device, "MacRx", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to providd a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for 
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the 
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static 
  // functions that are always there waiting for just such a case.
  //
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SatGeoNetDevice/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));
}

NetDeviceContainer 
SatGeoHelper::Install (NodeContainer c)
{
  // currently only one node supported by helper
  NS_ASSERT (c.GetN () == 1);

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add(Install(*i));
  }

  return devs;
}

Ptr<NetDevice>
SatGeoHelper::Install (Ptr<Node> n)
{
  NS_ASSERT (m_deviceCount == 0);

  // Create SatGeoNetDevice
  Ptr<SatGeoNetDevice> satDev = m_deviceFactory.Create<SatGeoNetDevice> ();

  satDev->SetAddress (Mac48Address::Allocate ());
  n->AddDevice(satDev);
  m_deviceCount++;

  return satDev;
}

Ptr<NetDevice>
SatGeoHelper::Install (std::string nName)
{
  Ptr<Node> n = Names::Find<Node> (nName);
  return Install (n);
}

void
SatGeoHelper::AttachChannels (Ptr<NetDevice> d, Ptr<SatChannel> ff, Ptr<SatChannel> fr, Ptr<SatChannel> uf, Ptr<SatChannel> ur, uint16_t beamId )
{
  NS_LOG_FUNCTION (this << ff << fr << uf << ur);

  Ptr<SatGeoNetDevice> dev = DynamicCast<SatGeoNetDevice> (d);

  // Create the first needed SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> uPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> uPhyRx = CreateObject<SatPhyRx> ();
  Ptr<SatPhyTx> fPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> fPhyRx = CreateObject<SatPhyRx> ();

  // Set SatChannels to SatPhyTx/SatPhyRx
  uPhyTx->SetChannel (uf);
  uPhyRx->SetChannel (ur);
  uPhyRx->SetDevice (dev);

  fPhyTx->SetChannel (fr);
  fPhyRx->SetChannel (ff);
  fPhyRx->SetDevice (dev);

  SatPhy::ReceiveCallback uCb = MakeCallback (&SatGeoNetDevice::ReceiveUser, dev);
  SatPhy::ReceiveCallback fCb = MakeCallback (&SatGeoNetDevice::ReceiveFeeder, dev);

  // Create SatPhy modules
  Ptr<SatPhy> uPhy = CreateObject<SatPhy> (uPhyTx, uPhyRx, beamId, uCb);
  Ptr<SatPhy> fPhy = CreateObject<SatPhy> (fPhyTx, fPhyRx, beamId, fCb);

  dev->AddUserPhy(uPhy, beamId);
  dev->AddFeederPhy(fPhy, beamId);
}

} // namespace ns3
