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

#ifndef SAT_BEAM_SCHEDULER_H
#define SAT_BEAM_SCHEDULER_H

#include <set>

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/mac48-address.h"
#include "ns3/satellite-superframe-sequence.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief class for module Beam Scheduler.
 *
 * This SatBeamScheduler class implements scheduler used to for one specific beam.
 * It's is created and used by NCC.
 *
 */
class SatBeamScheduler : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Construct a SatBeamScheduler
   *
   * This is the constructor for the SatBeamScheduler
   *
   */
  SatBeamScheduler ();

  /**
   * Destroy a SatBeamScheduler
   *
   * This is the destructor for the SatBeamScheduler.
   */
  ~SatBeamScheduler ();

   /**
   * Receive a packet from a beam.
   *
   * The SatBeamScheduler receives CR packets from own beam (sent by UTs)
   * and takes CRs into account when making schedule decisions.
   *
   * /param p       Pointer to the received CR packet.
   */

  void Receive (Ptr<Packet> p);

  /**
    * \param packet     the packet send
    * \param address    Packet destination address
    * \param protocol   protocol number to send packet.
    */
  typedef Callback<bool, Ptr<Packet>, const Address&, uint16_t > SendCallback;

  /**
    * \param beamId ID of the beam which for callback is set
    * \param cb callback to invoke whenever a TBTP is ready for sending and must
    *        be forwarded to the Beam UTs.
    */
  void Initialize (uint32_t beamId, SatBeamScheduler::SendCallback cb, Ptr<SatSuperframeSeq> seq);

  /**
    * \param utId ID (mac address) of the UT to be added
    */
  void AddUt (Address utId);

private:

  SatBeamScheduler& operator = (const SatBeamScheduler &);
  SatBeamScheduler (const SatBeamScheduler &);

  void DoDispose (void);
  bool Send ( Ptr<Packet> packet );
  void Schedule ();

  Ptr<SatSuperframeSeq> m_superframeSeq;
  uint32_t m_superFrameCounter;

  /**
   * The TBTP send callback.
   */
  SatBeamScheduler::SendCallback m_txCallback;

  /**
   * Set to store UTs in beam.
   */
  std::set<Address> m_uts;
  Address m_currentUtAddress;

  /**
   * ID of the beam
   */
  uint32_t m_beamId;
};

} // namespace ns3

#endif /* SAT_BEAM_SCHEDULER_H */
