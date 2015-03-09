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
#ifndef SATELLITE_BASE_FADER_CONF_H
#define SATELLITE_BASE_FADER_CONF_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Base class for fader configuration classes. This class
 * defines the functions which all the inherited fader
 * configuration classes must implement for the Markov-fading
 * model faders. This base class itself is abstract and will not
 * implement any real functionality.
 */
class SatBaseFaderConf : public Object
{
public:
  /**
   * \brief Constructor
   */
  SatBaseFaderConf ();

  /**
   * \brief Destructor
   */
  virtual ~SatBaseFaderConf ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for getting the fading value
   * \param set parameter set
   * \return fading value
   */
  virtual std::vector<std::vector<double> > GetParameters (uint32_t set) = 0;

private:
};

} // namespace ns3

#endif /* SATELLITE_BASE_FADER_CONF_H */
