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

#include <map>

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
 * provide query service on it.
 *
 * Usage example:
 *
 *     Ptr<SatLinkResultsDvbRcs2> linkDvbRcs2 = CreateObject<SatLinkResultsDvbRcs2> ();
 *     Ptr<SatLinkResultsDvbS2> linkDvbS2 = CreateObject<SatLinkResultsDvbRcs2> ();
 *     linkDvbRcs2->Initialize ();
 *     linkDvbS2->Initialize ();
 *
 */
class SatLinkResults : public Object
{
public:

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

protected:
  /**
   * \brief Initialize look up tables.
   *
   * Child classes must implement this function to initialize
   * m_table member variable. This is typically done by loading
   * pre-defined input files from the file system. In case of failure, the
   * function should throw an error by calling `NS_FATAL_ERROR`.
   */
  virtual void DoInitialize () = 0;

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
 * waveform id.
 *
 * See usage examples in the parent class documentation (SatLinkResults).
 */
class SatLinkResultsDvbRcs2 : public SatLinkResults
{
public:
  static TypeId GetTypeId ();

  /**
   * \brief Get a BLER value from link results.
   *
   * \param waveformId (which determines burst length and MODCOD)
   * \param sinrDb the received Es/No in dB
   * \return BLER value, which is a `double` ranging between [0..1]
   *
   * Must be run after SatLinkResults::Initialize is called.
   *
   */
  double GetBler (uint32_t waveformId, double sinrDb) const;

  /**
   * \brief Get a Es/No requirement for a given BLER target from link results.
   *
   * \param waveformId (which determines burst length and MODCOD)
   * \param blerTarget Target BLER for the system
   * \return Es/No value
   *
   * Must be run after SatLinkResults::Initialize is called.
   *
   */
  double GetEsNoDb (uint32_t waveformId, double blerTarget) const;

protected:
  /**
   * \brief Initialize by loading DVB-RCS2 look up tables.
   */
  void DoInitialize ();

private:

  /**
   * \brief Map of satellite link result look up tables.
   * - key = uint32_t, i.e. waveform id
   * - value = Ptr<SatLookUpTable>, i.e. look-up table containing the link results
   */
  std::map<uint32_t, Ptr<SatLookUpTable> > m_table;
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
  /**
   * \enum SatModcod_e
   *
   * \brief Modulation scheme and coding rate for DVB-S2.
   */
  enum SatModcod_e
  {
    SAT_MODCOD_QPSK_1_TO_2 = 0,
    SAT_MODCOD_QPSK_2_TO_3,
    SAT_MODCOD_QPSK_3_TO_4,
    SAT_MODCOD_QPSK_3_TO_5,
    SAT_MODCOD_QPSK_4_TO_5,
    SAT_MODCOD_QPSK_5_TO_6,
    SAT_MODCOD_QPSK_8_TO_9,
    SAT_MODCOD_QPSK_9_TO_10,
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
    SAT_MODCOD_32APSK_3_TO_4,
    SAT_MODCOD_32APSK_4_TO_5,
    SAT_MODCOD_32APSK_5_TO_6,
    SAT_MODCOD_32APSK_8_TO_9
  };

  static TypeId GetTypeId ();

  /**
   * \brief Get a BLER value from link results.
   *
   * \param modcod Modulation and coding scheme
   * \param sinrDb the received Es/No in dB
   * \return BLER value, which is a `double` ranging between [0..1]
   *
   * Must be run after SatLinkResults::Initialize is called.
   *
   */
  double GetBler (SatLinkResultsDvbS2::SatModcod_e modcod, double sinrDb) const;

  /**
   * \brief Get a Es/No requirement for a given BLER target from link results.
   *
   * \param modcod Modulation and coding scheme
   * \param blerTarget Target BLER for the system
   * \return Es/No value
   *
   * Must be run after SatLinkResults::Initialize is called.
   *
   */
  double GetEsNoDb (SatLinkResultsDvbS2::SatModcod_e modcod, double blerTarget) const;

protected:
  /**
   * \brief Initialize by loading DVB-S2 look up tables.
   */
  void DoInitialize ();

private:

  /**
   * \brief Map of satellite link result look up tables.
   * - key = SatModcod_e, i.e. modulation and coding scheme
   * - value = Ptr<SatLookUpTable>, i.e. look-up table containing the link results
   */
  std::map<SatLinkResultsDvbS2::SatModcod_e, Ptr<SatLookUpTable> > m_table;
};


} // end of namespace ns3


#endif /* SATELLITE_LINK_RESULTS_H */
