/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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

#ifndef SATELLITE_LINK_RESULTS_H
#define SATELLITE_LINK_RESULTS_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/satellite-look-up-table.h>
#include <vector>

namespace ns3 {


/**
 * Modulation scheme and coding rate for Satellite module
 */
enum SatModcod_e
{
  SAT_MODCOD_QPSK_1_TO_3 = 0,
  SAT_MODCOD_QPSK_1_TO_2,
  SAT_MODCOD_QPSK_2_TO_3,
  SAT_MODCOD_QPSK_3_TO_4,
  SAT_MODCOD_QPSK_3_TO_5,
  SAT_MODCOD_QPSK_5_TO_6,
  SAT_MODCOD_QPSK_8_TO_9,
  SAT_MODCOD_QPSK_9_TO_10,
  SAT_MODCOD_8PSK_1_TO_2,
  SAT_MODCOD_8PSK_2_TO_3,
  SAT_MODCOD_8PSK_3_TO_4,
  SAT_MODCOD_8PSK_3_TO_5,
  SAT_MODCOD_8PSK_5_TO_6,
  SAT_MODCOD_8PSK_8_TO_9,
  SAT_MODCOD_8PSK_9_TO_10,
  SAT_MODCOD_16APSK_2_TO_3,
  SAT_MODCOD_16APSK_3_TO_4,
  SAT_MODCOD_16APSK_4_TO_5,
  SAT_MODCOD_16APSK_5_TO_6,
  SAT_MODCOD_16APSK_8_TO_9,
  SAT_MODCOD_16APSK_9_TO_10,
  SAT_MODCOD_16QAM_3_TO_4,
  SAT_MODCOD_16QAM_5_TO_6,
  SAT_MODCOD_32APSK_3_TO_4,
  SAT_MODCOD_32APSK_4_TO_5,
  SAT_MODCOD_32APSK_5_TO_6,
  SAT_MODCOD_32APSK_8_TO_10
};


/**
 * \ingroup satellite
 *
 * \brief Abstract class for storing link results.
 */
class SatLinkResults : public Object
{
public:
  SatLinkResults ();
  double GetBler (SatModcod_e modcod, uint16_t burstLength);
protected:
  void DoStart () = 0;
  uint16_t DoGetIndex (SatModcod_e modcod, uint16_t burstLength) = 0;
  std::vector<Ptr<SatLookUpTable> > m_table;
};


/**
 * \ingroup satellite
 *
 * \brief Link results for DVB-RCS2
 *
 * Loads and maintains multiple SatLookUpTable. Provides query service based on
 * burst length and modulation and coding scheme.
 */
class SatLinkResultsDvbRcs2 : public SatLinkResults
{
protected:
  void DoStart ();
  uint16_t DoGetIndex (SatModcod_e modcod, uint16_t burstLength);
};


/**
 * \ingroup satellite
 *
 * \brief Link results for DVB-S2
 *
 * Loads and maintains multiple SatLookUpTable. Provides query service based on
 * modulation and coding scheme.
 */
class SatLinkResultsDvbS2 : public SatLinkResults
{
protected:
  void DoStart ();
  uint16_t DoGetIndex (SatModcod_e modcod, uint16_t burstLength);
};


} // end of namespace ns3


#endif /* SATELLITE_LINK_RESULTS_H */
