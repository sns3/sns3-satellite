/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef SATELLITE_STATS_PACKET_ERROR_HELPER_H
#define SATELLITE_STATS_PACKET_ERROR_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/satellite-enums.h>
#include <ns3/satellite-phy-rx-carrier.h>
#include <ns3/ptr.h>
#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <map>


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class DataCollectionObject;
class Probe;

/**
 * \ingroup satstats
 * \brief Base class for packet error statistics helpers.
 */
class SatStatsPacketErrorHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsPacketErrorHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsPacketErrorHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param nPackets number of packets in the received packet burst.
   * \param from the source address of the packet.
   * \param isError whether a PHY error has occurred.
   *
   * Used only in return link.
   */
  void ErrorRxCallback (uint32_t nPackets, const Address & from, bool isError);

  /**
   * \param traceSourceName name of trace source of PHY RX carrier to listen to.
   */
  void SetTraceSourceName (std::string traceSourceName);

  /**
   * \return name of trace source of PHY RX carrier to listen to.
   */
  std::string GetTraceSourceName () const;

  /**
   * \param linkDirection link direction where statistics are gathered from.
   */
  void SetLinkDirection (SatEnums::SatLinkDir_t linkDirection);

  /**
   * \return link direction where statistics are gathered from.
   */
  SatEnums::SatLinkDir_t GetLinkDirection () const;

  /**
   * \brief Get the valid carrier type
   * \return the valid carrier type
   */
  inline SatPhyRxCarrier::CarrierType GetValidCarrierType () const { return m_carrierType; };

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

  /**
   * \brief Set valid carrier type for this statistics helper type.
   * \param carrierType
   */
  inline void SetValidCarrierType (SatPhyRxCarrier::CarrierType carrierType) { m_carrierType = carrierType; };

private:
  /**
   * \brief Save the address and the proper identifier from the given UT node.
   * \param utNode a UT node.
   *
   * The address of the given node will be saved in the #m_identifierMap
   * member variable. Used only in return link.
   */
  void SaveAddressAndIdentifier (Ptr<Node> utNode);

  /**
   * \brief Set up several listeners on a GW node and connect them to the
   *        collectors.
   * \param gwNode
   */
  void InstallProbeOnGw (Ptr<Node> gwNode);

  /**
   * \brief Set up several probes on a UT node and connect them to the
   *        collectors.
   * \param utNode
   */
  void InstallProbeOnUt (Ptr<Node> utNode);

  /// Maintains a list of probes created by this helper (for forward link).
  std::list<Ptr<Probe> > m_probes;

  /// Maintains a list of collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

  /// Map of address and the identifier associated with it (for return link).
  std::map<const Address, uint32_t> m_identifierMap;

  /// Name of trace source of PHY RX carrier to listen to.
  std::string m_traceSourceName;

  /// Link direction where statistics are gathered from.
  SatEnums::SatLinkDir_t m_linkDirection;

  /// Valid carrier type
  SatPhyRxCarrier::CarrierType m_carrierType;

}; // end of class SatStatsPacketErrorHelper


// FORWARD LINK DEDICATED ACCESS //////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Dedicated Access in forward link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdDaPacketErrorHelper> s = Create<SatStatsFwdDaPacketErrorHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdDaPacketErrorHelper : public SatStatsPacketErrorHelper
{
public:
  // inherited from SatStatsPacketErrorHelper base class
  SatStatsFwdDaPacketErrorHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsFwdDaPacketErrorHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

};


// RETURN LINK DEDICATED ACCESS ///////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Dedicated Access in return link
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnDaPacketErrorHelper> s = Create<SatStatsRtnDaPacketErrorHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnDaPacketErrorHelper : public SatStatsPacketErrorHelper
{
public:
  // inherited from SatStatsPacketErrorHelper base class
  SatStatsRtnDaPacketErrorHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsRtnDaPacketErrorHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

};


// SLOTTED ALOHA //////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access Slotted ALOHA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsSlottedAlohaPacketErrorHelper> s = Create<SatStatsSlottedAlohaPacketErrorHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsSlottedAlohaPacketErrorHelper : public SatStatsPacketErrorHelper
{
public:
  // inherited from SatStatsPacketErrorHelper base class
  SatStatsSlottedAlohaPacketErrorHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsSlottedAlohaPacketErrorHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

};


// CRDSA //////////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce packet error statistics of Random Access CRDSA
 *        from a satellite module simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsCrdsaPacketErrorHelper> s = Create<SatStatsCrdsaPacketErrorHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsCrdsaPacketErrorHelper : public SatStatsPacketErrorHelper
{
public:
  // inherited from SatStatsPacketErrorHelper base class
  SatStatsCrdsaPacketErrorHelper (Ptr<const SatHelper> satHelper);


  /**
   * / Destructor.
   */
  virtual ~SatStatsCrdsaPacketErrorHelper ();


  /**
   * inherited from ObjectBase base class
   */
  static TypeId GetTypeId ();

};


} // end of namespace ns3


#endif /* SATELLITE_STATS_PACKET_ERROR_HELPER_H */
