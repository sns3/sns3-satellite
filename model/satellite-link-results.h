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

#include <vector>

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/satellite-look-up-table.h>

namespace ns3 {


/**
 * \ingroup satellite
 *
 * \brief Abstract class for storing link results.

 * The child class is expected to load and handle multiple SatLookUpTable, and
 * provide query service on it. Two purely virtual functions must be
 * overridden by the child class: DoStart and DoGetTableIndex.
 */
class SatLinkResults : public Object
{
public:

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
    SAT_MODCOD_QPSK_4_TO_5, // not in pptm
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
    SAT_MODCOD_32APSK_8_TO_9 // 9 or 10?
  };

  SatLinkResults ();

  static TypeId GetTypeId ();

  /**
   * \brief Initialize look up tables.
   *
   * Simply calls DoInitialize.
   */
  void Initialize ();

  /**
   * \brief Get a BLER or BLEP value from link results
   * \param modcod type of modulation scheme and coding rate for DVB-S2 and
   *               DVB-RCS2
   * \param burstLength burst length for DVB-RCS2, typically 262, 536, 664, or
   *                    1616
   * \param sinrDb the received SINR in dB
   * \return BLER or BLEP value, which is a ``double`` ranging between [0..1]
   *
   * Must be run after Initialize is called.
   *
   * The function is not expected to support all possible combinations of Modcod
   * and burst length. When an invalid combination is given, the function will
   * throw an error.
   */
  double GetBler (SatLinkResults::SatModcod_e modcod, uint16_t burstLength,
                  double sinrDb) const;

protected:

  /**
   * \brief Initialize look up tables.
   *
   * Child classes must implement this function to initialize the m_table
   * variable. This is typically done by loading pre-defined input files from
   * the file system. In case of failure, the function should throw an error by
   * calling ``NS_FATAL_ERROR``.
   */
  virtual void DoInitialize () = 0;

  /**
   * \brief Convert a combination of Modcod and burst length to an index of
   *        the m_table vector.
   *
   * Child classes must implement this function to indicate how the right look
   * up table can be found.
   *
   * When an invalid Modcod and burst length combination is given, the function
   * should throw an error (i.e. by calling ``NS_FATAL_ERROR``).
   */
  virtual uint8_t DoGetTableIndex (SatLinkResults::SatModcod_e modcod,
                                   uint16_t burstLength) const = 0;

  /**
   * \brief Linear one-dimensional array of satellite link result look up
   *        tables.
   */
  std::vector<Ptr<SatLookUpTable> > m_table;

  std::string m_inputPath;

  bool m_isInitialized;
};


/**
 * \ingroup satellite
 *
 * \brief Link results for DVB-RCS2.
 *
 * Loads and maintains multiple SatLookUpTable. Provides query service based on
 * burst length and modulation and coding scheme.
 */
class SatLinkResultsDvbRcs2 : public SatLinkResults
{
public:
  static TypeId GetTypeId ();
protected:
  void DoInitialize ();
  uint8_t DoGetTableIndex (SatLinkResults::SatModcod_e modcod,
                           uint16_t burstLength) const;
};


/**
 * \ingroup satellite
 *
 * \brief Link results for DVB-S2.
 *
 * Loads and maintains multiple SatLookUpTable. Provides query service based on
 * modulation and coding scheme.
 */
class SatLinkResultsDvbS2 : public SatLinkResults
{
public:
  static TypeId GetTypeId ();
protected:
  void DoInitialize ();
  uint8_t DoGetTableIndex (SatLinkResults::SatModcod_e modcod,
                           uint16_t burstLength) const;
};


} // end of namespace ns3


#endif /* SATELLITE_LINK_RESULTS_H */
