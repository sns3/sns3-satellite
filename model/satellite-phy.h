/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#ifndef SATELLITE_PHY_H
#define SATELLITE_PHY_H

#include "satellite-antenna-gain-pattern.h"
#include "satellite-beam-channel-pair.h"
#include "satellite-frame-conf.h"
#include "satellite-node-info.h"
#include "satellite-phy-rx-carrier-conf.h"
#include "satellite-signal-parameters.h"

#include <ns3/address.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>

#include <string>

namespace ns3
{

class SatPhyTx;
class SatPhyRx;
class SatChannel;
class SatMac;

/**
 * \ingroup satellite
 *
 * The SatPhy models the basic physical layer of the satellite system. SatPhy
 * is the base class implementing the commonalities between different satellite
 * nodes. This is a pure virtual class and actual implementations are located in:
 * - SatUtPhy
 * - SatGwPhy
 * - SatGeoFeederPhy
 * - SatGeoUserPhy
 */
class SatPhy : public Object
{
  public:
    /**
     * Define InterferenceModel in SatPhy
     */
    typedef SatPhyRxCarrierConf::InterferenceModel InterferenceModel;
    /**
     * Define InterferenceEliminationModel in SatPhy
     */
    typedef SatPhyRxCarrierConf::InterferenceEliminationModel InterferenceEliminationModel;
    /**
     * Define ErrorModel in SatPhy
     */
    typedef SatPhyRxCarrierConf::ErrorModel ErrorModel;
    /**
     * Define PacketContainer in SatPhy
     */
    typedef SatSignalParameters::PacketsInBurst_t PacketContainer_t;

    /**
     * \param  the container of pointers to packets received
     * \param  the id of the beam where packet is from
     */
    typedef Callback<void, PacketContainer_t, Ptr<SatSignalParameters>> ReceiveCallback;

    /**
     * \param The id of the satellite.
     * \param The id of the beam.
     * \param The id (address) of the source or sender
     * \param The id (address) of the destination or receiver
     * \param C/N0 value
     */
    typedef Callback<void, uint32_t, uint32_t, Address, Address, double, bool> CnoCallback;

    /**
     * \param satellite Id
     * \param beam Id
     * \param carrier Id
     * \param allocation channel Id
     * \param average normalized offered load
     */
    typedef Callback<void, uint32_t, uint32_t, uint32_t, uint8_t, double>
        AverageNormalizedOfferedLoadCallback;

    /**
     * \brief Creation parameters for base PHY object
     */
    typedef struct
    {
        Ptr<NetDevice> m_device;
        Ptr<SatChannel> m_txCh;
        Ptr<SatChannel> m_rxCh;
        uint32_t m_satId;
        uint32_t m_beamId;
        SatEnums::SatLoraNodeType_t m_standard;
    } CreateParam_t;

    /**
     * Default constructor
     */
    SatPhy(void);

    /**
     * Constructor to create PHY objects with parameters.
     */
    SatPhy(CreateParam_t& params);

    /**
     * Destructor
     */
    virtual ~SatPhy();

    /**
     * Derived from Object
     */
    static TypeId GetTypeId(void);
    /**
     * Derived from Object
     */
    TypeId GetInstanceTypeId(void) const;
    /**
     * Initialization of SatPhy
     */
    virtual void DoInitialize(void);

    /**
     * Dispose of SatPhy
     */
    virtual void DoDispose(void);

    /**
     * \brief Get additional interference, used to compute final SINR at RX
     *
     * \return Additional interference
     */
    virtual double GetAdditionalInterference() = 0;

    /**
     * \brief Calculate final SINR with PHY specific parameters and given calculated SINR.
     * Additional interference value is added.
     *
     * \param sinr Calculated SINR
     * \param otherInterference Interference to add to the sinr
     * \return Final SINR
     */
    double CalculateSinr(double sinr, double otherInterference);

    /**
     * \brief Initialize phy.
     */
    void Initialize();

    /**
     * \brief Get the noise temperature of the receiver in dbK.
     * \return the receiver noise temperature in dbK.
     */
    inline double GetRxNoiseTemperatureDbk() const
    {
        return m_rxNoiseTemperatureDbk;
    }

    /**
     * \brief Set the noise temperature of the receiver in dbK.
     * \param temperatureDbk the receiver noise temperature in dbK.
     */
    inline void SetRxNoiseTemperatureDbk(double temperatureDbk)
    {
        m_rxNoiseTemperatureDbk = temperatureDbk;
    }

    /**
     * \brief Get the maximum antenna gain of the receiver in dB.
     * \return the receiver noise temperature in dB.
     */
    inline double GetRxAntennaGainDb() const
    {
        return m_rxMaxAntennaGainDb;
    }

    /**
     * \brief Set the maximum antenna gain of the receiver in dB.
     * \param gainDb the receiver antenna gain in dB.
     */
    inline void SetRxAntennaGainDb(double gainDb)
    {
        m_rxMaxAntennaGainDb = gainDb;
    }

    /**
     * \brief Get the antenna loss of the receiver in dB.
     * \return the receiver antenna loss in dB.
     */
    inline double GetRxAntennaLossDb() const
    {
        return m_rxAntennaLossDb;
    }

    /**
     * \brief Set the antenna loss of the receiver in dB.
     * \param lossDb the receiver antenna loss in dB.
     */
    inline void SetRxAntennaLossDb(double lossDb)
    {
        m_rxAntennaLossDb = lossDb;
    }

    /**
     * \brief Get the maximum antenna gain of the transmitter in dB.
     * \return the transmitter noise temperature in dB.
     */
    inline double GetTxAntennaGainDb() const
    {
        return m_txMaxAntennaGainDb;
    }

    /**
     * \brief Set the maximum antenna gain of the transmitter in dB.
     * \param gainDb the transmitter antenna gain in dB.
     */
    inline void SetTxAntennaGainDb(double gainDb)
    {
        m_txMaxAntennaGainDb = gainDb;
    }

    /**
     * \brief Get the maximum transmit power of the transmitter in dB.
     * \return the transmitter transmit power in dB.
     */
    inline double GetTxMaxPowerDbw() const
    {
        return m_txMaxPowerDbw;
    }

    /**
     * \brief Set the maximum transmit power of the transmitter in dB.
     * \param powerDb the transmitter transmit power in dB.
     */
    inline void SetTxMaxPowerDbw(double powerDb)
    {
        m_txMaxPowerDbw = powerDb;
    }

    /**
     * \brief Get the output loss of the transmitter in dB.
     * \return the transmitter output loss in dB.
     */
    inline double GetTxOutputLossDb() const
    {
        return m_txOutputLossDb;
    }

    /**
     * \brief Set the output loss of the transmitter in dB.
     * \param lossDb the transmitter output loss in dB.
     */
    inline void SetTxOutputLossDb(double lossDb)
    {
        m_txOutputLossDb = lossDb;
    }

    /**
     * \brief Get the pointing loss of the transmitter in dB.
     * \return the transmitter pointing loss in dB.
     */
    inline double GetTxPointingLossDb() const
    {
        return m_txPointingLossDb;
    }

    /**
     * \brief Set the pointing loss of the transmitter in dB.
     * \param lossDb the transmitter pointing loss in dB.
     */
    inline void SetTxPointingLossDb(double lossDb)
    {
        m_txPointingLossDb = lossDb;
    }

    /**
     * \brief Get the OBO loss of the transmitter in dB.
     * \return the transmitter OBO loss in dB.
     */
    inline double GetTxOboLossDb() const
    {
        return m_txOboLossDb;
    }

    /**
     * \brief Set the OBO loss of the transmitter in dB.
     * \param lossDb the transmitter OBO loss in dB.
     */
    inline void SetTxOboLossDb(double lossDb)
    {
        m_txOboLossDb = lossDb;
    }

    /**
     * \brief Get the antenna loss of the transmitter in dB.
     * \return the transmitter antenna loss in dB.
     */
    inline double GetTxAntennaLossDb() const
    {
        return m_txAntennaLossDb;
    }

    /**
     * \brief Set the antenna loss of the transmitter in dB.
     * \param lossDb the transmitter antenna loss in dB.
     */
    inline void SetTxAntennaLossDb(double lossDb)
    {
        m_txAntennaLossDb = lossDb;
    }

    /**
     * \brief Get the default fading of the PHY.
     * \return the default fading of the PHY.
     */
    inline double GetDefaultFading() const
    {
        return m_defaultFadingValue;
    }

    /**
     * \brief Set the default fading of the PHY.
     * \param fading the default fading of the PHY.
     */
    inline void SetDefaultFading(double fading)
    {
        m_defaultFadingValue = fading;
    }

    /**
     * \brief Set the transmit antenna gain pattern.
     * \param agp antenna gain pattern
     * \param mobility mobility model of satellite
     */
    virtual void SetTxAntennaGainPattern(Ptr<SatAntennaGainPattern> agp,
                                         Ptr<SatMobilityModel> satelliteMobility);

    /**
     * \brief Set the receive antenna gain pattern.
     * \param agp antenna gain pattern
     * \param mobility mobility model of satellite
     */
    virtual void SetRxAntennaGainPattern(Ptr<SatAntennaGainPattern> agp,
                                         Ptr<SatMobilityModel> satelliteMobility);

    /**
     * \brief Configure Rx carriers
     * \param carrierConf Carrier configuration class
     * \param superFrameConf Superframe configuration
     */
    void ConfigureRxCarriers(Ptr<SatPhyRxCarrierConf> carrierConf,
                             Ptr<SatSuperframeConf> superFrameConf);

    /**
     * \brief Set fading container
     * \param fadingContainer fading container
     */
    void SetRxFadingContainer(Ptr<SatBaseFading> fadingContainer);

    /**
     * \brief Set fading container
     * \param fadingContainer fading container
     */
    void SetTxFadingContainer(Ptr<SatBaseFading> fadingContainer);

    /**
     * \brief Get the SatPhyTx pointer
     * \return a pointer to the SatPhyTx instance
     */
    virtual Ptr<SatPhyTx> GetPhyTx() const;

    /**
     * \brief Get the SatPhyRx pointer
     * \return a pointer to the SatPhyRx instance
     */
    virtual Ptr<SatPhyRx> GetPhyRx() const;

    /**
     * \brief Set the SatPhyTx module
     * \param phyTx Transmitter PHY module
     */
    virtual void SetPhyTx(Ptr<SatPhyTx> phyTx);

    /**
     * \brief Set the SatPhyRx module
     * \param phyRx Receiver PHY module
     */
    virtual void SetPhyRx(Ptr<SatPhyRx> phyRx);

    /**
     * \brief Get the Tx satellite channel
     * \return the Tx channel
     */
    Ptr<SatChannel> GetTxChannel();

    /**
     * \brief Send Pdu to the PHY tx module (for initial transmissions from either UT or GW)
     * \param p packet to be sent
     * \param carrierId Carrier id for the packet transmission
     * \param duration the packet transmission duration (from MAC layer)
     * \param txInfo Tx information (e.g. packet type, modcod, waveform ID)
     */
    virtual void SendPdu(PacketContainer_t,
                         uint32_t carrierId,
                         Time duration,
                         SatSignalParameters::txInfo_s txInfo);

    /**
     * \brief Send Pdu to the PHY tx module (for GEO satellite switch packet forwarding)
     * \param rxParams Transmission parameters
     */
    virtual void SendPduWithParams(Ptr<SatSignalParameters> rxParams);

    /**
     * \brief Set the satId this PHY is connected with
     * \param satId Satellite ID
     */
    void SetSatId(uint32_t satId);

    /**
     * \brief Set the beamId this PHY is connected with
     * \param beamId Satellite beam id
     */
    bool SetBeamId(uint32_t beamId);

    /**
     * \brief Receives packets from lower layer.
     * \param rxParams Packet reception parameters
     * \param phyError Boolean indicating whether the packet successfully
     * received or not?
     */
    virtual void Receive(Ptr<SatSignalParameters> rxParams, bool phyError);

    /**
     * \brief Function for getting the C/NO information
     * \param beamId Beam id of C/N0 is received
     * \param source Id (address) of the source (sender)
     * \param destination Id (address) of the destination
     * \param cno Value of the C/N0
     * \param isSatelliteMac If true, cno corresponds to link SAT to GW; if false, cno corresponds
     * to link UT to GW
     */
    void CnoInfo(uint32_t satId,
                 uint32_t beamId,
                 Address source,
                 Address destination,
                 double cno,
                 bool isSatelliteMac);

    /**
     * \brief Function for getting the normalized offered load of the specific random access
     * allocation channel \param satId Satellite id of average normalized load is received \param
     * beamId Beam id of average normalized load is received \param carrierId Carrier id of average
     * normalized load is received \param allocationChannelId allocation channel ID \param
     * averageNormalizedOfferedLoad Value of average normalized offered load
     */
    void AverageNormalizedOfferedRandomAccessLoadInfo(uint32_t satId,
                                                      uint32_t beamId,
                                                      uint32_t carrierId,
                                                      uint8_t allocationChannelId,
                                                      double averageNormalizedOfferedLoad);

    /**
     * \brief Set the node info class
     * \param nodeInfo Node information related to this SatPhy
     */
    void SetNodeInfo(const Ptr<SatNodeInfo> nodeInfo);

    /**
     * \brief Begin frame/window end scheduling for processes utilizing frame length as interval
     */
    void BeginEndScheduling();

    /**
     * \brief Callback for retrieving a pair of SatChannel associated to a beam
     * \param uint32_t  beam ID
     * \return A pair of SatChannel to use as communication channel in this beam
     */
    typedef Callback<SatChannelPair::ChannelPair_t, uint32_t, uint32_t> ChannelPairGetterCallback;

    /**
     * \brief Set the channel pair getter callback
     * \param cb callback to invoke whenever a SatChannel pair for a beam is required.
     */
    void SetChannelPairGetterCallback(SatPhy::ChannelPairGetterCallback cb);

  protected:
    /**
     * \brief Invoke the `Rx` trace source for each received packet.
     * \param packets Container of the pointers to the packets received.
     */
    virtual void RxTraces(SatPhy::PacketContainer_t packets);

    /**
     * \brief Invoke the `RxLinkModcod` trace source for each received packet.
     * \param rxParams Pointer to SatSignalParameters of packets received.
     */
    void ModcodTrace(Ptr<SatSignalParameters> rxParams);

    /**
     * \brief Set SatPhyTimeTag of packets
     * \param packets Container of the pointers to the packets to tag.
     */
    void SetTimeTag(SatPhy::PacketContainer_t packets);

    /**
     * \brief Get the link TX direction. Must be implemented by child clases.
     * \return The link TX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkTxDir();

    /**
     * \brief Get the link RX direction. Must be implemented by child clases.
     * \return The link RX direction
     */
    virtual SatEnums::SatLinkDir_t GetSatLinkRxDir();

    /**
     * The upper layer package receive callback.
     */
    SatPhy::ReceiveCallback m_rxCallback;

    /**
     * Callback for retrieving SatChannel pairs by beam
     */
    SatPhy::ChannelPairGetterCallback m_retrieveChannelPair;

    /**
     * Trace callback used for packet tracing:
     */
    TracedCallback<Time,
                   SatEnums::SatPacketEvent_t,
                   SatEnums::SatNodeType_t,
                   uint32_t,
                   Mac48Address,
                   SatEnums::SatLogLevel_t,
                   SatEnums::SatLinkDir_t,
                   std::string>
        m_packetTrace;

    /**
     * Traced callback for all received packets, including the address of the
     * senders.
     */
    TracedCallback<Ptr<const Packet>, const Address&> m_rxTrace;

    /**
     * Traced callback for all received packets, including delay information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxDelayTrace;

    /**
     * Traced callback for all received packets, including link delay information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxLinkDelayTrace;

    /**
     * Traced callback for all received packets, including jitter information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxJitterTrace;

    /**
     * Traced callback for all received packets, including link jitter information and
     * the address of the senders.
     */
    TracedCallback<const Time&, const Address&> m_rxLinkJitterTrace;

    /**
     * Traced callback for all received packets, including link MODCOD information and
     * the address of the senders.
     */
    TracedCallback<uint32_t, const Address&> m_rxLinkModcodTrace;

    /**
     * Node info containing node related information, such as
     * node type, node id and MAC address (of the SatNetDevice)
     */
    Ptr<SatNodeInfo> m_nodeInfo;

    /**
     * Pointer to internal SatPhyTx instance
     */
    Ptr<SatPhyTx> m_phyTx;

    /**
     * Pointer to internal SatPhyRx instance
     */
    Ptr<SatPhyRx> m_phyRx;

    /**
     * Satellite ID
     */
    uint32_t m_satId;

    /**
     * Beam ID
     */
    uint32_t m_beamId;

    /**
     * Calculated EIRP without gain in W.
     */
    double m_eirpWoGainW;

    /**
     * `EnableStatisticsTags` attribute.
     */
    bool m_isStatisticsTagsEnabled;

    /**
     * Last delay measurement. Used to compute jitter.
     */
    Time m_lastDelay;

    /**
     * Last delay measurement for link. Used to compute link jitter.
     */
    Time m_lastLinkDelay;

  private:
    /**
     * The C/N0 info callback
     */
    SatPhy::CnoCallback m_cnoCallback;

    /**
     * Average normalized offered load callback
     */
    SatPhy::AverageNormalizedOfferedLoadCallback m_avgNormalizedOfferedLoadCallback;

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

} // namespace ns3

#endif /* SATELLITE_PHY_H */
