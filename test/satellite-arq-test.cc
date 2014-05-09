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
#include "ns3/config.h"
#include "../model/satellite-return-link-encapsulator-arq.h"
#include "../model/satellite-queue.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SatRtnArqTestCase");

/**
 * \brief Automatic Repeat reQuest test suite. Test suite holds two test cases:
 * - RTN link ARQ
 * - FWD link ARQ
 *
 * The test case generates m_numPackets packets and buffers them to RLE/GSE queue.
 * The test case generates random sized Tx opportunities at a specified semi-random
 * interval to RLE/GSE. The packets shall be received by the RLE/GSE receiver at
 * a configured probability (m_packetErrorProbability). The RLE/GSE receiver generates
 * ACKs for received packets which are also having a configurable error probability
 * (m_ackErrorProbability). The test ends after a configurable amount of generated
 * Tx opportunities (m_numTxOpportunities).
 *
 * Expected result: the test case shall have at maximum of configurable HL packet
 * error probability (m_errorProbabilityThreshold).
 */
class SatRtnArqTestCase : public TestCase
{
public:
  SatRtnArqTestCase ();
  virtual ~SatRtnArqTestCase ();

  /**
   * Transmit/enque a number of packets to RLE/SatQueue
   * \param numPackets Number of packets
   */
  void TransmitPdus (uint32_t numPackets);

  /**
   * Notify Tx opportunity
   */
  void NotifyTxOpportunity ();

  /**
   * Receive packet
   * \param p Packet
   */
  void ReceivePdu (Ptr<Packet> p);

  /**
   * Send ARQ ACK message to the sender (source)
   * \param msg Control message
   * \param dest Destination MAC address
   * \return bool Whether sending was successfull
   */
  bool SendAck (Ptr<SatControlMessage> msg, const Address& dest);

  /**
   * Receive ARQ ACK message
   * \param ack Acknowledgement
   */
  void ReceiveAck (Ptr<SatArqAckMessage> ack);

  /**
   * Receive packet and check that it is of correct size
   * \param p Ptr to packet
   * \param macAddress UT MAC address
   */
  void Receive (Ptr<Packet> p, Mac48Address macAddress);

private:
  virtual void DoRun (void);

  // Same RLE ARQ handles both transmission and reception
  // operations
  Ptr<SatReturnLinkEncapsulatorArq> m_rle;

  // Random variable
  Ptr<UniformRandomVariable> m_unif;

  // Addressing
  Mac48Address m_source;
  Mac48Address m_dest;

  // Number of created packets
  uint32_t m_numPackets;

  // Number of generated time slots. When we run out of Tx opportunities
  // the test is finished.
  uint32_t m_numTxOpportunities;

  // Tx opportunities (RTN time slot Bytes)
  uint32_t m_minTxOpportunity;
  uint32_t m_maxTxOpportunity;

  // Error ratios
  double m_packetErrorRatio;
  double m_ackErrorRatio;

  // Propagation delay of the satellite channel
  // = constant
  Time m_propagationDelay;

  // RC index for the RLE
  uint8_t m_rcIndex;

  // Error probability threshold for HL packets. If the
  // test results in higher error probability than the threshold
  // the test is failed.
  double m_errorProbabilityThreshold;

  // Interval for creating tx opportunities
  Time m_txoInterval;

  // Jitter (or addition to static Tx opportunity interval)
  double m_minTimeSlotJitterInMs;
  double m_maxTimeSlotJitterInMs;

  uint32_t m_txs;
  uint32_t m_txErrors;
  uint32_t m_acks;
  uint32_t m_ackErrors;

  // Sent packet statistics
  std::vector<uint32_t> m_sentPacketSizes;

  // Received packet statistics
  std::vector<uint32_t> m_rcvdPacketSizes;
};

SatRtnArqTestCase::SatRtnArqTestCase ()
  : TestCase ("Test RLE."),
    m_source (Mac48Address::Allocate ()),
    m_dest (Mac48Address::Allocate ()),
    m_numPackets (1000),
    m_numTxOpportunities (60000),
    m_minTxOpportunity (38),
    m_maxTxOpportunity (599),
    m_packetErrorRatio (0.05),
    m_ackErrorRatio (0.05),
    m_propagationDelay (MilliSeconds (270)),
    m_rcIndex (0),
    m_errorProbabilityThreshold (0.01),
    m_txoInterval (MilliSeconds (5)),
    m_minTimeSlotJitterInMs (1),
    m_maxTimeSlotJitterInMs (10),
    m_txs (0),
    m_txErrors (0),
    m_acks (0),
    m_ackErrors (0)
{
  // The parameters of RLE ARQ may be reconfigured
  Config::SetDefault ("ns3::SatQueue::MaxPackets", UintegerValue(1001));
  Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::MaxNoOfRetransmissions", UintegerValue(3));
  Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::WindowSize", UintegerValue(50));
  Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::RetransmissionTimer", TimeValue(Seconds (0.6)));
  Config::SetDefault ("ns3::SatReturnLinkEncapsulatorArq::RxWaitingTime", TimeValue(Seconds (2.3)));

  Ptr<SatQueue> queue = CreateObject<SatQueue> (m_rcIndex);
  m_rle = CreateObject<SatReturnLinkEncapsulatorArq> (m_source, m_dest, m_rcIndex);
  m_rle->SetQueue (queue);

  // Create a receive callback to Receive method of this class.
  m_rle->SetReceiveCallback (MakeCallback (&SatRtnArqTestCase::Receive, this));

  // Create ACK sending callback
  m_rle->SetCtrlMsgCallback (MakeCallback (&SatRtnArqTestCase::SendAck, this));

  // Random variable for sent packet sizes and tx opportunities
  m_unif = CreateObject<UniformRandomVariable> ();
}

SatRtnArqTestCase::~SatRtnArqTestCase ()
{
  m_rle = NULL;
}


void
SatRtnArqTestCase::DoRun (void)
{
  // Enque packets to RLE
  TransmitPdus (m_numPackets);

  // Schedule the first TxO
  Simulator::Schedule (Seconds (0.1), &SatRtnArqTestCase::NotifyTxOpportunity, this);

  // Create Tx opportunities
  Simulator::Run ();

  // Calculate HL error probability
  double errorProb = 1.0 - (m_rcvdPacketSizes.size() / (double)m_sentPacketSizes.size());

  //std::cout << "Sent packets: " << m_sentPacketSizes.size () << " received packets: " << m_rcvdPacketSizes.size () << std::endl;
  //std::cout << "Tx: " << m_txs << " txErrors: " << m_txErrors << " Acks: " << m_acks << " ackErrors: " << m_ackErrors << std::endl;

  NS_TEST_ASSERT_MSG_LT(errorProb, m_errorProbabilityThreshold, "HL packet error probability is higher than threshold!");

  Simulator::Destroy ();
}

void SatRtnArqTestCase::TransmitPdus (uint32_t numPackets)
{
  // Create packets and push them to RLE
  for (uint32_t i = 0; i < numPackets; ++i)
    {
      uint32_t packetSize = m_unif->GetInteger (3, 500);
      Ptr<Packet> packet = Create<Packet> (packetSize);
      m_sentPacketSizes.push_back (packetSize);
      m_rle->TransmitPdu (packet, m_dest);
    }
}


void SatRtnArqTestCase::NotifyTxOpportunity ()
{
  /**
   * Create TxOpportunities for RLE and call receive method to do decapsuling,
   * defragmentation and reassembly.
   */
  uint32_t nextMinTxO (0);
  uint32_t bytesLeft (1);

  uint32_t bytes = m_unif->GetInteger (m_minTxOpportunity, m_maxTxOpportunity);
  Ptr<Packet> p = m_rle->NotifyTxOpportunity (bytes, bytesLeft, nextMinTxO);

  // If packet received
  if (p)
    {
      m_txs++;

      // Packet received after propagation delay
      if (m_packetErrorRatio < m_unif->GetValue (0.0, 1.0))
        {
          // Schedule packet receive
          Simulator::Schedule (m_propagationDelay, &SatRtnArqTestCase::ReceivePdu, this, p);
        }
      // Transmission error for packet
      else
        {
          m_txErrors++;
        }
    }

  if (m_numTxOpportunities > 0)
    {
      // Schedule next TxO
      Time nextTxOTime = m_txoInterval + MilliSeconds (m_unif->GetValue (m_minTimeSlotJitterInMs, m_maxTimeSlotJitterInMs));
      Simulator::Schedule (nextTxOTime, &SatRtnArqTestCase::NotifyTxOpportunity, this);
    }

  // Reduce count
  m_numTxOpportunities--;
}

void SatRtnArqTestCase::ReceivePdu (Ptr<Packet> p)
{
  m_rle->ReceivePdu (p);
}


bool SatRtnArqTestCase::SendAck (Ptr<SatControlMessage> msg, const Address& dest)
{
  m_acks++;

  // ACK received after propagation delay
  if (m_ackErrorRatio < m_unif->GetValue (0.0, 1.0))
    {
      Ptr<SatArqAckMessage> ack = DynamicCast<SatArqAckMessage> (msg);
      Simulator::Schedule (m_propagationDelay, &SatRtnArqTestCase::ReceiveAck, this, ack);
    }
  // Transmission error for ACK
  else
    {
      m_ackErrors++;
    }

  return true;
}


void SatRtnArqTestCase::ReceiveAck (Ptr<SatArqAckMessage> ack)
{
  m_rle->ReceiveAck (ack);
}


void SatRtnArqTestCase::Receive (Ptr<Packet> p, Mac48Address macAddr)
{
  uint32_t rcvdPacketSize = p->GetSize ();
  m_rcvdPacketSizes.push_back (rcvdPacketSize);

  //std::cout << "Now: " << Now ().GetSeconds () << " sent: " << m_sentPacketSizes.at (m_rcvdPacketSizes.size ()-1) << " Rcvd: " << rcvdPacketSize << std::endl;
}

/**
 * \brief Test suite for ARQ.
 */
class SatArqTestSuite : public TestSuite
{
public:
  SatArqTestSuite ();
};

SatArqTestSuite::SatArqTestSuite ()
  : TestSuite ("sat-arq-test", UNIT)
{
  AddTestCase (new SatRtnArqTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatArqTestSuite satArqTestSuite;

