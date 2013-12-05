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

#ifndef SATELLITE_GENERIC_ENCAPSULATOR
#define SATELLITE_GENERIC_ENCAPSULATOR


#include <map>
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "satellite-encapsulator.h"
#include "satellite-encapsulation-sequence-number.h"


namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatGenericEncapsulator class. The implementation is based on
 * LTE sequence number based encapsulator.
 */
class SatGenericEncapsulator : public SatEncapsulator
{
public:
  SatGenericEncapsulator ();
  SatGenericEncapsulator (Mac48Address source, Mac48Address dest);
  virtual ~SatGenericEncapsulator ();

  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  virtual void TransmitPdu (Ptr<Packet> p);
  virtual Ptr<Packet> NotifyTxOpportunity (uint32_t bytes, uint32_t &bytesLeft);
  virtual void ReceivePdu (Ptr<Packet> p);

  virtual uint32_t GetTxBufferSizeInBytes () const;
  virtual uint32_t GetMinTxOpportunityInBytes () const;
  virtual Time GetHolDelay () const;

private:
  void ExpireReorderingTimer (void);
  void ExpireRbsTimer (void);

  bool IsInsideReorderingWindow (SequenceNumber10 seqNumber);

  void ReassembleOutsideWindow (void);
  void ReassembleSnInterval (SequenceNumber10 lowSeqNumber, SequenceNumber10 highSeqNumber);

  void ReassembleAndDeliver (Ptr<Packet> packet);

private:

  Mac48Address m_sourceAddress;
  Mac48Address m_destAddress;

  uint32_t m_maxTxBufferSize;
  uint32_t m_txBufferSize;
  std::vector < Ptr<Packet> > m_txBuffer;       // Transmission buffer
  std::map <uint16_t, Ptr<Packet> > m_rxBuffer; // Reception buffer
  std::vector < Ptr<Packet> > m_reasBuffer;     // Reassembling buffer

  std::list < Ptr<Packet> > m_sdusBuffer;       // List of SDUs in a packet

  /**
   * State variables. See section 7.1 in TS 36.322
   */
  SequenceNumber10 m_sequenceNumber; // VT(US)

  SequenceNumber10 m_vrUr;           // VR(UR)
  SequenceNumber10 m_vrUx;           // VR(UX)
  SequenceNumber10 m_vrUh;           // VR(UH)

  /**
   * Constants. See section 7.2 in TS 36.322
   */
  uint16_t m_windowSize;

  /**
   * Timers. See section 7.3 in TS 36.322
   */
  EventId m_reorderingTimer;

  /**
   * Reassembling state
   */
  typedef enum { NONE            = 0,
                 WAITING_S0_FULL = 1,
                 WAITING_SI_SF   = 2 } ReassemblingState_t;
  ReassemblingState_t m_reassemblingState;
  Ptr<Packet> m_keepS0;

  /**
   * Expected Sequence Number
   */
  SequenceNumber10 m_expectedSeqNumber;

};


} // namespace ns3

#endif // SATELLITE_GENERIC_ENCAPSULATOR
