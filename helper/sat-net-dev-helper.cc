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
#include "ns3/satellite-phy.h"
#include "ns3/satellite-phy-tx.h"
#include "ns3/satellite-phy-rx.h"
#include "ns3/virtual-channel.h"
#include "ns3/propagation-delay-model.h"


#include "ns3/trace-helper.h"
#include "ns3/sat-net-dev-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatNetDevHelper");

namespace ns3 {

SatNetDevHelper::SatNetDevHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::SatNetDevice");
  m_channelFactory.SetTypeId ("ns3::SatChannel");

  //LogComponentEnable ("SatNetDevHelper", LOG_LEVEL_INFO);

  m_beamId = 1;
}

void 
SatNetDevHelper::SetQueue (std::string type,
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
SatNetDevHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
SatNetDevHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

void 
SatNetDevHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type SatNetDevice.
  //
  Ptr<SatNetDevice> device = nd->GetObject<SatNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("SatNetDevHelper::EnablePcapInternal(): Device " << device << " not of type ns3::SatNetDevice");
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
SatNetDevHelper::EnableAsciiInternal (
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
      NS_LOG_INFO ("SatNetDevHelper::EnableAsciiInternal(): Device " << device <<
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

  oss << "/NodeList/" << nd->GetNode ()->GetId () << "/DeviceList/" << deviceid << "/$ns3::SatNetDevice/MacRx";
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
SatNetDevHelper::Install (NodeContainer c)
{
  NS_ASSERT (c.GetN () == 2);
  return Install (c.Get (0), c.Get (1));
}

NetDeviceContainer 
SatNetDevHelper::Install (Ptr<Node> a, Ptr<Node> b)
{
  NetDeviceContainer container;

  // Create SatNetDevices
  Ptr<SatNetDevice> aDev = m_deviceFactory.Create<SatNetDevice> ();
  Ptr<SatNetDevice> bDev = m_deviceFactory.Create<SatNetDevice> ();

  // Create the SatPhyTx and SatPhyRx modules
  Ptr<SatPhyTx> aPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> aPhyRx = CreateObject<SatPhyRx> ();
  Ptr<SatPhyTx> bPhyTx = CreateObject<SatPhyTx> ();
  Ptr<SatPhyRx> bPhyRx = CreateObject<SatPhyRx> ();

  // Create SatChannels
  Ptr<SatChannel> abChannel = m_channelFactory.Create<SatChannel> ();
  Ptr<SatChannel> baChannel = m_channelFactory.Create<SatChannel> ();

  // Create VirtualChannel used for getting the global routing to work
  Ptr<VirtualChannel> vChannel = Create<VirtualChannel> ();
  vChannel->Add (aDev);
  vChannel->Add (bDev);
  aDev->SetVirtualChannel (vChannel);
  bDev->SetVirtualChannel (vChannel);

  /*
   * Average propagation delay between UT/GW and satellite in seconds
   * \todo Change the propagation delay to be a parameter.
   */
  double pd = 0.13;
  Ptr<ConstantPropagationDelayModel> pDelay = Create<ConstantPropagationDelayModel> (pd);
  abChannel->SetPropagationDelayModel (pDelay);
  baChannel->SetPropagationDelayModel (pDelay);

  // Set SatChannels to SatPhyTx/SatPhyRx
  aPhyTx->SetChannel (abChannel);
  aPhyRx->SetChannel (baChannel);
  aPhyRx->SetDevice (aDev);

  bPhyTx->SetChannel (baChannel);
  bPhyRx->SetChannel (abChannel);
  bPhyRx->SetDevice (bDev);

  Ptr<SatMac> aMac = CreateObject<SatMac> ();
  Ptr<SatMac> bMac = CreateObject<SatMac> ();

  // Create and set queues for Mac modules
  Ptr<Queue> queueA = m_queueFactory.Create<Queue> ();
  aMac->SetQueue (queueA);
  Ptr<Queue> queueB = m_queueFactory.Create<Queue> ();
  bMac->SetQueue (queueB);

  // Create SatPhy modules
  Ptr<SatPhy> aPhy = CreateObject<SatPhy> (aPhyTx, aPhyRx, m_beamId);
  Ptr<SatPhy> bPhy = CreateObject<SatPhy> (bPhyTx, bPhyRx, m_beamId);

  // Attach the Mac layers to Phys
  aPhy->SetMac (aMac);
  bPhy->SetMac (bMac);

  // Attach the PHY layers to SatNetDevice
  aDev->SetPhy (aPhy);
  bDev->SetPhy (bPhy);

  // Attach the PHY layers to SatMac
  aMac->SetPhy (aPhy);
  bMac->SetPhy (bPhy);

  // Attach the devices to Mac layers
  aDev->SetMac(aMac);
  bDev->SetMac(bMac);

  // Attach the Mac layers to SatNetDevice
  aMac->SetDevice (aDev);
  bMac->SetDevice (bDev);

  // Attach the SatNetDevices to nodes
  a->AddDevice (aDev);
  b->AddDevice (bDev);

  container.Add (aDev);
  container.Add (bDev);

  return container;
}

NetDeviceContainer 
SatNetDevHelper::Install (Ptr<Node> a, std::string bName)
{
  Ptr<Node> b = Names::Find<Node> (bName);
  return Install (a, b);
}

NetDeviceContainer 
SatNetDevHelper::Install (std::string aName, Ptr<Node> b)
{
  Ptr<Node> a = Names::Find<Node> (aName);
  return Install (a, b);
}

NetDeviceContainer 
SatNetDevHelper::Install (std::string aName, std::string bName)
{
  Ptr<Node> a = Names::Find<Node> (aName);
  Ptr<Node> b = Names::Find<Node> (bName);
  return Install (a, b);
}

} // namespace ns3
