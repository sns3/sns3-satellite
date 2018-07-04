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
#include <ns3/satellite-mac.h>
#include <ns3/satellite-phy.h>

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
   * Default construct of SatGwMac. Should not been used.
   *
   * This is the constructor for the SatGwMac
   *
   */
  SatGwMac ();

  /**
   * Construct a SatGwMac
   *
   * This is the constructor for the SatGwMac
   *
   */
  SatGwMac (uint32_t beamId);

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
   * Callback to receive capacity request (CR) messages.
   * \param uint32_t The beam ID.
   * \param Address Address of the sender UT.
   * \param Ptr<SatControlMessage> Pointer to the received CR message.
   */
  typedef Callback<void, uint32_t, Address, Ptr<SatCrMessage> > CrReceiveCallback;

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
   * Signaling packet receiver, which handles all the signaling packet
   * receptions.
   * \param packet Received signaling packet
   */
  void ReceiveSignalingPacket (Ptr<Packet> packet);

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
   * Trace for transmitted BB frames.
   */
  TracedCallback<SatEnums::SatBbFrameType_t> m_bbFrameTxTrace;

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
};

} // namespace ns3

#endif /* SAT_GW_MAC_H */
