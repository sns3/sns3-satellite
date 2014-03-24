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


#ifndef SATELLITE_PHY_RX_CARRIER_H
#define SATELLITE_PHY_RX_CARRIER_H

#include "ns3/object.h"

#include "satellite-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-signal-parameters.h"
#include "satellite-channel.h"
#include "satellite-interference.h"
#include "satellite-phy-rx-carrier-conf.h"
#include <map>
#include <list>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * The SatPhyRxCarrier models the physical layer receiver of satellite system. There
 * are one SatPhyRxCarrier receiver for each carrier in both forward and return links.
 */

class SatPhyRxCarrier : public Object
{
public:

  /**
   * \brief Struct for storing the packet specific Rx parameters
   */
  typedef struct
  {
    Ptr<SatSignalParameters> rxParams;
    Mac48Address destAddress;
    Mac48Address sourceAddress;
    Ptr<SatInterference::InterferenceChangeEvent> interferenceEvent;
  } rxParams_s;

  /**
   * \brief Struct for storing the CRDSA packet specific Rx parameters
   */
  typedef struct
  {
    Ptr<SatSignalParameters> rxParams;
    Mac48Address destAddress;
    Mac48Address sourceAddress;
    uint16_t ownSlotId;
    std::vector<uint16_t> slotIdsForOtherReplicas;
    bool hasCollision;
    bool packetHasBeenProcessed;
    double cSinr;
    double ifPower;
    bool phyError;
  } crdsaPacketRxParams_s;

  /**
   * \brief
   * \param carrierId
   * \param carrierConf
   */
  SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf);

  /**
   * \brief
   */
  virtual ~SatPhyRxCarrier ();

  /**
   * \brief PHY states
   */
  enum State
  {
    IDLE, RX
  };

  /**
   * \brief
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   * \brief
   */
  virtual void DoDispose ();

  /**
  * \brief Set the SatPhy module
  * \param phy PHY module
  */
  void SetPhy (Ptr<SatPhy> phy);

  /**
   * \brief Set the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint32_t beamId);

  /**
   * \brief Set the node info class
   * \param nodeInfo Node information related to this SatPhyRxCarrier
   */
  void SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo);

  /**
   * \brief Start packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);

  /**
   * \brief
   * \param cb
   */
  void SetReceiveCb(SatPhyRx::ReceiveCallback cb);

  /**
   * \brief
   * \param cb
   */
  void SetCnoCb(SatPhyRx::CnoCallback cb);

private:

  /**
   * \brief Boltzmann Constant
   */
  static const double BoltzmannConstant = 1.3806488e-23;

  /**
   * \brief Function for composite SINR output tracing
   * \param cSinr composite SINR
   */
  void DoCompositeSinrOutputTrace (double cSinr);

  /**
   * \brief
   * \param newState
   */
  void ChangeState (State newState);

  /**
   * \brief Function for checking the SINR against the link results
   * \param cSinr composite SINR
   * \param rxParams Rx parameters
   * \return result of the check
   */
  bool CheckAgainstLinkResults (double cSinr, Ptr<SatSignalParameters> rxParams);

  /**
   * \brief
   * \param key
   */
  void EndRxData (uint32_t key);

  /**
   *
   * \param key
   */
  void EndRxDataTransparent (uint32_t key);

  /**
   *
   * \param key
   */
  void EndRxDataNormal (uint32_t key);

  /**
   *
   */
  void DoFrameEnd ();

  /**
   *
   * \param rxPowerW
   * \param ifPowerW
   * \param rxNoisePowerW
   * \param rxAciIfPowerW
   * \param rxExtNoisePowerW
   * \return
   */
  double CalculateSinr (double rxPowerW,
                        double ifPowerW,
                        double rxNoisePowerW,
                        double rxAciIfPowerW,
                        double rxExtNoisePowerW,
                        SatPhyRxCarrierConf::SinrCalculatorCallback sinrCalculate);

  /**
   * \brief
   * \param sinr1
   * \param sinr2
   * \return
   */
  double CalculateCompositeSinr(double sinr1, double sinr2);

  /**
   *
   *\param packetType
   */
  void IncreaseNumOfRxState (SatEnums::PacketType_t packetType);

  /**
   *
   * \param packetType
   */
  void DecreaseNumOfRxState (SatEnums::PacketType_t packetType);

  /**
   *
   */
  void CheckRxStateSanity ();

  /**
   * \brief
   * \param
   */
  void AddCrdsaPacket (SatPhyRxCarrier::crdsaPacketRxParams_s rxParams);

  /**
   * \brief
   * \return
   */
  std::vector<SatPhyRxCarrier::crdsaPacketRxParams_s> ProcessFrame ();

  /**
   *
   * \param packet
   * \param numOfPacketsForThisSlot
   * \return
   */
  SatPhyRxCarrier::crdsaPacketRxParams_s ProcessReceivedCrdsaPacket (SatPhyRxCarrier::crdsaPacketRxParams_s packet,
                                                                     uint32_t numOfPacketsForThisSlot);

  /**
   *
   * \param
   */
  void FindAndRemoveReplicas (SatPhyRxCarrier::crdsaPacketRxParams_s);

  /**
   *
   * \param packet
   * \param iter
   * \return
   */
  bool IsReplica (SatPhyRxCarrier::crdsaPacketRxParams_s packet, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iter);

  /**
   *
   * \param packet
   * \param iter
   * \return
   */
  bool HaveSameSlotIds (SatPhyRxCarrier::crdsaPacketRxParams_s packet, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iter);

  /**
   *
   * \param cSinr
   * \param rxParams
   * \param interferenceEvent
   * \return
   */
  bool ProcessSlottedAlohaCollisions (double cSinr,
                                      Ptr<SatSignalParameters> rxParams,
                                      Ptr<SatInterference::InterferenceChangeEvent> interferenceEvent);

  /**
   * \brief
   */
  std::map<uint32_t, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> > m_crdsaPacketContainer;

  /**
   * \brief
   */
  State m_state;

  /**
   * \brief
   */
  bool m_receivingDedicatedAccess;

  /**
   * \brief
   */
  uint32_t m_beamId;

  /**
   * \brief
   */
  uint32_t m_carrierId;

  /**
   * \brief
   * Interference model:
   * - Constant
   * - Per-packet
   * - Traced
   */
  Ptr<SatInterference> m_satInterference;

  /**
   * \brief Link results used for error modeling
   */
  Ptr<SatLinkResults> m_linkResults;

  /**
   * \brief RX noise temperature in K.
   */
  double m_rxTemperatureK;

  /**
   * \brief RX Bandwidth in Hz
   */
  double m_rxBandwidthHz;

  /**
   * \brief External noise power  system RX noise
   */
  double m_rxExtNoisePowerW;

  /**
   * \brief RX noise
   */
  double m_rxNoisePowerW;

  /**
   * \brief RX Adjacent channel interference
   */
  double m_rxAciIfPowerW;

  /**
   * \brief The upper layer package receive callback.
   */
  SatPhyRx::ReceiveCallback m_rxCallback;

  /**
   * \brief The upper layer C/N0 receive callback.
   */
  SatPhy::CnoCallback m_cnoCallback;

  /**
   * \brief Address of the device owning this object.
   */
  Mac48Address m_ownAddress;

  /**
   * \brief Receiving mode.
   */
  SatPhyRxCarrierConf::RxMode m_rxMode;

  /**
   * \brief Error model.
   */
  SatPhyRxCarrierConf::ErrorModel m_errorModel;

  /**
   * \brief Channel type.
   */
  SatEnums::ChannelType_t m_channelType;

  /**
   * \brief Callback to calculate SINR.
   */
  SatPhyRxCarrierConf::SinrCalculatorCallback m_sinrCalculate;

  /**
   * \brief Error rate for constant error model
   */
  double m_constantErrorRate;

  /**
   * \brief The trace source fired for added interference
   *
   * \see class CallBackTraceSource
   */
  TracedCallback< Ptr<SatSignalParameters>, // RX signalling parameters
                  Mac48Address,             // receiver address
                  Mac48Address,             // packet destination address
                  double,                   // interference power
                  double                    // composite SINR
                  >
     m_packetTrace;

  /**
   * \brief Enable composite SINR output tracing
   */
  bool m_enableCompositeSinrOutputTrace;

  /**
   * \brief A random variable for packet reception
   */
  Ptr<UniformRandomVariable> m_uniformVariable;

  /**
   * \brief A node info class containing node related
   * information, such as node id and address.
   */
  Ptr<SatNodeInfo> m_nodeInfo;

  /**
   * \brief Contains information about how many ongoing Rx events there are
   */
  uint32_t m_numOfOngoingRx;

  /**
   * \brief Running counter for received packets
   */
  uint32_t m_rxPacketCounter;

  /**
   * \brief A map of Rx params
   */
  std::map <uint32_t, SatPhyRxCarrier::rxParams_s> m_rxParamsMap;

  /**
   * \brief Channel estimation error container
   */
  Ptr<SatChannelEstimationErrorContainer> m_channelEstimationError;

  /**
   *
   */
  bool m_dropCollidingRandomAccessPackets;
};

}

#endif /* SATELLITE_PHY_RX_CARRIER_H */
