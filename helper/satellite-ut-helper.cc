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
#include "ns3/queue.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/mpi-interface.h"
#include "ns3/mpi-receiver.h"
#include "ns3/satellite-channel.h"
#include "ns3/satellite-mac.h"
#include "ns3/satellite-net-device.h"
#include "ns3/satellite-geo-net-device.h"
#include "ns3/satellite-phy.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/virtual-channel.h"
#include "ns3/propagation-delay-model.h"


#include "ns3/trace-helper.h"
#include "ns3/satellite-ut-helper.h"
#include "ns3/satellite-geo-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatUtHelper");

namespace ns3 {

SatUtHelper::SatUtHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  //LogComponentEnable ("SatUtHelper", LOG_LEVEL_INFO);

  m_beamId = 1;
}

void 
SatUtHelper::SetQueue (std::string type,
                              std::string n1, const AttributeValue &v1,
                              std::string n2, const AttributeValue &v2,
                              std::string n3, const AttributeValue &v3,
                              std::string n4, const AttributeValue &v4)
{
  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void 
SatUtHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
SatUtHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

void 
SatUtHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type SatNetDevice.
  //
  Ptr<SatNetDevice> device = nd->GetObject<SatNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("SatUtHelper::EnablePcapInternal(): Device " << device << " not of type ns3::SatNetDevice");
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
SatUtHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type SatNetDevice.
  //
  Ptr<SatNetDevice> device = nd->GetObject<SatNetDevice> ();
//  Ptr<SimpleNetDevice> device = nd->GetObject<SimpleNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("SatUtHelper::EnableAsciiInternal(): Device " << device <<
                   " not of type ns3::SatNetDevice");
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
      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<SatNetDevice> (device, "MacRx", theStream);

      // PhyRxDrop trace source for "d" event
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<SatNetDevice> (device, "PhyRxDrop", theStream);

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

  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SatNetDevice/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SatNetDevice/TxQueue/Enqueue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SatNetDevice/TxQueue/Dequeue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SatNetDevice/TxQueue/Drop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SatNetDevice/PhyRxDrop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer 
SatUtHelper::Install (NodeContainer c, uint16_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
    NetDeviceContainer devs;

    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add(Install(*i, beamId, fCh, rCh));
    }

    return devs;
}

Ptr<NetDevice>
SatUtHelper::Install (Ptr<Node> n, uint16_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  NetDeviceContainer container;

  // Create SatNetDevice
  Ptr<SatNetDevice> dev = m_deviceFactory.Create<SatNetDevice> ();

  dev->SetAddress (Mac48Address::Allocate ());

  // Create the SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> phyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> phyRx = CreateObject<SatPhyRx> ();

  /*
   * Average propagation delay between UT/GW and satellite in seconds
   * \todo Change the propagation delay to be a parameter.
   */
  double pd = 0.13;
  Ptr<ConstantPropagationDelayModel> pDelay = Create<ConstantPropagationDelayModel> (pd);
  fCh->SetPropagationDelayModel (pDelay);
  rCh->SetPropagationDelayModel (pDelay);

  // Set SatChannels to SatPhyTx/SatPhyRx
  phyTx->SetChannel (rCh);
  phyRx->SetChannel (fCh);
  phyRx->SetDevice (dev);

  Ptr<SatMac> mac = CreateObject<SatMac> ();

  // Create and set queues for Mac modules
  Ptr<Queue> queue = m_queueFactory.Create<Queue> ();
  mac->SetQueue (queue);

  // Attach the Mac layer receiver to Phy
  SatPhy::ReceiveCallback cb = MakeCallback (&SatMac::Receive, mac);

  // Create SatPhy modules
  Ptr<SatPhy> phy = CreateObject<SatPhy> (phyTx, phyRx, beamId, cb);

  // Attach the PHY layer to SatNetDevice
  dev->SetPhy (phy);

  // Attach the PHY layer to SatMac
  mac->SetPhy (phy);

  // Attach the Mac layer to SatNetDevice
  dev->SetMac(mac);

  // Attach the device receive callback to SatMac
  mac->SetReceiveCallback (MakeCallback (&SatNetDevice::Receive, dev));

  // Attach the SatNetDevices to nodes
  n->AddDevice (dev);

  return DynamicCast <NetDevice> (dev);
}

Ptr<NetDevice>
SatUtHelper::Install (std::string aName, uint16_t beamId, Ptr<SatChannel> fCh, Ptr<SatChannel> rCh )
{
  Ptr<Node> a = Names::Find<Node> (aName);
  return Install (a, beamId, fCh, rCh);
}

} // namespace ns3
