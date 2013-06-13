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

#include <ns3/test.h>
#include <ns3/satellite-link-results.h>
#include <ns3/satellite-look-up-table.h>
#include <ns3/log.h>
#include <ns3/ptr.h>

NS_LOG_COMPONENT_DEFINE("TestLinkResult");

#define SAT_LINK_RESULTS_TOLERANCE 0.000001

namespace ns3 {


/*
 * TEST CASE 1
 */


class SatLookUpTableTestCase : public TestCase
{
public:
  SatLookUpTableTestCase (Ptr<SatLookUpTable> table, double sinrDb,
                          double bler);
private:
  virtual void DoRun ();
  Ptr<SatLookUpTable> m_table;
  double m_sinrDb;
  double m_bler;
};


SatLookUpTableTestCase::SatLookUpTableTestCase (Ptr<SatLookUpTable> table,
                                                double sinrDb, double bler)
  : TestCase ("Simple value comparison test of SatLookUpTable"),
    m_table (table),
    m_sinrDb (sinrDb),
    m_bler (bler)
{
}


void
SatLookUpTableTestCase::DoRun ()
{
  NS_LOG_FUNCTION (this << m_sinrDb << m_bler);
  double actualBler = m_table->GetBler (m_sinrDb);
  NS_TEST_ASSERT_MSG_EQ_TOL(actualBler, m_bler, SAT_LINK_RESULTS_TOLERANCE,
                            "BLER outside tolerable range");
}



/*
 * TEST CASE 2
 */


class SatLinkResultsDvbRcs2TestCase : public TestCase
{
public:
  SatLinkResultsDvbRcs2TestCase (Ptr<SatLinkResultsDvbRcs2> linkResults,
                                 SatLinkResults::SatModcod_e modcod,
                                 uint16_t burstLength, double sinrDb,
                                 double bler);
private:
  virtual void DoRun ();
  Ptr<SatLinkResultsDvbRcs2> m_linkResults;
  SatLinkResults::SatModcod_e m_modcod;
  uint16_t m_burstLength;
  double m_sinrDb;
  double m_bler;
};


SatLinkResultsDvbRcs2TestCase::SatLinkResultsDvbRcs2TestCase (
  Ptr<SatLinkResultsDvbRcs2> linkResults, SatLinkResults::SatModcod_e modcod,
  uint16_t burstLength, double sinrDb, double bler)
  : TestCase ("Simple value comparison test of SatLinkResults"),
    m_linkResults (linkResults),
    m_modcod (modcod),
    m_burstLength (burstLength),
    m_sinrDb (sinrDb),
    m_bler (bler)
{
}


void
SatLinkResultsDvbRcs2TestCase::DoRun ()
{
  NS_LOG_FUNCTION (this << m_modcod << m_burstLength << m_sinrDb << m_bler);
  double actualBler = m_linkResults->GetBler (m_modcod, m_burstLength,
                                              m_sinrDb);
  NS_TEST_ASSERT_MSG_EQ_TOL(actualBler, m_bler, SAT_LINK_RESULTS_TOLERANCE,
                            "BLER outside tolerable range");
}



/*
 * TEST CASE 3
 */


class SatLinkResultsDvbS2TestCase : public TestCase
{
public:
  SatLinkResultsDvbS2TestCase (Ptr<SatLinkResultsDvbS2> linkResults,
                               SatLinkResults::SatModcod_e modcod,
                               uint16_t burstLength, double sinrDb,
                               double bler);
private:
  virtual void DoRun ();
  Ptr<SatLinkResultsDvbS2> m_linkResults;
  SatLinkResults::SatModcod_e m_modcod;
  uint16_t m_burstLength;
  double m_sinrDb;
  double m_bler;
};


SatLinkResultsDvbS2TestCase::SatLinkResultsDvbS2TestCase (
  Ptr<SatLinkResultsDvbS2> linkResults, SatLinkResults::SatModcod_e modcod,
  uint16_t burstLength, double sinrDb, double bler)
  : TestCase ("Simple value comparison test of SatLinkResults"),
    m_linkResults (linkResults),
    m_modcod (modcod),
    m_burstLength (burstLength),
    m_sinrDb (sinrDb),
    m_bler (bler)
{
}


void
SatLinkResultsDvbS2TestCase::DoRun ()
{
  NS_LOG_FUNCTION (this << m_modcod << m_burstLength << m_sinrDb << m_bler);
  double actualBler = m_linkResults->GetBler (m_modcod, m_burstLength,
                                              m_sinrDb);
  NS_TEST_ASSERT_MSG_EQ_TOL(actualBler, m_bler, SAT_LINK_RESULTS_TOLERANCE,
                            "BLER outside tolerable range");
}



/*
 * TEST SUITE
 */


static class LinkResultTestSuite : public TestSuite
{
public:
  LinkResultTestSuite () : TestSuite ("link-results", TestSuite::UNIT)
  {
    //LogComponentEnable ("SatLookUpTable", LOG_LEVEL_ALL);
    //LogComponentEnable ("SatLookUpTable", LOG_PREFIX_TIME);
    //LogComponentEnable ("SatLookUpTable", LOG_PREFIX_FUNC);

//    Ptr<SatLookUpTable> table;
//    std::string basePath = "src/satellite/data/linkresults/";
//
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat10.txt");
//    // arguments: table, sinrDb, expectedBler
//    AddTestCase (new SatLookUpTableTestCase (table, 0.0, 1.0));
//    AddTestCase (new SatLookUpTableTestCase (table, 5.0, 1.0));
//    AddTestCase (new SatLookUpTableTestCase (table, 5.00134, 1.0));
//    AddTestCase (new SatLookUpTableTestCase (table, 5.00134770889488, 1.0));
//    AddTestCase (new SatLookUpTableTestCase (table, 5.3989218328841, 0.576405635790883));
//    AddTestCase (new SatLookUpTableTestCase (table, 5.6010781671159, 0.309205403018096));
//    AddTestCase (new SatLookUpTableTestCase (table, 5.80323450134771, 0.113061825886482));
//    AddTestCase (new SatLookUpTableTestCase (table, 5.99865229110512, 0.0358068560118245));
//    AddTestCase (new SatLookUpTableTestCase (table, 6.20080862533693, 0.0073681796810957));
//    AddTestCase (new SatLookUpTableTestCase (table, 6.39622641509434, 0.00159059813221707));
//    AddTestCase (new SatLookUpTableTestCase (table, 6.59838274932615, 0.000138179027765088));
//    AddTestCase (new SatLookUpTableTestCase (table, 6.80053908355795, 1.52529907364512e-05));
//    AddTestCase (new SatLookUpTableTestCase (table, 7.00269541778976, 2.92105451302717e-06));
//    AddTestCase (new SatLookUpTableTestCase (table, 7.19811320754717, 7.63774033777321e-07));
//    AddTestCase (new SatLookUpTableTestCase (table, 7.40026954177898, 2.7927612512804e-07));
//    AddTestCase (new SatLookUpTableTestCase (table, 7.60242587601078, 9.73411536314302e-08));
//    AddTestCase (new SatLookUpTableTestCase (table, 7.60243, 0.0));

//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat11.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat12.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat13.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat14.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat15.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat16.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat17.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat18.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat19.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat20.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat21.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat22.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat2.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat3.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat4.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat5.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat6.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat7.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat8.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "rcs2_waveformat9.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_16apsk_2_to_3.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_16apsk_3_to_4.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_16apsk_4_to_5.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_16apsk_5_to_6.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_16apsk_8_to_9.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_16apsk_9_to_10.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_32apsk_3_to_4.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_32apsk_4_to_5.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_32apsk_5_to_6.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_32apsk_8_to_9.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_8psk_2_to_3.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_8psk_3_to_4.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_8psk_3_to_5.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_8psk_5_to_6.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_8psk_8_to_9.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_8psk_9_to_10.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_1_to_2.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_2_to_3.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_3_to_4.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_3_to_5.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_4_to_5.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_5_to_6.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_8_to_9.txt");
//    table = CreateObject<SatLookUpTable> (basePath + "s2_qpsk_9_to_10.txt");

    Ptr<SatLinkResultsDvbS2> linkResultsDvbS2 = CreateObject<SatLinkResultsDvbS2> ();
    linkResultsDvbS2->Initialize ();
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatLinkResults::SAT_MODCOD_16APSK_2_TO_3, 626, 8.800000, 1.000000e+00));
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatLinkResults::SAT_MODCOD_16APSK_2_TO_3, 626, 8.900000, 2.628144e-04));
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatLinkResults::SAT_MODCOD_16APSK_2_TO_3, 626, 9.000000, 0.000000e+00));

    // TODO: write a class for plotting
    // TODO: write a class for printing test cases

  }
} g_linkResultTestSuite;
// end of static class LinkResultTestSuite

} // end of namespace ns3

