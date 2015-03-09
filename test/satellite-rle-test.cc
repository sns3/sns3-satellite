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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

/**
 * \ingroup satellite
 * \file satellite-rle-test.cc
 * \brief Return Link Encapsulator test suite
 */

#include <vector>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/random-variable-stream.h"
#include "../model/satellite-return-link-encapsulator.h"
#include "../model/satellite-queue.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

/**
 * \ingroup satellite
 * \brief Return Link Encapsulation (RLE) test case implementation.
 *
 * Expected results
 * - 100 packets of random size (in bytes) are generated and enqued to RLE
 * - NotifyTxOpportunity is called with random size (in bytes) until all the packets have been
 *   dequed from RLE. RLE performs encapsulation, fragmentation and packing if needed.
 * - Packets are forwarded to the receive functionality of RLE, where they are reassembled
 * - The same amount of packets have to be received as were transmitted
 * - The packet sizes of each enqued HL packet has to be the same as the received (reassembled) packet
 */
class SatRleTestCase : public TestCase
{
public:
  SatRleTestCase ();
  virtual ~SatRleTestCase ();

  /**
   * Receive packet and check that it is of correct size
   * \param p Ptr to packet
   * \param source Source MAC address
   * \param dest Destination MAC address
   */
  void Receive (Ptr<Packet> p, Mac48Address source, Mac48Address dest);

private:
  virtual void DoRun (void);

  /**
   * Sent packet sizes
   */
  std::vector<uint32_t> m_sentPacketSizes;

  /**
   * Received packet sizes
   */
  std::vector<uint32_t> m_rcvdPacketSizes;
};

SatRleTestCase::SatRleTestCase ()
  : TestCase ("Test RLE.")
{
}

SatRleTestCase::~SatRleTestCase ()
{
}


void
SatRleTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-rle", "", true);

  Mac48Address source = Mac48Address::Allocate ();
  Mac48Address dest = Mac48Address::Allocate ();

  uint8_t rcIndex (0);
  Ptr<SatQueue> queue = CreateObject<SatQueue> (rcIndex);
  Ptr<SatReturnLinkEncapsulator> rle = CreateObject<SatReturnLinkEncapsulator> (source, dest, rcIndex);
  rle->SetQueue (queue);

  // Create a receive callback to Receive method of this class.
  rle->SetReceiveCallback (MakeCallback (&SatRleTestCase::Receive, this));

  // Random variable for sent packet sizes and tx opportunities
  Ptr<UniformRandomVariable> unif = CreateObject<UniformRandomVariable> ();

  // Number of created packets
  uint32_t numPackets (100);

  // Create packets and push them to RLE
  for (uint32_t i = 0; i < numPackets; ++i)
    {
      uint32_t packetSize = unif->GetInteger (3, 1500);
      Ptr<Packet> packet = Create<Packet> (packetSize);
      m_sentPacketSizes.push_back (packetSize);
      rle->EnquePdu (packet, dest);
    }

  /**
   * Create TxOpportunities for RLE and call receive method to do decapsuling,
   * defragmentation and reassembly.
   */
  uint32_t nextMinTxO (0);
  uint32_t bytesLeft (1);
  while (bytesLeft > 0)
    {
      Ptr<Packet> p = rle->NotifyTxOpportunity (unif->GetInteger (3, 1500), bytesLeft, nextMinTxO);
      rle->ReceivePdu (p);
    }

  /**
   * Test that the amount of sent packets is the same as the amount of received packets.
   */
  NS_TEST_ASSERT_MSG_EQ ( m_sentPacketSizes.size (), m_rcvdPacketSizes.size (), "All sent packets are not received");

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

void SatRleTestCase::Receive (Ptr<Packet> p, Mac48Address source, Mac48Address dest)
{
  uint32_t rcvdPacketSize = p->GetSize ();
  m_rcvdPacketSizes.push_back (rcvdPacketSize);
  uint32_t numRcvdPackets = m_rcvdPacketSizes.size ();

  /**
   * Test the sent packet size is the same as the received packet size. This tests the
   * encapsulation, fragmentation and packing functionality as well as reassembly.
   */
  NS_TEST_ASSERT_MSG_EQ ( m_sentPacketSizes[numRcvdPackets - 1], m_rcvdPacketSizes[numRcvdPackets - 1], "Wrong size packet received");
}

/**
 * \ingroup satellite
 * \brief Test suite for RLE.
 */
class SatRleTraceSuite : public TestSuite
{
public:
  SatRleTraceSuite ();
};

SatRleTraceSuite::SatRleTraceSuite ()
  : TestSuite ("sat-rle-test", UNIT)
{
  AddTestCase (new SatRleTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatRleTraceSuite SatRleTestSuite;

