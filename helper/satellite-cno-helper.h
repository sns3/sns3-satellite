/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2020 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef __SATELLITE_CNO_HELPER_H__
#define __SATELLITE_CNO_HELPER_H__

#include <ns3/object.h>
#include <ns3/type-id.h>
#include <ns3/log.h>

#include <ns3/satellite-helper.h>

namespace ns3 {

/**
 * \brief This helper allows to set customs C/N0 values for some GW or UT nodes.
 * For each node, the C/N0 value can be constant over time, or follow a trace file
 * giving the temporal evolution of this parameter.
 */
class SatCnoHelper : public Object
{
public:
  /**
   * \brief Struct for storing the custom C/N0 for some nodes.
   */
  typedef struct
  {
    Ptr<Node> node;
    bool isGw;
    bool constant;
    SatEnums::ChannelType_t channelType;
    std::string pathToFile;
    double cno;
  } cnoCustomParams_s;

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of object instance
   * \return the TypeId of object instance
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Default constructor
   */
  SatCnoHelper ();

  /**
   * \brief Create a base SatCnoHelper
   * \param satHelper The satellite Helper
   */
  SatCnoHelper (Ptr<SatHelper> satHelper);

  /**
   * Destructor for SatCnoHelper.
   */
  virtual ~SatCnoHelper ()
  {
  }

  /**
   * Set m_useTraces attribute.
   */
  void UseTracesForDefault (bool useTraces);

  /**
   * Set a constant C/N0 for one GW node and one channel direction
   * \param node The node to apply the new C/N0
   * \param channel The channel type
   * \param cno The constant C/N0 value to set
   */
  void SetGwNodeCno (Ptr<Node> node, SatEnums::ChannelType_t channel, double cno);

  /**
   * Set a constant C/N0 for one UT node and one channel direction
   * \param node The node to apply the new C/N0
   * \param channel The channel type
   * \param cno The constant C/N0 value to set
   */
  void SetUtNodeCno (Ptr<Node> node, SatEnums::ChannelType_t channel, double cno);

  /**
   * Set a constant C/N0 for one GW node and one channel direction
   * \param nodeId The ID of the node to apply the new C/N0
   * \param channel The channel type
   * \param cno The constant C/N0 value to set
   */
  void SetGwNodeCno (uint32_t nodeId, SatEnums::ChannelType_t channel, double cno);

  /**
   * Set a constant C/N0 for one UT node and one channel direction
   * \param nodeId The ID of the node to apply the new C/N0
   * \param channel The channel type
   * \param cno The constant C/N0 value to set
   */
  void SetUtNodeCno (uint32_t nodeId, SatEnums::ChannelType_t channel, double cno);

  /**
   * Set a constant C/N0 for one GW node and one channel direction
   * \param nodes The nodes to apply the new C/N0
   * \param channel The channel type
   * \param cno The constant C/N0 value to set
   */
  void SetGwNodeCno (NodeContainer nodes, SatEnums::ChannelType_t channel, double cno);

  /**
   * Set a constant C/N0 for one UT node and one channel direction
   * \param nodes The nodes to apply the new C/N0
   * \param channel The channel type
   * \param cno The constant C/N0 value to set
   */
  void SetUtNodeCno (NodeContainer nodes, SatEnums::ChannelType_t channel, double cno);

  /**
   * Set a constant C/N0 for one GW node and one channel direction
   * \param node The node to apply the new C/N0
   * \param channel The channel type
   * \param path The path to the file to read
   */
  void SetGwNodeCnoFile (Ptr<Node> node, SatEnums::ChannelType_t channel, std::string path);

  /**
   * Set a constant C/N0 for one UT node and one channel direction
   * \param node The node to apply the new C/N0
   * \param channel The channel type
   * \param path The path to the file to read
   */
  void SetUtNodeCnoFile (Ptr<Node> node, SatEnums::ChannelType_t channel, std::string path);

  /**
   * Set a constant C/N0 for one GW node and one channel direction
   * \param nodeId The ID of the node to apply the new C/N0
   * \param channel The channel type
   * \param path The path to the file to read
   */
  void SetGwNodeCnoFile (uint32_t nodeId, SatEnums::ChannelType_t channel, std::string path);

  /**
   * Set a constant C/N0 for one UT node and one channel direction
   * \param nodeId The ID of the node to apply the new C/N0
   * \param channel The channel type
   * \param path The path to the file to read
   */
  void SetUtNodeCnoFile (uint32_t nodeId, SatEnums::ChannelType_t channel, std::string path);

private:

  /**
   * \brief Pointer to the SatHelper objet
   */

  Ptr<SatHelper> m_satHelper;

  /**
   * \brief Use C/N0 input traces instead of power calculation from antenna gain
   */
  bool m_useTraces;

  /**
   * \brief Array storing manual C/N0 updates (constant or custom file)
   */
  std::vector<cnoCustomParams_s> m_customCno;

  /**
   * Apply configuration to all the satellite channels
   * Needs to be done after node creation
   */
  void ApplyConfiguration ();

  /**
   * Verify if a node has already been set
   * \param node The node to verify
   * \return true if the node is aleady stored in m_customCno
   */
  bool CheckDuplicate (Ptr<Node> node, SatEnums::ChannelType_t channel);
  
};

} // namespace ns3

#endif /* __SATELLITE_CNO_HELPER_H__ */