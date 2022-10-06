/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#ifndef SAT_NCC_H
#define SAT_NCC_H

#include <map>
#include <utility>

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>

#include "satellite-beam-scheduler.h"


namespace ns3 {

class Packet;
class Address;
class SatCrMessage;
class SatSuperframeSeq;
class SatLowerLayerServiceConf;

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
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
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
   * \brief Update UT specific C/N0 information.
   *
   * The SatNcc receives C/N0 information of packet receptions from UTs
   * to take into account when making schedule decisions.
   *
   * \param beamId  The id of the beam where C/N0 is from.
   * \param sourceMac  The MAC of the UT or SAT. (sender address)
   * \param gwId  The id (address) of the GW. (receiver address)
   * \param cno Value of the C/N0.
   * \param isSatelliteMac If true, cno corresponds to link SAT to GW; if false, cno corresponds to link UT to GW
   */
  void UtCnoUpdated (uint32_t beamId, Address sourceMac, Address gwId, double cno, bool isSatelliteMac);

  /**
   * \brief Function for adjusting the random access allocation channel specific load
   * \param beamId Beam ID
   * \param carrierId Carrier ID
   * \param allocationChannelId Allocation channel ID
   * \param averageNormalizedOfferedLoad Measured average normalized offered load
   */
  void DoRandomAccessDynamicLoadControl (uint32_t beamId, uint32_t carrierId, uint8_t allocationChannelId, double averageNormalizedOfferedLoad);

  /**
   * \brief Capacity request receiver.
   *
   * The SatNcc receives Capacity Rrequest (CR) messages from UTs
   * to take into account when making schedule decisions.
   *
   * \param beamId  The id of the beam where C/N0 is from.
   * \param utId  The id of the UT. (sender address)
   * \param crMsg Pointer to received Capacity Request
   */
  void UtCrReceived (uint32_t beamId, Address utId, Ptr<SatCrMessage> crMsg);

  /**
   * Define type SendCallback
   */
  typedef SatBeamScheduler::SendCtrlMsgCallback SendCallback;

  /**
   * \brief Function for adding the beam
   * \param beamId ID of the beam which for callback is set
   * \param cb callback to invoke whenever a TBTP is ready for sending and must
   *        be forwarded to the Beam UTs.
   * \param seq Super frame sequence
   * \param maxFrameSizeInBytes Maximum non fragmented BB frame size with most robust ModCod
   * \param gwAddress Mac address of the gateway responsible for this beam
   */
  void AddBeam (uint32_t beamId, SatNcc::SendCallback cb, Ptr<SatSuperframeSeq> seq, uint32_t maxFrameSizeInBytes, Address gwAddress);

  /**
   * \brief Function for adding the UT
   * \param utId ID (mac address) of the UT to be added
   * \param llsConf Lower layer service configuration for the UT to be added.
   * \param beamId ID of the beam where UT is connected.
   * \param setRaChannelCallback  callback to invoke whenever the UT to be
   *        added should change its RA allocation channel
   */
  void AddUt (Ptr<SatLowerLayerServiceConf> llsConf, Address utId, uint32_t beamId, Callback<void, uint32_t> setRaChannelCallback, bool verifyExisting = false);

  /**
   * Remove a UT
   * \param utId ID (mac address) of the UT to be removed
   * \param beamId ID of the beam where UT is connected
   */
  void RemoveUt (Address utId, uint32_t beamId);

  /**
   * \brief Function for setting the random access allocation channel specific high load backoff probabilities
   * \param allocationChannelId Allocation channel ID
   * \param threshold Average normalized offered load threshold
   */
  void SetRandomAccessAverageNormalizedOfferedLoadThreshold (uint8_t allocationChannelId, double threshold);

  /**
   * \brief Function for setting the random access allocation channel specific high load backoff probabilities
   * \param allocationChannelId Allocation channel ID
   * \param lowLoadBackOffProbability Low load backoff probability
   */
  void SetRandomAccessLowLoadBackoffProbability (uint8_t allocationChannelId, uint16_t lowLoadBackOffProbability);

  /**
   * \brief Function for setting the random access allocation channel specific high load backoff probabilities
   * \param allocationChannelId Allocation channel ID
   * \param highLoadBackOffProbability High load backoff probability
   */
  void SetRandomAccessHighLoadBackoffProbability (uint8_t allocationChannelId, uint16_t highLoadBackOffProbability);

  /**
   * \brief Function for setting the random access allocation channel specific high load backoff time
   * \param allocationChannelId Allocation channel ID
   * \param lowLoadBackOffTime Low load backoff time
   */
  void SetRandomAccessLowLoadBackoffTime (uint8_t allocationChannelId, uint16_t lowLoadBackOffTime);

  /**
   * \brief Function for setting the random access allocation channel specific high load backoff time
   * \param allocationChannelId Allocation channel ID
   * \param highLoadBackOffTime High load backoff time
   */
  void SetRandomAccessHighLoadBackoffTime (uint8_t allocationChannelId, uint16_t highLoadBackOffTime);

  /**
   * \param beamId the ID of the beam.
   * \return pointer to the beam scheduler, or zero if the beam is not found.
   */
  Ptr<SatBeamScheduler> GetBeamScheduler (uint32_t beamId) const;

  /**
   * \brief Check if a terminal can be moved between two beams. If yes, schedule
   * the actual move at a later point in time.
   * \param utId the UT wanting to move between beams
   * \param srcBeamId the beam ID this UT is moving from
   * \param destBeamId the beam ID this UT is moving to
   */
  void MoveUtBetweenBeams (Address utId, uint32_t srcBeamId, uint32_t destBeamId);

  /**
   * \brief Update routes and ARP tables on gateways after a terminal handover
   * \param Address address of the UT whose handover is completed
   * \param Address address of the GW handling this UT before handover
   * \param Address address of the GW handling this UT after handover
   */
  typedef Callback<void, Address, Address, Address> UpdateRoutingCallback;

  /**
   * \brief Set the callback used to update routes and APR tables after a terminal handover
   * \param cb the routing update callback
   */
  void SetUpdateRoutingCallback (SatNcc::UpdateRoutingCallback cb);

  /**
   * \param msg        the TBTP sent
   */
  typedef Callback<void, Ptr<SatTbtpMessage> > SendTbtpCallback;

  /**
   * Set the callback to inform GW Mac a TBTP has been sent.
   */
  void SetSendTbtpCallback (SendTbtpCallback cb);

  void ReserveLogonChannel (uint32_t logonChannelId);

  /**
   * Function called when a TBTP has been sent by the SatBeamScheduler.
   */
  void TbtpSent (Ptr<SatTbtpMessage> tbtp);

  /**
   * Function to call when a control burst has been received.
   * \param utId The address of the sending UT
   * \param beamId The beam ID
   */
  void ReceiveControlBurst (Address utId, uint32_t beamId);

  /**
   * Set if logon is used in this simulation. Logoff is disbled if logon is not used.
   * \param useLogon boolean indicating if logon is used.
   */
  void SetUseLogon (bool useLogon);

  /**
   * Set if SNS-3 is used with Lora standard. TBTPs are not sent in this mode.
   * \param useLora boolean indicating if lora is used.
   */
  void SetUseLora (bool useLora);

private:
  SatNcc& operator = (const SatNcc &);
  SatNcc (const SatNcc &);

  void DoDispose (void);

  /**
   * \brief Function for creating the random access control message
   * \param backoffProbability Backoff probability
   * \param backoffProbability Backoff Time
   * \param beamId Beam ID
   * \param allocationChannelId Allocation channel ID
   */
  void CreateRandomAccessLoadControlMessage (uint16_t backoffProbability, uint16_t backoffTime, uint32_t beamId, uint8_t allocationChannelId);

  /**
   * \brief Perform terminal handover on the terestrial network
   * \param utId the UT moving between beams
   * \param srcBeamId the beam ID this UT is moving from
   * \param destBeamId the beam ID this UT is moving to
   */
  void DoMoveUtBetweenBeams (Address utId, uint32_t srcBeamId, uint32_t destBeamId);

  /**
   * \brief Check if a UT has not been receiving control bursts, and then need to logoff
   * \param utId The UT to check
   * \param beamId The beam ID
   */
  void CheckTimeout (Address utId, uint32_t beamId);

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
   * Map for keeping track of the load status of each random access allocation channel
   */
  std::map<std::pair<uint32_t, uint8_t>, bool> m_isLowRandomAccessLoad;

  /**
   * Map for random access allocation channel specific load thresholds
   */
  std::map<uint8_t, double> m_randomAccessAverageNormalizedOfferedLoadThreshold;

  /**
   * Map for random access allocation channel specific low load backoff probabilities
   */
  std::map<uint8_t, uint16_t> m_lowLoadBackOffProbability;

  /**
   * Map for random access allocation channel specific high load backoff probabilities
   */
  std::map<uint8_t, uint16_t> m_highLoadBackOffProbability;

  /**
   * Map for random access allocation channel specific low load backoff time
   */
  std::map<uint8_t, uint16_t> m_lowLoadBackOffTime;

  /**
   * Map for random access allocation channel specific high load backoff time
   */
  std::map<uint8_t, uint16_t> m_highLoadBackOffTime;

  /**
   * Delay between handover acceptance and effective information transfer
   */
  Time m_utHandoverDelay;

  /**
   * Timeout to logoff a UT, if logon procedure is used
   */
  Time m_utTimeout;

  /**
   * Flag indicating if logon procedure is used
   */
  bool m_useLogon;

  /**
   * Flag indicating if lora standard is used
   */
  bool m_useLora;

  /**
   * List of reception time for all UTs. Used to trigger timeouts and logoff UTs.
   */
  std::map< std::pair<Address, uint32_t> , Time> m_lastControlBurstReception;

  /**
   * Callback to update routing tables and ARP tables on gateways
   * once a handover request has been accepted and treated
   */
  UpdateRoutingCallback m_updateRoutingCallback;

  /**
   * The TBTP send callback to inform GW Mac.
   */
  SatBeamScheduler::SendTbtpCallback m_txTbtpCallback;
};

} // namespace ns3

#endif /* SAT_NCC_H */
