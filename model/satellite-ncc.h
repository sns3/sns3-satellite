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

#ifndef SAT_NCC_H
#define SAT_NCC_H

#include <map>

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/mac48-address.h"
#include "satellite-beam-scheduler.h"
#include "ns3/satellite-superframe-sequence.h"

namespace ns3 {

/**
 * \ingroup satellite
  * \brief class for module NCC used as shared module among Gateways (GWs).
 *
 * This SatNcc class implements NCC functionality in Satellite network. It is shared
 * module among GWs. Communication between NCC and GW is handled by callback functions.
 *
 */
class SatNcc : public Object
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Construct a SatNcc
   *
   * This is the constructor for the SatNcc
   *
   */
  SatNcc ();

  /**
   * Destroy a SatNcc
   *
   * This is the destructor for the SatNcc.
   */
  ~SatNcc ();

  /**
   * Receive a packet from a beam.
   *
   * The SatNcc receives CR packets from its connected GWs (CRs sent by UT)
   * and takes CRs into account when making schedule decisions.
   *
   * \param p       Pointer to the received CR packet.
   * \param beamId  The id of the beam where packet is from.
   */
  void Receive (Ptr<Packet> p, uint32_t beamId);

  /**
   * Update UT specific C/N0 information.
   *
   * The SatNcc receives C/N0 information of packet receptions from UTs
   * to take into account when making schedule decisions.
   *
   * \param beamId  The id of the beam where C/N0 is from.
   * \param utId  The id of the UT. (sender address)
   * \param gwId  The id (address) of the GW. (receiver address)
   * \param cno Value of the C/N0.
   */
  void UtCnoUpdated (uint32_t beamId, Address utId, Address gwId, double cno);

  /**
   *
   * \param beamId
   * \param carrierId
   * \param averageNormalizedOfferedLoad
   */
  void DoRandomAccessDynamicLoadControl (uint32_t beamId, uint32_t carrierId, double averageNormalizedOfferedLoad);

  /**
   * Capacity request receiver.
   *
   * The SatNcc receives Capacity Rrequest (CR) messages from UTs
   * to take into account when making schedule decisions.
   *
   * \param beamId  The id of the beam where C/N0 is from.
   * \param utId  The id of the UT. (sender address)
   * \param crMsg Pointer to received Capacity Request
   */
  void UtCrReceived (uint32_t beamId, Address utId, Ptr<SatCrMessage> crMsg);

  typedef SatBeamScheduler::SendCtrlMsgCallback SendCallback;

  /**
    * \param beamId ID of the beam which for callback is set
    * \param cb callback to invoke whenever a TBTP is ready for sending and must
    *        be forwarded to the Beam UTs.
    * \param seq Super frame sequence
    * \param maxRcCount Maximum number of RCs in use.
    * \param maxFrameSizeInBytes Maximum non fragmented BB frame size with most robust ModCod
    */
  void AddBeam (uint32_t beamId, SatNcc::SendCallback cb, Ptr<SatSuperframeSeq> seq, uint8_t macRcCount, uint32_t maxFrameSizeInBytes);

  /**
    * \param utId ID (mac address) of the UT to be added
    * \param llsConf Lower layer service configuration for the UT to be added.
    * \param beamId ID of the beam where UT is connected.
    *
    * \return RA channel index assigned to added UT.
    */
  uint32_t AddUt (Address utId, Ptr<SatLowerLayerServiceConf> llsConf, uint32_t beamId);

  /**
   *
   * \param threshold
   */
  void SetRandomAccessHighLoadThreshold (double threshold) { m_randomAccessHighLoadThreshold = threshold; }

  /**
   *
   * \param lowLoadBackOffProbability
   */
  void SetRandomAccessLowLoadBackoffProbability (uint16_t lowLoadBackOffProbability) { m_lowLoadBackOffProbability = lowLoadBackOffProbability; }

  /**
   *
   * \param highLoadBackOffProbability
   */
  void SetRandomAccessHighLoadBackoffProbability (uint16_t highLoadBackOffProbability) { m_highLoadBackOffProbability = highLoadBackOffProbability; }

private:

  SatNcc& operator = (const SatNcc &);
  SatNcc (const SatNcc &);

  void DoDispose (void);

  /**
   *
   * \param backoffProbability
   * \param beamId
   */
  void CreateRandomAccessLoadControlMessage (uint16_t backoffProbability, uint32_t beamId);

  /**
   * The map containing beams in use (set).
   */
  std::map<uint32_t, Ptr<SatBeamScheduler> > m_beamSchedulers;

  /**
   * The trace source fired for Capacity Requests (CRs) received by the NCC.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_nccRxTrace;


  /**
   * The trace source fired for TBTPs sent by the NCC.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_nccTxTrace;

  /**
   *
   */
  std::map<uint32_t,bool> m_isLowRandomAccessLoad;

  /**
   *
   */
  double m_randomAccessHighLoadThreshold;

  /**
   *
   */
  uint16_t m_lowLoadBackOffProbability;

  /**
   *
   */
  uint16_t m_highLoadBackOffProbability;
};

} // namespace ns3

#endif /* SAT_NCC_H */
