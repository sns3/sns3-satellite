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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 *         Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SATELLITE_RX_CNO_INPUT_TRACE_CONTAINER_H
#define SATELLITE_RX_CNO_INPUT_TRACE_CONTAINER_H

#include "satellite-base-trace-container.h"
#include "satellite-enums.h"

#include <ns3/mac48-address.h>
#include <ns3/satellite-input-fstream-time-double-container.h>

#include <map>

namespace ns3
{

/**
 * \ingroup satellite
 *
 * \brief Class for Rx C/N0 input trace container. The class contains
 * multiple Rx C/N0 input sample traces and provides an interface to them.
 * It can also be set manually to a constant for chosen nodes.
 */
class SatRxCnoInputTraceContainer : public SatBaseTraceContainer
{
  public:
    /**
     * \brief typedef for map key
     */
    typedef std::pair<Address, SatEnums::ChannelType_t> key_t;

    /**
     * \brief typedef for map of containers
     */
    typedef std::map<key_t, Ptr<SatInputFileStreamTimeDoubleContainer>> container_t;

    /**
     * \brief typedef for map of containers
     */
    typedef std::map<key_t, double> containerConstantCno_t;

    /**
     * \brief Constructor
     */
    SatRxCnoInputTraceContainer();

    /**
     * \brief Destructor
     */
    ~SatRxCnoInputTraceContainer();

    /**
     * \brief NS-3 type id function
     * \return type id
     */
    static TypeId GetTypeId(void);

    /**
     * \brief NS-3 instance type id function
     * \return Instance type is
     */
    TypeId GetInstanceTypeId(void) const;

    /**
     *  \brief Do needed dispose actions.
     */
    void DoDispose();

    /**
     * \brief Function for getting the Rx C/N0
     * \param key key
     * \return Rx C/N0
     */
    double GetRxCno(key_t key);

    /**
     * \brief Function for setting the Rx C/N0 with constant value
     * \param key key
     * \param cno C/N0
     */
    void SetRxCno(key_t key, double cno);

    /**
     * \brief Function for setting the Rx C/N0 with input file
     * \param key key
     * \param path path to C/N0 file
     */
    void SetRxCnoFile(key_t key, std::string path);

    /**
     * \brief Function for resetting the variables
     */
    void Reset();

    /**
     * \brief Function for adding the node to the map
     * \param key key
     * \return pointer to the added container
     */
    Ptr<SatInputFileStreamTimeDoubleContainer> AddNode(
        std::pair<Address, SatEnums::ChannelType_t> key);

  private:
    /**
     * \brief Function for finding the container matching the key
     * \param key key
     * \return matching container
     */
    Ptr<SatInputFileStreamTimeDoubleContainer> FindNode(key_t key);

    /**
     * \brief Map for containers
     */
    container_t m_container;

    /**
     * \brief Container to store the constant values of C/N0. Overrides the use of an input file for
     * them.
     */
    containerConstantCno_t m_containerConstantCno;
};

} // namespace ns3

#endif /* SATELLITE_RX_CNO_INPUT_TRACE_CONTAINER_H */
