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

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-interference.h>
#include <ns3/satellite-phy-rx-carrier-conf.h>
#include <vector>
#include <map>
#include <list>
#include <deque>

namespace ns3 {

class Address;
class SatPhy;
class SatSignalParameters;
class SatLinkResults;
class SatChannelEstimationErrorContainer;
class SatNodeInfo;


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
   * \brief Constructor
   * \param carrierId Carrier ID
   * \param carrierConf Carrier configuration object
   * \param isRandomAccessEnabledForThisCarrier Is random access enabled for this carrier
   */
  SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf, bool isRandomAccessEnabledForThisCarrier);

  /**
   * \brief Destructor
   */
  virtual ~SatPhyRxCarrier ();

  /**
   * \brief Enum for PHY states
   */
  enum State
  {
    IDLE, RX
  };

  /**
   * \brief Function for gettign the NS-3 type ID
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Dispose function
   */
  virtual void DoDispose ();

  /**
  * \brief Function for setting the SatPhy module
  * \param phy PHY module
  */
  void SetPhy (Ptr<SatPhy> phy);

  /**
   * \brief Function for setting the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  void SetBeamId (uint32_t beamId);

  /**
   * \brief Function for setting the node info class
   * \param nodeInfo Node information related to this SatPhyRxCarrier
   */
  void SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo);

  /**
   * \brief Function for starting packet reception from the SatChannel
   * \param rxParams The needed parameters for the received signal
   */
  void StartRx (Ptr<SatSignalParameters> rxParams);

  /**
   * \brief Function for setting the receive callback
   * \param cb callback
   */
  void SetReceiveCb (SatPhyRx::ReceiveCallback cb);

  /**
   * \brief Function for settign the C/NO callback
   * \param cb callback
   */
  void SetCnoCb (SatPhyRx::CnoCallback cb);

  /**
   * \brief Function for setting the AverageNormalizedOfferedLoadCallback callback
   * \param callback callback
   */
  void SetAverageNormalizedOfferedLoadCallback (SatPhyRx::AverageNormalizedOfferedLoadCallback callback);

  /**
   * \brief Function for comparing the CRDSA unique packet IDs
   * \param obj1 Comparison object 1
   * \param obj2 Comparison object 2
   * \return Comparison result
   */
  static bool CompareCrdsaPacketId (SatPhyRxCarrier::crdsaPacketRxParams_s obj1, SatPhyRxCarrier::crdsaPacketRxParams_s obj2);

  /**
   * \brief Function for initializing the frame end scheduling
   */
  void BeginFrameEndScheduling ();

  /**
   * \brief Function for setting the random access allocation channel ID
   * \param randomAccessAllocationChannelId
   */
  void SetRandomAccessAllocationChannelId (uint8_t randomAccessAllocationChannelId);

  /**
   * \brief Function for getting the random access allocation channel ID
   * \return randomAccessAllocationChannelId
   */
  uint8_t GetRandomAccessAllocationChannelId () const;

  /**
   * \brief Callback signature for `LinkBudgetTrace` trace source.
   * \param rxParams RX signalling parameters
   * \param receiverAddress receiver address
   * \param destinationAddress packet destination address
   * \param interference interference power (in W)
   * \param sinr composite SINR (in linear unit)
   */
  typedef void (*LinkBudgetTraceCallback)
    (Ptr<SatSignalParameters> rxParams,
    Mac48Address receiverAddress,
    Mac48Address destinationAddress,
    double interference,
    double sinr);

  /**
   * \brief Callback signature for `RxPowerTrace` trace source.
   * \param rxPower received signal power (in dbW)
   */
  typedef void (*RxPowerTraceCallback)(double rxPower);

  /**
   * \brief Callback signature for `LinkSinrTrace` trace source.
   * \param sinr link-specific SINR (in dB)
   */
  typedef void (*LinkSinrTraceCallback)(double sinr);

  /**
   * \brief Common callback signature for trace sources related to packets
   *        reception by PHY and its error.
   * \param nPackets number of upper layer packets in the received packet burst.
   * \param from the MAC48 address of the sender of the packets.
   * \param isError whether a PHY error has occurred.
   */
  typedef void (*PhyRxErrorCallback)
    (uint32_t nPackets, const Address &from, bool isError);

  /**
   * \brief Common callback signature for trace sources related to packets
   *        reception by PHY and its collision.
   * \param nPackets number of packets in the received packet burst.
   * \param from the MAC48 address of the sender of the packets.
   * \param isCollided whether a collision has occurred.
   */
  typedef void (*PhyRxCollisionCallback)
    (uint32_t nPackets, const Address &from, bool isCollided);

private:
  /**
   * \brief Function for composite SINR output tracing
   * \param cSinr composite SINR
   */
  void DoCompositeSinrOutputTrace (double cSinr);

  /**
   * \brief Function for changing the receiver state
   * \param newState New state
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
   * \brief Function for ending the packet reception from the SatChannel
   * \param key Key for Rx params map
   */
  void EndRxData (uint32_t key);

  /**
   * \brief Function for ending the packet reception from the SatChannel in the satellite
   * \param key Key for Rx params map
   */
  void EndRxDataTransparent (uint32_t key);

  /**
   * \brief Function for ending the packet reception from the SatChannel in the ground node
   * \param key Key for Rx params map
   */
  void EndRxDataNormal (uint32_t key);

  /**
   * \brief Function for processing the frame interval operations
   */
  void DoFrameEnd ();

  /**
   * \brief Function for calculating the SINR
   * \param rxPowerW Rx power in Watts
   * \param ifPowerW Interference power in Watts
   * \param rxNoisePowerW Rx noise power in Watts
   * \param rxAciIfPowerW Rx ACI power in Watts
   * \param rxExtNoisePowerW Rx external noise power in Watts
   * \param sinrCalculate SINR calculator callback
   * \return Calculated SINR
   */
  double CalculateSinr (double rxPowerW,
                        double ifPowerW,
                        double rxNoisePowerW,
                        double rxAciIfPowerW,
                        double rxExtNoisePowerW,
                        SatPhyRxCarrierConf::SinrCalculatorCallback sinrCalculate);

  /**
   * \brief Function for calculating the composite SINR
   * \param sinr1 SINR 1
   * \param sinr2 SINR 2
   * \return Composite SINR
   */
  double CalculateCompositeSinr (double sinr1, double sinr2);

  /**
   * \brief Function for increasing the number of ongoing transmissions
   * \param packetType Packet type of the ongoing transmission
   */
  void IncreaseNumOfRxState (SatEnums::PacketType_t packetType);

  /**
   * \brief Function for decreasing the number of ongoing transmissions
   * \param packetType Packet type of the ongoing transmission
   */
  void DecreaseNumOfRxState (SatEnums::PacketType_t packetType);

  /**
   * \brief Function for checking the Rx state sanity
   */
  void CheckRxStateSanity ();

  /**
   * \brief Funciton for storing the received CRDSA packets
   * \param Rx parameters of the packet
   */
  void AddCrdsaPacket (SatPhyRxCarrier::crdsaPacketRxParams_s rxParams);

  /**
   * \brief Function for processing the CRDSA frame
   * \return Processed packets
   */
  std::vector<SatPhyRxCarrier::crdsaPacketRxParams_s> ProcessFrame ();

  /**
   * \brief Function for processing the received CRDSA packets
   * \param packet Received packet
   * \param numOfPacketsForThisSlot Number of packets in this slot
   * \return Processed packet
   */
  SatPhyRxCarrier::crdsaPacketRxParams_s ProcessReceivedCrdsaPacket (SatPhyRxCarrier::crdsaPacketRxParams_s packet,
                                                                     uint32_t numOfPacketsForThisSlot);

  /**
   * \brief Function for finding and removing the replicas of the CRDSA packet
   * \param packet CRDSA packet
   */
  void FindAndRemoveReplicas (SatPhyRxCarrier::crdsaPacketRxParams_s packet);

  /**
   * \brief Function for eliminating the interference to other packets in the slot from the correctly received packet
   * \param iter Packets in the slot
   * \param processedPacket Correctly received processed packet
   */
  void EliminateInterference (std::map<uint32_t,std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> >::iterator iter, SatPhyRxCarrier::crdsaPacketRxParams_s processedPacket);

  /**
   * \brief Function for identifying whether the packet is a replica of another packet
   * \param packet Packet
   * \param iter A packet in certain slot
   * \return Is the packet a replica
   */
  bool IsReplica (SatPhyRxCarrier::crdsaPacketRxParams_s packet, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iter);

  /**
   * \brief Function for checking do the packets have identical slots
   * \param packet Packet
   * \param iter A packet in certain slot
   * \return Have the packets identical slots
   */
  bool HaveSameSlotIds (SatPhyRxCarrier::crdsaPacketRxParams_s packet, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s>::iterator iter);

  /**
   * \brief Function for processing the Slotted ALOHA collisions
   * \param cSinr Composite SINR
   * \param rxParams Rx parameters of the packet
   * \param interferenceEvent Interference event details
   * \return PHY error
   */
  bool ProcessSlottedAlohaCollisions (double cSinr,
                                      Ptr<SatSignalParameters> rxParams,
                                      Ptr<SatInterference::InterferenceChangeEvent> interferenceEvent);

  /**
   * \brief Function for measuring the random access load
   */
  void MeasureRandomAccessLoad ();

  /**
   * \brief Function for calculating the normalized offered random access load
   * \return Normalized offered load
   */
  double CalculateNormalizedOfferedRandomAccessLoad ();

  /**
   * \brief Function for saving the measured random access load
   * \param measuredRandomAccessLoad Measured random access load
   */
  void SaveMeasuredRandomAccessLoad (double measuredRandomAccessLoad);

  /**
   * \brief Function for calculating the average normalized offered random access load
   * \return Average normalized offered load
   */
  double CalculateAverageNormalizedOfferedRandomAccessLoad ();

  /**
   * \brief CRDSA packet container
   */
  std::map<uint32_t, std::list<SatPhyRxCarrier::crdsaPacketRxParams_s> > m_crdsaPacketContainer;

  /**
   * \brief Rx state
   */
  State m_state;

  /**
   * \brief Are we receiving dedicated access at this moment
   */
  bool m_receivingDedicatedAccess;

  /**
   * \brief Beam ID
   */
  uint32_t m_beamId;

  /**
   * \brief Carrier ID
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
   * \brief Average normalized offered load callback
   */
  SatPhy::AverageNormalizedOfferedLoadCallback m_avgNormalizedOfferedLoadCallback;

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
   * \brief The trace source on packet receptiong
   *
   * \see class CallBackTraceSource
   */
  TracedCallback< Ptr<SatSignalParameters>, // RX signalling parameters
                  Mac48Address,             // receiver address
                  Mac48Address,             // packet destination address
                  double,                   // interference power
                  double                    // composite SINR
                  >
  m_linkBudgetTrace;

  /**
   * \brief A callback for received signal power in dBW
   *
   */
  TracedCallback<double> m_rxPowerTrace;

  /**
   * \brief A callback for transmission composite SINR at UT (BBFrame) or GW
   *        (time slot).
   *
   * The first argument is the SINR in dB. The second argument is the address
   * of the node where the signal originates from.
   */
  TracedCallback<double, const Address &> m_sinrTrace;

  /**
   * \brief A callback for link specific SINR in dB.
   *
   */
  TracedCallback<double> m_linkSinrTrace;

  /**
   * \brief `DaRx` trace source.
   *
   * Fired when a packet burst is received through Dedicated Channel.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a PHY error has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_daRxTrace;

  /**
   * \brief `SlottedAlohaRxCollision` trace source.
   *
   * Fired when a packet burst is received through Random Access Slotted ALOHA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a collision has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_slottedAlohaRxCollisionTrace;

  /**
   * \brief `SlottedAlohaRxError` trace source.
   *
   * Fired when a packet burst is received through Random Access Slotted ALOHA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a PHY error has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_slottedAlohaRxErrorTrace;

  /**
   * \brief `CrdsaReplicaRx` trace source.
   *
   * Fired when a CRDSA packet replica is received through Random Access CRDSA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a collision has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_crdsaReplicaRxTrace;

  /**
   * \brief `CrdsaUniquePayloadRx` trace source.
   *
   * Fired when a unique CRDSA payload is received (after frame processing)
   * through Random Access CRDSA.
   *
   * Contains the following information:
   * - number of upper layer packets in the received packet burst;
   * - the MAC48 address of the sender; and
   * - whether a PHY error has occurred.
   */
  TracedCallback<uint32_t, const Address &, bool> m_crdsaUniquePayloadRxTrace;

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
   * \brief Random access collision model
   */
  SatPhyRxCarrierConf::RandomAccessCollisionModel m_randomAccessCollisionModel;

  /**
   * \brief Random access average normalized offered load measurement window size
   */
  uint32_t m_randomAccessAverageNormalizedOfferedLoadMeasurementWindowSize;

  /**
   * \brief Container for calculated normalized offered loads
   */
  std::deque<double> m_randomAccessDynamicLoadControlNormalizedOfferedLoad;

  /**
   * \brief Is random access enabled for this carrier
   */
  bool m_isRandomAccessEnabledForThisCarrier;

  /**
   * \brief Number of random access bits in the frame
   */
  uint32_t m_randomAccessBitsInFrame;

  /**
   * \brief Has the frame end scheduling been initialized
   */
  bool m_frameEndSchedulingInitialized;

  /**
   * \brief Random access allocation channel ID
   */
  uint8_t m_randomAccessAllocationChannelId;

  /**
   * \brief Enable random access dynamic load control
   */
  bool m_enableRandomAccessDynamicLoadControl;
};

}

#endif /* SATELLITE_PHY_RX_CARRIER_H */
