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
#include <ns3/satellite-utils.h>
#include <ns3/satellite-interference.h>
#include <ns3/satellite-phy.h>
#include <ns3/satellite-phy-rx.h>
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
 * \brief Base class for all SatPhyRxCarriers.
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

  /**
   * Constructor
   * \param carrierId
   * \param carrierConf
   * \param isRandomAccessEnabled
   */
  SatPhyRxCarrier (uint32_t carrierId, Ptr<SatPhyRxCarrierConf> carrierConf, bool isRandomAccessEnabled);

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
   * \brief Possible carrier types
   */
  enum CarrierType
	{
  	BASE, DEDICATED_ACCESS, RA_SLOTTED_ALOHA, RA_CRDSA
	};

  /**
   * \brief Function for gettign the NS-3 type ID
   * \return
   */
  static TypeId GetTypeId (void);

  /**
  * \brief Function for setting the SatPhy module
  * \param phy PHY module
  */
  void SetPhy (Ptr<SatPhy> phy);

  /**
   * \brief Function for setting the beam id for all the transmissions from this SatPhyTx
   * \param beamId the Beam Identifier
   */
  inline void SetBeamId (uint32_t beamId) { m_beamId = beamId; };

  /**
   * \brief Get ID the ID of the beam this carrier is attached to
   * \return Beam ID
   */
  inline uint32_t GetBeamId () { return m_beamId; };


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
   * \brief Method for querying the type of the carrier
   */
  inline virtual CarrierType GetCarrierType () { return BASE; }

  //////////// Set callbacks ///////////////

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

protected:

  /**
   * Get the default receive mode for the carrier. In satellite nodes, this will
   * be overrided to always receive packets.
   * \return True or false, false by default in base class
   */
  inline virtual const bool GetDefaultReceiveMode () { return false; };

  /**
   * \brief Check if the carrier is receiving a dedicated access packet
   * \return true or false
   */
  inline bool IsReceivingDedicatedAccess () { return m_receivingDedicatedAccess; };

  /**
   * \brief Get pointer to the current interference model.
   * \return interference model
   */
  inline Ptr<SatInterference> GetInterferenceModel () { return m_satInterference; };

  /**
   * \brief Create an interference event based on Rx parameters and address.
   * 				Implemented by child classes.
   *
   * \return Pointer to the interference event.
   */
	virtual Ptr<SatInterference::InterferenceChangeEvent>
	  CreateInterference (Ptr<SatSignalParameters> rxParams, Address rxAddress) = 0;

	/**
	 * Rx parameter storage methods
	 */

	/**
	 * Get receive parameters from signal parameters.
	 * \param rxParams SatSignalParameters
	 * \return A pair of boolean and rxParams_s struct. Boolean tells if we are about to receive a packet
	 * 				 and struct contains all receiveing info.
	 */
	std::pair<bool, SatPhyRxCarrier::rxParams_s> GetReceiveParams (Ptr<SatSignalParameters> rxParams);

	/// Get stored rxParams under a key
  inline rxParams_s GetStoredRxParams (uint32_t key) { return m_rxParamsMap[key]; }

  /// Store rxParams under a key
  inline void StoreRxParams (uint32_t key, rxParams_s rxParams) { m_rxParamsMap[key] = rxParams; }

  /// Remove stored rxParams under a key
  inline void RemoveStoredRxParams (uint32_t key) { m_rxParamsMap.erase (key); }

  /**
   * Get the MAC address of the carrier
   * \return MAC address
   */
  inline Mac48Address GetOwnAddress () { return m_ownAddress; };

  /**
   * Get the satellite node info.
   * \return SatNodeInfo pointer
   */
  inline Ptr<SatNodeInfo> GetNodeInfo () { return m_nodeInfo; };

  /**
   * \brief Function for composite SINR output tracing
   * \param cSinr composite SINR
   */
  void DoCompositeSinrOutputTrace (double cSinr);

  /**
   * Create an interference model for this carrier.
   * \param carrierConf
   * \param carrierId
   * \param rxBandwidthHz
   * \param randomAccessEnabled
   */
  virtual void DoCreateInterferenceModel (Ptr<SatPhyRxCarrierConf> carrierConf, uint32_t carrierId, double rxBandwidthHz);

  /**
   * \brief Get the channel type. Base class has undefined channel type, while child classes have
   * 				can have other channel types.
   * \return Channel type
   */
  inline virtual SatEnums::ChannelType_t GetChannelType () { return m_channelType; };

  /**
   * \brief Set the channel type for the carrier
   * \param channelType Channel type.
   */
  inline void SetChannelType (SatEnums::ChannelType_t channelType) { m_channelType = channelType; };

  /**
   * \brief Function for changing the receiver state
   * \param newState New state
   */
  void ChangeState (State newState);

  /**
   * \brief Get the state of the carrier
   * \return State of the carrier.
   */
  inline State GetState () { return m_state; }

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
  virtual void EndRxData (uint32_t key) = 0;

  /**
   * Is random access enabled for this carrier.
   */
  const bool m_randomAccessEnabled;

  /**
   * \brief Dispose
   */
  virtual void DoDispose ();

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
   * \brief Get the ID of the carrier
   * \return Carrier ID
   */
  inline uint32_t GetCarrierId () const { return m_carrierId; }

  /**
   * \brief Get pointer to the link results given by the carrier
   * 				creation configuration.
   * \return Link results
   */
  inline Ptr<SatLinkResults> GetLinkResults () { return m_linkResults; };

  /**
   * \brief Get a pointer to the channel estimation error container of the carrier.
   * \return channel estimation error containe pointer
   */
  inline Ptr<SatChannelEstimationErrorContainer> GetChannelEstimationErrorContainer ()
  {
  	return m_channelEstimationError;
  };

  /**
   * \brief Check if composite SINR output trace is enabled.
   */
  inline bool IsCompositeSinrOutputTraceEnabled () const { return m_enableCompositeSinrOutputTrace; };

  /**
   * \brief A helper method for getting values form a uniform random variable in child classes.
   * \param min Minimum value
   * \param max Maximum value
   * \return Double between min and max
   */
  inline double GetUniformRandomValue (double min, double max) { return m_uniformVariable->GetValue (min, max); };

  ///// CALCULATION VARIABLES //////////
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

  //////// TRACED CALLBACKS ////////////
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

  ////////////// CALLBACKS /////////////////////

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
   * \brief Callback to calculate SINR.
   */
  SatPhyRxCarrierConf::SinrCalculatorCallback m_sinrCalculate;

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

private:

  /**
   * \brief Function for checking the SINR against the link results
   * \param cSinr composite SINR
   * \param rxParams Rx parameters
   * \return result of the check
   */
  bool CheckAgainstLinkResultsErrorModelAvi (double cSinr, Ptr<SatSignalParameters> rxParams);

  State m_state; 																//< Current state of the carrier
  uint32_t m_beamId; 														//< Beam ID
  uint32_t m_carrierId; 												//< Carrier ID
  bool m_receivingDedicatedAccess; 							//< Is the carrier receiving a dedicated access packet
  Ptr<SatInterference> m_satInterference; 			//< Interference model
  bool m_enableCompositeSinrOutputTrace;				//< Enable composite SINR output tracing

  /**
   * \brief Contains information about how many ongoing Rx events there are
   */
  uint32_t m_numOfOngoingRx;

  /**
   * \brief Running counter for received packets
   */
  uint32_t m_rxPacketCounter;

  std::map<uint32_t, rxParams_s> m_rxParamsMap; //< Storage for Rx parameters by ID
  Mac48Address m_ownAddress; 										//< Carrier address
  Ptr<SatNodeInfo> m_nodeInfo; 									//< NodeInfo of the node where carrier is attached
  SatEnums::ChannelType_t m_channelType;				//< Channel type
  Ptr<SatLinkResults> m_linkResults; 						//< Link results from the carrier configuration
  Ptr<UniformRandomVariable> m_uniformVariable;	//< Uniform helper random variable
  SatPhyRxCarrierConf::ErrorModel m_errorModel;	//< Error model
  double m_constantErrorRate;										//< Error rate for constant error model

  /**
   * \brief Channel estimation error container
   */
  Ptr<SatChannelEstimationErrorContainer> m_channelEstimationError;

};

}

#endif /* SATELLITE_PHY_RX_CARRIER_H */
