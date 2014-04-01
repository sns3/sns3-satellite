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

#ifndef SATELLITE_PHY_H
#define SATELLITE_PHY_H

#include <string>
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "satellite-phy-rx-carrier-conf.h"
#include "satellite-antenna-gain-pattern.h"
#include "satellite-signal-parameters.h"
#include "satellite-node-info.h"

namespace ns3 {

class SatPhyTx;
class SatPhyRx;
class SatChannel;
class SatMac;

/**
 * \ingroup satellite
 *
 * The SatPhy models the physical layer of the satellite system (UT, GW, satellite).
 * SatPhy is the base class implementing the commonalities between different satellite
 * nodes. Actual implementations are located in:
 * - SatUtPhy
 * - SatGwPhy
 * - SatGeoFeederPhy
 * - SatGeoUserPhy
 */
class SatPhy : public Object
{
public:
  typedef SatPhyRxCarrierConf::CarrierBandwidthConverter CarrierBandwidthConverter;
  typedef SatPhyRxCarrierConf::InterferenceModel InterferenceModel;
  typedef SatPhyRxCarrierConf::ErrorModel ErrorModel;
  typedef SatSignalParameters::PacketsInBurst_t PacketContainer_t;

  /**
   * \param  the container of pointers to packets received
   * \param  the id of the beam where packet is from
   */
  typedef Callback<void, PacketContainer_t, Ptr<SatSignalParameters> > ReceiveCallback;

  /**
   * \param The id of the beam.
   * \param The id (address) of the source or sender
   * \param The id (address) of the destination or receiver
   * \param C/N0 value
   */
  typedef Callback<void, uint32_t, Address, Address, double> CnoCallback;

  /**
   * \param
   * \param
   * \param
   */
  typedef Callback<void,uint32_t,uint32_t,double> AverageNormalizedOfferedLoadCallback;

  /**
   * \brief Creation parameters for base PHY object
   */
  typedef struct
  {
    Ptr<NetDevice> m_device;
    Ptr<SatChannel> m_txCh;
    Ptr<SatChannel> m_rxCh;
    uint32_t m_beamId;
  } CreateParam_t;

  /**
   * Default constructor
   */
  SatPhy (void);

  /**
   * Constructor to create PHY objects with parameters.
   */
  SatPhy ( CreateParam_t& params );

  virtual ~SatPhy ();

  // inherited from Object
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

  /**
   * Calculate final SINR with PHY specific parameters and given calculated SINR.
   * Objects inheriting this PHY object must implement this method.
   *
   * \param sinr Calculated C/NI
   */
  virtual double CalculateSinr (double sinr) = 0;

  /**
   * Initialize phy.
   */
  void Initialize();

  /**
   * Get the noise temperature of the receiver in dbK.
   * \return the receiver noise temperature in dbK.
   */
  inline double GetRxNoiseTemperatureDbk () const { return m_rxNoiseTemperatureDbk; }

  /**
   * Set the noise temperature of the receiver in dbK.
   * \param temperatureDbk the receiver noise temperature in dbK.
   */
  inline void SetRxNoiseTemperatureDbk (double temperatureDbk) { m_rxNoiseTemperatureDbk = temperatureDbk; }

  /**
   * Get the maximum antenna gain of the receiver in dB.
   * \return the receiver noise temperature in dB.
   */
  inline double GetRxAntennaGainDb () const { return m_rxMaxAntennaGainDb; }

  /**
   * Set the maximum antenna gain of the receiver in dB.
   * \param gainDb the receiver antenna gain in dB.
   */
  inline void SetRxAntennaGainDb (double gainDb) { m_rxMaxAntennaGainDb = gainDb; }

  /**
   * Get the antenna loss of the receiver in dB.
   * \return the receiver antenna loss in dB.
   */
  inline double GetRxAntennaLossDb () const { return m_rxAntennaLossDb; }

  /**
   * Set the antenna loss of the receiver in dB.
   * \param lossDb the receiver antenna loss in dB.
   */
  inline void SetRxAntennaLossDb (double lossDb) { m_rxAntennaLossDb = lossDb; }

  /**
   * Get the maximum antenna gain of the transmitter in dB.
   * \return the transmitter noise temperature in dB.
   */
  inline double GetTxAntennaGainDb () const { return m_txMaxAntennaGainDb; }

  /**
   * Set the maximum antenna gain of the transmitter in dB.
   * \param gainDb the transmitter antenna gain in dB.
   */
  inline void SetTxAntennaGainDb (double gainDb) { m_txMaxAntennaGainDb = gainDb; }

  /**
   * Get the maximum transmit power of the transmitter in dB.
   * \return the transmitter transmit power in dB.
   */
  inline double GetTxMaxPowerDbw () const { return m_txMaxPowerDbw; }

  /**
   * Set the maximum transmit power of the transmitter in dB.
   * \param powerDb the transmitter transmit power in dB.
   */
  inline void SetTxMaxPowerDbw (double powerDb) { m_txMaxPowerDbw = powerDb; }

  /**
   * Get the output loss of the transmitter in dB.
   * \return the transmitter output loss in dB.
   */
  inline double GetTxOutputLossDb () const { return m_txOutputLossDb; }

  /**
   * Set the output loss of the transmitter in dB.
   * \param lossDb the transmitter output loss in dB.
   */
  inline void SetTxOutputLossDb (double lossDb) { m_txOutputLossDb = lossDb; }

  /**
   * Get the pointing loss of the transmitter in dB.
   * \return the transmitter pointing loss in dB.
   */
  inline double GetTxPointingLossDb () const { return m_txPointingLossDb; }

  /**
   * Set the pointing loss of the transmitter in dB.
   * \param lossDb the transmitter pointing loss in dB.
   */
  inline void SetTxPointingLossDb (double lossDb) { m_txPointingLossDb = lossDb; }

  /**
   * Get the OBO loss of the transmitter in dB.
   * \return the transmitter OBO loss in dB.
   */
  inline double GetTxOboLossDb () const { return m_txOboLossDb; }

  /**
   * Set the OBO loss of the transmitter in dB.
   * \param lossDb the transmitter OBO loss in dB.
   */
  inline void SetTxOboLossDb (double lossDb) { m_txOboLossDb = lossDb; }

  /**
   * Get the antenna loss of the transmitter in dB.
   * \return the transmitter antenna loss in dB.
   */
  inline double GetTxAntennaLossDb () const { return m_txAntennaLossDb; }

  /**
   * Set the antenna loss of the transmitter in dB.
   * \param lossDb the transmitter antenna loss in dB.
   */
  inline void SetTxAntennaLossDb (double lossDb) { m_txAntennaLossDb = lossDb; }

  /**
   * Get the default fading of the PHY.
   * \return the default fading of the PHY.
   */
  inline double GetDefaultFading () const { return m_defaultFadingValue; }

  /**
   * Set the default fading of the PHY.
   * \param fading the default fading of the PHY.
   */
  inline void SetDefaultFading (double fading) { m_defaultFadingValue = fading; }

  /**
   * Set the transmit antenna gain pattern.
   * \param agp antenna gain pattern
   */
  virtual void SetTxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp);

  /**
   * Set the receive antenna gain pattern.
   * \param agp antenna gain pattern
   */
  virtual void SetRxAntennaGainPattern (Ptr<SatAntennaGainPattern> agp);

  /**
   * \param carrierConf Carrier configuration class
   */
  void ConfigureRxCarriers (Ptr<SatPhyRxCarrierConf> carrierConf);

  /**
   * \brief Set fading container
   * \param fadingContainer fading container
   */
  void SetRxFadingContainer (Ptr<SatBaseFading> fadingContainer);

  /**
   * \brief Set fading container
   * \param fadingContainer fading container
   */
  void SetTxFadingContainer (Ptr<SatBaseFading> fadingContainer);

  /**
   * Get the SatPhyTx pointer
   * \return a pointer to the SatPhyTx instance
   */
  virtual Ptr<SatPhyTx> GetPhyTx () const;

  /**
   * Get the SatPhyRx pointer
   * \return a pointer to the SatPhyRx instance
   */
  virtual Ptr<SatPhyRx> GetPhyRx () const;

  /**
   * Set the SatPhyTx module
   * \param phyTx Transmitter PHY module
   */
  virtual void SetPhyTx (Ptr<SatPhyTx> phyTx);

  /**
   * Set the SatPhyRx module
   * \param phyRx Receiver PHY module
   */
  virtual void SetPhyRx (Ptr<SatPhyRx> phyRx);

  /**
   * Get the Tx satellite channel
   * \return the Tx channel
   */
  Ptr<SatChannel> GetTxChannel ();

  /**
   * Send Pdu to the PHY tx module (for initial transmissions from either UT or GW)
   * \param p packet to be sent
   * \param carrierId Carrier id for the packet transmission
   * \param duration the packet transmission duration (from MAC layer)
   * \param txInfo Tx information (e.g. packet type, modcod, waveform ID)
   */
  virtual void SendPdu (PacketContainer_t, uint32_t carrierId, Time duration, SatSignalParameters::txInfo_s txInfo);

  /**
   * Send Pdu to the PHY tx module (for GEO satellite switch packet forwarding)
   * \param rxParams Transmission parameters
   */
  virtual void SendPduWithParams (Ptr<SatSignalParameters> rxParams);

  /**
   * Set the beamId this PHY is connected with
   *
   * \param beamId Satellite beam id
   */
  void SetBeamId (uint32_t beamId);

  /**
   * Receives packets from lower layer (phyRx)
   *
   * \param rxParams Packet reception parameters
   */
  virtual void Receive (Ptr<SatSignalParameters> rxParams, bool phyError);

  /**
   *
   * \param beamId Beam id of C/N0 is received
   * \param source Id (address) of the source (sender)
   * \param cno Value of the C/N0
   */
  void CnoInfo (uint32_t beamId, Address source, Address destination, double cno);

  /**
   *
   * \param beamId Beam id of average normalized load is received
   * \param carrierId Carrier id of average normalized load is received
   * \param averageNormalizedOfferedLoad Value of average normalized offered load
   */
  void AverageNormalizedOfferedRandomAccessLoadInfo (uint32_t beamId, uint32_t carrierId, double averageNormalizedOfferedLoad);

  /**
   * \brief Set the node info class
   * \param nodeInfo Node information related to this SatPhy
   */
  void SetNodeInfo (const Ptr<SatNodeInfo> nodeInfo);

protected:

  /**
   * The upper layer package receive callback.
   */
  SatPhy::ReceiveCallback m_rxCallback;

  /**
   * Trace callback used for packet tracing:
   */
  TracedCallback< Time,
                  SatEnums::SatPacketEvent_t,
                  SatEnums::SatNodeType_t,
                  uint32_t,
                  Mac48Address,
                  SatEnums::SatLogLevel_t,
                  SatEnums::SatLinkDir_t,
                  std::string
                  > m_packetTrace;

  /**
   * Traced callback for all received packets, including the address of the
   * senders.
   */
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

  /**
   * Traced callback for all received packets, including delay information and
   * the address of the senders.
   */
  TracedCallback<Time, const Address &> m_rxDelayTrace;

  /**
   * Node info containing node related information, such as
   * node type, node id and MAC address (of the SatNetDevice)
   */
  Ptr<SatNodeInfo> m_nodeInfo;

  Ptr<SatPhyTx> m_phyTx;
  Ptr<SatPhyRx> m_phyRx;

  /**
   * Calculated EIRP without gain in W.
   */
  double m_eirpWoGainW;

private:

  /**
   * Beam ID
   */
  uint32_t m_beamId;

  /**
   * The C/N0 info callback
   */
  SatPhy::CnoCallback m_cnoCallback;

  /**
   *
   */
  SatPhy::AverageNormalizedOfferedLoadCallback m_avgNormalizedOfferedLoadCallback;

  /**
   * `EnableStatisticsTags` attribute.
   */
  bool m_isStatisticsTagsEnabled;

  /**
   * Configured receiver noise temperature in dBK.
   */
  double m_rxNoiseTemperatureDbk;

  /**
   * Configured maximum receiver antenna gain in dBi.
   */
  double m_rxMaxAntennaGainDb;

  /**
   * Configured receiver antenna loss in Dbs
   */
  double m_rxAntennaLossDb;

  /**
   * Configured maximum transmitter antenna gain in dBi
   */
  double m_txMaxAntennaGainDb;

  /**
   * Configured maximum transmitter power in DbWs
   */
  double m_txMaxPowerDbw;

  /**
   * Configured transmitter output loss in Dbs
   */
  double m_txOutputLossDb;

  /**
   * Configured transmitter pointing loss in Dbs
   */
  double m_txPointingLossDb;

  /**
   * Configured transmitter OBO loss in Dbs
   */
  double m_txOboLossDb;

  /**
   * Configured transmitter antenna loss in Dbs
   */
  double m_txAntennaLossDb;

  /**
   * \brief Default fading value
   */
  double m_defaultFadingValue;
};


}

#endif /* SATELLITE_PHY_H */
