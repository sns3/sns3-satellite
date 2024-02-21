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
 */
#ifndef SATELLITE_FADING_OUTPUT_TRACE_CONTAINER_H
#define SATELLITE_FADING_OUTPUT_TRACE_CONTAINER_H

#include "satellite-base-trace-container.h"
#include "satellite-enums.h"

#include <ns3/mac48-address.h>
#include <ns3/satellite-output-fstream-double-container.h>

#include <map>

namespace ns3
{

/**
 * \ingroup satellite
 *
 * \brief Class for fading output trace container. The class contains
 * multiple fading output sample traces and provides an interface to them.
 */
class SatFadingOutputTraceContainer : public SatBaseTraceContainer
{
  public:
    /**
     * \brief typedef for map key
     */
    typedef std::pair<Address, SatEnums::ChannelType_t> key_t;

    /**
     * \brief typedef for map of containers
     */
    typedef std::map<key_t, Ptr<SatOutputFileStreamDoubleContainer>> container_t;

    /**
     * \brief Constructor
     */
    SatFadingOutputTraceContainer();

    /**
     * \brief Destructor
     */
    ~SatFadingOutputTraceContainer();

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
     * \brief Add the vector containing the values to container matching the key
     * \param key key
     * \param newItem vector of values
     */
    void AddToContainer(key_t key, std::vector<double> newItem);

    /**
     * Function for enabling / disabling figure output
     * \param enableFigureOutput
     */
    void EnableFigureOutput(bool enableFigureOutput)
    {
        m_enableFigureOutput = enableFigureOutput;
    }

    /**
     * \brief Function for resetting the variables
     */
    void Reset();

  private:
    /**
     * \brief Function for adding the node to the map
     * \param key key
     * \return pointer to the added container
     */
    Ptr<SatOutputFileStreamDoubleContainer> AddNode(
        std::pair<Address, SatEnums::ChannelType_t> key);

    /**
     * \brief Function for finding the container matching the key
     * \param key key
     * \return matching container
     */
    Ptr<SatOutputFileStreamDoubleContainer> FindNode(key_t key);

    /**
     * \brief Write the contents of a container matching to the key into a file
     */
    void WriteToFile();

    /**
     * \brief Map for containers
     */
    container_t m_container;

    /**
     * \brief Switch for figure output
     */
    bool m_enableFigureOutput;
};

} // namespace ns3

#endif /* SATELLITE_FADING_OUTPUT_TRACE_CONTAINER_H */
