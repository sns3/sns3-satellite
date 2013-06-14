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
 *
 * This class cannot be instantiated. Use the child classes instead.
 *
 * The child class is expected to load and handle multiple SatLookUpTable, and
 * provide query service on it. Two purely virtual functions must be overriden
 * by the child class: SatLinkResults::DoStart and
 * SatLinkResults::DoGetTableIndex.
 *
 * Usage example:
 *
 *     Ptr<SatLinkResultsDvbRcs2> linkDvbRcs2 = CreateObject<SatLinkResultsDvbRcs2> ();
 *     Ptr<SatLinkResultsDvbS2> linkDvbS2 = CreateObject<SatLinkResultsDvbRcs2> ();
 *     linkDvbRcs2->Initialize ();
 *     linkDvbS2->Initialize ();
 *
 * The above initialization steps will load link results data from text files in
 * `src/satellite/test/reference` directory. After that, BLER can be retrieved
 * by the following call:
 *
 *      linkDvbRcs2->GetBler (SatLinkResults::SAT_MODCOD_QPSK_1_TO_3,
 *                            536, 0.9);
 *
 * In this example, the object will consult the link results data for DVB-RCS2
 * using QPSK 1/3 Modcod and 536 burst length, and then compute the BLER for
 * SINR of 0.9 dB.
 *
 * Modcod is a mix of modulation scheme and coding rate, which is defined in
 * SatLinkResults::SatModcod_e.
 *
 * \sa SatLinkResultsDvbRcs2, SatLinkResultsDvbS2
 */
class SatLinkResults : public Object
{
public:
  /**
   * \enum SatModcod_e
   *
   * \brief Modulation scheme and coding rate for Satellite module.
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
   * Must be called before any SatLinkResults::GetBler can be invoked.
   *
   * Simply calls SatLinkResults::DoInitialize.
   */
  void Initialize ();

  /**
   * \brief Get a BLER (or BLEP, FER) value from link results.
   *
   * \param modcod type of modulation scheme and coding rate
   * \param burstLength burst length, typically 262, 536, 664, or 1616
   * \param sinrDb the received SINR in dB
   * \return BLER value, which is a `double` ranging between [0..1]
   *
   * Must be run after SatLinkResults::Initialize is called.
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
   * Child classes must implement this function to initialize
   * SatLinkResults::m_table member variable. This is typically done by loading
   * pre-defined input files from the file system. In case of failure, the
   * function should throw an error by calling `NS_FATAL_ERROR`.
   */
  virtual void DoInitialize () = 0;

  /**
   * \brief Convert a combination of Modcod and burst length to an index of
   *        the SatLinkResults::m_table vector.
   *
   * \param modcod type of modulation scheme and coding rate
   * \param burstLength burst length, typically 262, 536, 664, or 1616
   * \return an unsigned integer, representing an index in
   *         SatLinkResults::m_table
   *
   * Child classes must implement this function to indicate how the correct look
   * up table can be found.
   *
   * When an invalid Modcod and burst length combination is given, the function
   * should throw an error (i.e. by calling `NS_FATAL_ERROR`).
   */
  virtual uint8_t DoGetTableIndex (SatLinkResults::SatModcod_e modcod,
                                   uint16_t burstLength) const = 0;

  /**
   * \brief Array of satellite link result look up tables.
   *
   * \sa SatLookUpTable
   */
  std::vector<Ptr<SatLookUpTable> > m_table;

  /**
   * \brief Linked to the attribute *InputPath*, the base path where the text
   *        files containing link results data can be found.
   */
  std::string m_inputPath;

  /**
   * \brief Indicates if SatLinkResults::Initialize has been called.
   */
  bool m_isInitialized;
};


/**
 * \ingroup satellite
 *
 * \brief Link results for DVB-RCS2.
 *
 * Loads and maintains multiple SatLookUpTable. Provides query service based on
 * burst length and modulation and coding scheme.
 *
 * See usage examples in the parent class documentation (SatLinkResults).
 */
class SatLinkResultsDvbRcs2 : public SatLinkResults
{
public:
  static TypeId GetTypeId ();
protected:
  /**
   * \brief Initialize by loading DVB-RCS2 look up tables.
   */
  void DoInitialize ();

  /**
   * \brief Convert Modcod to an index of the SatLinkResults::m_table vector.
   *
   * \param modcod type of modulation scheme and coding rate
   * \param burstLength burst length, either 262, 536, or 1616
   * \return an unsigned integer, representing an index in
   *         SatLinkResults::m_table
   */
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
 *
 * See usage examples in the parent class documentation (SatLinkResults).
 */
class SatLinkResultsDvbS2 : public SatLinkResults
{
public:
  static TypeId GetTypeId ();
protected:
  /**
   * \brief Initialize by loading DVB-S2 look up tables.
   */
  void DoInitialize ();

  /**
   * \brief Convert Modcod to an index of the SatLinkResults::m_table vector.
   *
   * \param modcod type of modulation scheme and coding rate
   * \param burstLength burst length, ignored because it is not used in DVB-S2
   * \return an unsigned integer, representing an index in
   *         SatLinkResults::m_table
   */
  uint8_t DoGetTableIndex (SatLinkResults::SatModcod_e modcod,
                           uint16_t burstLength) const;
};


} // end of namespace ns3


#endif /* SATELLITE_LINK_RESULTS_H */
