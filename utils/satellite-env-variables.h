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
#ifndef SATELLITE_ENV_VARIABLES_H
#define SATELLITE_ENV_VARIABLES_H

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for environmental variables
 */
class SatEnvVariables : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatEnvVariables ();

  /**
   * \brief Destructor
   */
  virtual ~SatEnvVariables ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for getting the simulator root path
   * \return
   */
  std::string GetSimulatorRootPath ();

private:

  /**
   * \brief Path to simulator root folder
   */
  std::string m_simulatorRootPath;

};

} // namespace ns3

#endif /* SATELLITE_ENV_VARIABLES_H */
