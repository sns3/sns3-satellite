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

#ifndef SATELLITE_STATS_HELPER_H
#define SATELLITE_STATS_HELPER_H

#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/attribute.h>
#include <ns3/net-device-container.h>
#include <map>


namespace ns3 {


class SatHelper;
class Node;
class CollectorMap;
class DataCollectionObject;

/**
 * \ingroup satellite
 * \defgroup satstats Satellite Statistics
 *
 * Data Collection Framework (DCF) implementation on Satellite module. For
 * usage in simulation script, see SatStatsHelperContainer.
 *
 * \warning SatStatsHelperContainer takes care of setting the attributes
 *          `Name`, `IdentifierType`, and `OutputType`. Thus it's *not*
 *          recommended to manually set the values of these attributes while
 *          using SatStatsHelperContainer.
 */

/**
 * \ingroup satstats
 * \brief Abstract class.
 */
class SatStatsHelper : public Object
{
public:

  // COMMON ENUM DATA TYPES ///////////////////////////////////////////////////

  /**
   * \enum IdentifierType_t
   * \brief
   */
  typedef enum
  {
    IDENTIFIER_GLOBAL = 0,
    IDENTIFIER_GW,
    IDENTIFIER_BEAM,
    IDENTIFIER_UT,
    IDENTIFIER_UT_USER
  } IdentifierType_t;

  /**
   * \param identifierType
   * \return
   */
  static std::string GetIdentifierTypeName (IdentifierType_t identifierType);

  /**
   * \enum OutputType_t
   * \brief
   */
  typedef enum
  {
    OUTPUT_NONE = 0,
    OUTPUT_SCALAR_FILE,
    OUTPUT_SCATTER_FILE,
    OUTPUT_HISTOGRAM_FILE,
    OUTPUT_PDF_FILE,        // probability distribution function
    OUTPUT_CDF_FILE,        // cumulative distribution function
    OUTPUT_SCALAR_PLOT,
    OUTPUT_SCATTER_PLOT,
    OUTPUT_HISTOGRAM_PLOT,
    OUTPUT_PDF_PLOT,        // probability distribution function
    OUTPUT_CDF_PLOT,        // cumulative distribution function
  } OutputType_t;

  /**
   * \param outputType
   * \return
   */
  static std::string GetOutputTypeName (OutputType_t outputType);

  // CONSTRUCTOR AND DESTRUCTOR ///////////////////////////////////////////////

  /**
   * \brief
   * \param satHelper
   */
  SatStatsHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsHelper ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();


  // PUBLIC METHODS ///////////////////////////////////////////////////////////

  /**
   * \brief Install probes, collectors, and aggregators.
   *
   * Behaviour should be implemented by child class in DoInstall().
   */
  void Install ();

  // SETTER AND GETTER METHODS ////////////////////////////////////////////////

  /**
   * \param name
   */
  void SetName (std::string name);

  /**
   * \return
   */
  std::string GetName () const;

  /**
   * \param identifierType
   * \warning Does not have any effect if invoked after Install().
   */
  void SetIdentifierType (IdentifierType_t identifierType);

  /**
   * \return
   */
  IdentifierType_t GetIdentifierType () const;

  /**
   * \param outputType
   * \warning Does not have any effect if invoked after Install().
   */
  void SetOutputType (OutputType_t outputType);

  /**
   * \return
   */
  OutputType_t GetOutputType () const;

  /**
   * \return
   */
  bool IsInstalled () const;

  /**
   * \return
   */
  Ptr<const SatHelper> GetSatHelper () const;

protected:
  /**
   * \brief
   */
  virtual void DoInstall () = 0;

  /**
   * \brief Create the aggregator according to the output type.
   * \param aggregatorTypeId the type of aggregator to be created.
   * \param n1 the name of the attribute to be set on the aggregator created.
   * \param v1 the value of the attribute to be set on the aggregator created.
   * \param n2 the name of the attribute to be set on the aggregator created.
   * \param v2 the value of the attribute to be set on the aggregator created.
   * \param n3 the name of the attribute to be set on the aggregator created.
   * \param v3 the value of the attribute to be set on the aggregator created.
   * \param n4 the name of the attribute to be set on the aggregator created.
   * \param v4 the value of the attribute to be set on the aggregator created.
   * \param n5 the name of the attribute to be set on the aggregator created.
   * \param v5 the value of the attribute to be set on the aggregator created.
   * \return the created aggregator.
   *
   * The created aggregator is stored in #m_aggregator. It can be retrieved
   * from outside using GetAggregator().
   */
  Ptr<DataCollectionObject> CreateAggregator (std::string aggregatorTypeId,
                                              std::string n1 = "",
                                              const AttributeValue &v1 = EmptyAttributeValue (),
                                              std::string n2 = "",
                                              const AttributeValue &v2 = EmptyAttributeValue (),
                                              std::string n3 = "",
                                              const AttributeValue &v3 = EmptyAttributeValue (),
                                              std::string n4 = "",
                                              const AttributeValue &v4 = EmptyAttributeValue (),
                                              std::string n5 = "",
                                              const AttributeValue &v5 = EmptyAttributeValue ());

  /**
   * \brief
   * \param collectorMap
   * \return number of collectors created.
   */
  uint32_t CreateCollectorPerIdentifier (CollectorMap &collectorMap) const;

  // IDENTIFIER RELATED METHODS ///////////////////////////////////////////////

  /**
   * \brief
   * \param utUserNode
   * \return the ID previously assigned to the UT user, or zero if the UT user
   *         is not assigned to any ID.
   */
  uint32_t GetUtUserId (Ptr<Node> utUserNode) const;

  /**
   * \brief
   * \param utNode
   * \return the ID previously assigned to the UT, or zero if the UT is not
   *         assigned to any ID.
   */
  uint32_t GetUtId (Ptr<Node> utNode) const;

  /**
   * \brief
   * \param gwNode
   * \return the ID previously assigned to the GW, or zero if the GW is not
   *         assigned to any ID.
   */
  uint32_t GetGwId (Ptr<Node> gwNode) const;

  /**
   * \param utUserNode
   * \return
   */
  uint32_t GetIdentifierForUtUser (Ptr<Node> utUserNode) const;

  /**
   * \param utNode
   * \return
   */
  uint32_t GetIdentifierForUt (Ptr<Node> utNode) const;

  /**
   * \param beamId
   * \return
   */
  uint32_t GetIdentifierForBeam (uint32_t beamId) const;

  /**
   * \param gwNode
   * \return
   */
  uint32_t GetIdentifierForGw (Ptr<Node> gwNode) const;

  // DEVICE RETRIEVAL METHODS /////////////////////////////////////////////////

  /**
   * \return
   */
  static NetDeviceContainer GetGwSatNetDevice (Ptr<Node> gwNode);

  /**
   * \return
   */
  static Ptr<NetDevice> GetUtSatNetDevice (Ptr<Node> utNode);

private:
  std::string           m_name;            ///<
  IdentifierType_t      m_identifierType;  ///<
  OutputType_t          m_outputType;      ///<
  bool                  m_isInstalled;     ///<
  Ptr<const SatHelper>  m_satHelper;       ///<

}; // end of class SatStatsHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_HELPER_H */
