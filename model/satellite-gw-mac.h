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

#ifndef SAT_GW_MAC_H
#define SAT_GW_MAC_H

#include <ns3/ptr.h>
#include <ns3/callback.h>
#include <ns3/traced-callback.h>
#include <ns3/nstime.h>

#include "satellite-mac.h"
#include "satellite-phy.h"


namespace ns3 {

class Packet;
class Address;
class Mac48Address;
class SatBbFrame;
class SatCrMessage;
class SatSignalParameters;
class SatFwdLinkScheduler;

/**
 * \ingroup satellite
  * \brief GW specific Mac class for Sat Net Devices.
 *
 * This SatGwMac class specializes the Mac class with GW characteristics.
 */

class SatGwMac : public SatMac
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor, which is not used.
   */
  SatGwMac ();

  /**
   * Construct a SatGwMac
   *
   * This is the constructor for the SatGwMac
   *
   * \param satId ID of sat for UT
   * \param beamId ID of beam for UT
   * \param forwardLinkRegenerationMode Forward link regeneration mode
   * \param returnLinkRegenerationMode Return link regeneration mode
   */
  SatGwMac (uint32_t satId,
            uint32_t beamId,
            SatEnums::RegenerationMode_t forwardLinkRegenerationMode,
            SatEnums::RegenerationMode_t returnLinkRegenerationMode);

  /**
   * Destroy a SatGwMac
   *
   * This is the destructor for the SatGwMac.
   */
  ~SatGwMac ();

  /**
   * Starts periodical transmissions. Called when MAC is wanted to take care of periodic sending.
   */
  void StartPeriodicTransmissions ();

  /**
   * Receive packet from lower layer.
   *
   * \param packets Pointers to packets received.
   */
  void Receive (SatPhy::PacketContainer_t packets, Ptr<SatSignalParameters> /*rxParams*/);

  /**
   * Function called when a TBTP has been sent by the SatBeamScheduler.
   * \param tbtp The TBTP sent by the scheduler.
   */
  void TbtpSent (Ptr<SatTbtpMessage> tbtp);

  /**
   * Callback to receive capacity request (CR) messages.
   * \param uint32_t The satellite ID.
   * \param uint32_t The beam ID.
   * \param Address Address of the sender UT.
   * \param Ptr<SatControlMessage> Pointer to the received CR message.
   */
  typedef Callback<void, uint32_t, uint32_t, Address, Ptr<SatCrMessage> > CrReceiveCallback;

  /**
   * Method to set read control message callback.
   * \param cb callback to invoke whenever a control message is wanted to read.
   */
  void SetCrReceiveCallback (SatGwMac::CrReceiveCallback cb);

  /**
   * Callback to notify upper layer about Tx opportunity.
   * \param uint32_t payload size in bytes
   * \param Mac48Address address
   * \return packet Packet to be transmitted to PHY
   */
  typedef Callback< Ptr<Packet>, uint32_t, Mac48Address, uint32_t& > TxOpportunityCallback;

  /**
   * Method to set Tx opportunity callback.
   * \param cb callback to invoke whenever a packet has been received and must
   *        be forwarded to the higher layers.
   */
  void SetTxOpportunityCallback (SatGwMac::TxOpportunityCallback cb);

  /**
   * Callback to query/apply handover on the terrestrial network
   * \param Address identification of the UT originating the request
   * \param uint32_t satellite ID
   * \param uint32_t source beam ID the UT is still in
   * \param uint32_t destination beam ID the UT would like to go in
   */
  typedef Callback<void, Address, uint32_t, uint32_t, uint32_t> HandoverCallback;

  /**
   * Method to set handover callback
   * \param cb callback to invoke whenever a handover recommendation is received
   */
  void SetHandoverCallback (SatGwMac::HandoverCallback cb);

  /**
   * Callback to register UT logon
   * \param Address identification of the UT originating the request
   * \param uint32_t sat ID the UT is requesting logon on
   * \param uint32_t beam ID the UT is requesting logon on
   * \param Callback setRaChannelCallback the callback to call when RA channel has been selected
   */
  typedef Callback<void, Address, uint32_t, uint32_t, Callback<void, uint32_t> > LogonCallback;

  /**
   * Method to set logon callback
   * \param cb callback to invoke whenever a logon is received
   */
  void SetLogonCallback (SatGwMac::LogonCallback cb);

  /**
   * Callback to inform NCC a control burst has been received.
   * \param Address identification of the UT that sent the burst
   * \param uint32_t satellite ID where the UT is connected
   * \param uint32_t beam ID where the UT is connected
   */
  typedef Callback<void, Address, uint32_t, uint32_t> ControlMessageReceivedCallback;

  /**
   * Method to set callback for control burst reception
   * \param cb callback to invoke whenever a control burst is received
   */
  void SetControlMessageReceivedCallback (SatGwMac::ControlMessageReceivedCallback cb);

  /**
   * Callback to indicate NCC a UT needs to be removed
   * \param Address identification of the UT to remove
   * \param uint32_t satellite ID where the UT is connected
   * \param uint32_t beam ID where the UT is connected
   */
  typedef Callback<void, Address, uint32_t, uint32_t> RemoveUtCallback;

  /**
   * Method to set callback for UT removing
   * \param cb callback to invoke whenever a UT needs to be removed
   */
  void SetRemoveUtCallback (SatGwMac::RemoveUtCallback cb);

  /**
   * Method to set forward link scheduler
   * \param The scheduler to use
   */
  void SetFwdScheduler (Ptr<SatFwdLinkScheduler> fwdScheduler);

private:
  SatGwMac& operator = (const SatGwMac &);
  SatGwMac (const SatGwMac &);

  void DoDispose (void);

  /**
   * Start sending a Packet Down the Wire.
   *
   * The StartTransmission method is used internally in the
   * SatGwMac to begin the process of sending a packet out on the PHY layer.
   *
   * \param carrierId id of the carrier.
   * \returns true if success, false on failure
   */
  void StartTransmission (uint32_t carrierId);

  /**
   * Send a NCR packet to the UTs. This method periodically calls itself.
   */
  void StartNcrTransmission ();

  /**
   * Signaling packet receiver, which handles all the signaling packet
   * receptions.
   * \param packet Received signaling packet
   * \param beamId ID of beam on UT
   */
  void ReceiveSignalingPacket (Ptr<Packet> packet, uint32_t satId, uint32_t beamId);

  void SendNcrMessage ();

  /**
   * Function used to clear old TBTP.
   * \param superframeCounter The SuperFrame counter to erase.
   */
  void RemoveTbtp (uint32_t superframeCounter);

  void SendCmtMessage (Address utId, Time burstDuration, Time satelliteReceptionTime, uint32_t satId,  uint32_t beamId);

  void SendLogonResponse (Address utId, uint32_t raChannel);
  static void SendLogonResponseHelper (SatGwMac* self,Address utId, uint32_t raChannel);

  /**
   * List of TBTPs sent to UTs. Key is superframe counter, value is TBTP.
   */
  std::map <uint32_t, std::vector<Ptr<SatTbtpMessage>>> m_tbtps;

  /**
   * Scheduler for the forward link.
   */
  Ptr<SatFwdLinkScheduler> m_fwdScheduler;

  /**
   * Guard time for BB frames. The guard time is modeled by shortening
   * the duration of a BB frame by a m_guardTime set by an attribute.
   */
  Time m_guardTime;

  /**
   * Interval between two broadcast of NCR dates
   */
  Time m_ncrInterval;

  /**
   * Use CMT control messages to correct time on the UTs
   */
  bool m_useCmt;

  /**
   * Time of last CMT sending for each UT
   */
  std::map<Address, Time> m_lastCmtSent;

  /**
   * Minimum interval between two CMT control messages for a same UT
   */
  Time m_cmtPeriodMin;

  /**
   * Broadcast NCR messages to all UTs
   */
  bool m_broadcastNcr;

  /**
   * Trace for transmitted BB frames.
   */
  TracedCallback<Ptr<SatBbFrame>> m_bbFrameTxTrace;

  /**
   * Capacity request receive callback.
   */
  SatGwMac::CrReceiveCallback  m_crReceiveCallback;

  /**
   * Callback to notify the txOpportunity to upper layer
   * Returns a packet
   * Attributes: payload in bytes
   */
  SatGwMac::TxOpportunityCallback m_txOpportunityCallback;

  /**
   * Callback to query/apply handover on the terrestrial network
   */
  SatGwMac::HandoverCallback m_handoverCallback;

  /**
   * Callback to log a terminal on
   */
  SatGwMac::LogonCallback m_logonCallback;

  /**
   * Callback to indicate NCC a control burst has been received
   */
  SatGwMac::ControlMessageReceivedCallback m_controlMessageReceivedCallback;

  /**
   * Callback to indicate NCC a UT needs to be removed
   */
  SatGwMac::RemoveUtCallback m_removeUtCallback;
};

} // namespace ns3

#endif /* SAT_GW_MAC_H */
