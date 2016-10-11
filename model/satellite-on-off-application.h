/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */
#ifndef SAT_ONOFF_APPLICATION_H
#define SAT_ONOFF_APPLICATION_H

#include <ns3/onoff-application.h>

namespace ns3 {

class Socket;

/**
* \ingroup satellite
*
* \brief This class implements Satellite specific OnOff application.
*        It subclasses ns-3 'native' OnOffApplication to provide needed support for statics.
*        Otherwise functionality of original OnOffApplication is not changed.
*
*/
class SatOnOffApplication : public OnOffApplication
{
public:

  /**
   */
  static TypeId GetTypeId (void);

  /**
   * Constructor for Satellite specific on-off application
   */
  SatOnOffApplication ();

  /**
   * Destructor Satellite specific on-off application
   */
  virtual ~SatOnOffApplication ();

  /**
   * Enable or disable statistic tags
   *
   * \param enableStatus Enable status for statistics
   */
  void EnableStatisticTags (bool enableStatus);

  /**
   * Get enable status of statistic tags.
   *
   * \return true if statistics are enabled, false if statistics are disabled
   */
  bool IsStatisticTagsEnabled () const;

  /**
   * Trace callback for on-off application Tx.
   *
   * \param packet Packet send by on-off application.
   */
  void SendPacketTrace (Ptr<const Packet> packet);
private:
  bool  m_isStatisticsTagsEnabled;  ///< `EnableStatisticsTags` attribute.
  bool  m_isConnectedWithTraceSource;
};

} // namespace ns3

#endif /* SAT_ONOFF_APPLICATION_H */
