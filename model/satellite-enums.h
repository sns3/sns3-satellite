/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
#ifndef SATELLITE_ENUMS_H
#define SATELLITE_ENUMS_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 * 
 * \brief SatEnums class is for general enumerators used in satellite module.
 */
class SatEnums
{
public:

  /**
   * Possible types of channel.
   */
  typedef enum
  {
    UNKNOWN_CH = 0,
    FORWARD_FEEDER_CH = 1,
    FORWARD_USER_CH = 2,
    RETURN_USER_CH = 3,
    RETURN_FEEDER_CH = 4
  } ChannelType_t;

  static inline std::string GetChannelTypeName (ChannelType_t channelType)
  {
    switch (channelType)
    {
      case UNKNOWN_CH:
        {
          return "UNKNOWN_CH";
        }
      case FORWARD_FEEDER_CH:
        {
          return "FORWARD_FEEDER_CH";
        }
      case FORWARD_USER_CH:
        {
          return "FORWARD_USER_CH";
        }
      case RETURN_USER_CH:
        {
          return "RETURN_USER_CH";
        }
      case RETURN_FEEDER_CH:
        {
          return "RETURN_FEEDER_CH";
        }
      default :
        {
          NS_FATAL_ERROR ("SatEnums::GetChannelTypeName - Invalid channel type");
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetChannelTypeName - Invalid channel type");
    return "";
  };

private:

  /**
   * Destructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatEnums() = 0;
};

} // namespace ns3

#endif /* SATELLITE_ENUMS_H */
