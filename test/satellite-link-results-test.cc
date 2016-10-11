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

/**
 * \file satellite-link-results-test.cc
 * \ingroup satellite
 * \brief Test cases for satellite link results.
 */

#include <ns3/test.h>
#include <ns3/satellite-link-results.h>
#include <ns3/satellite-look-up-table.h>
#include <ns3/log.h>
#include <ns3/ptr.h>

NS_LOG_COMPONENT_DEFINE ("TestLinkResult");

#define SAT_LINK_RESULTS_PRECISION 0.00011
// TODO precision loss occurs somewhere, causing the test to fail if this is set to 0.00010

namespace ns3 {


/*
 * DVB-RCS2 TEST CASE
 */

/**
 * \brief Test case for comparing a BLER value computed by DVB-RCS2 link results
 *        with a BLER value taken from a reference.
 *
 * The test fails if the difference of both BLER values are greater than
 * define #SAT_LINK_RESULTS_PRECISION.
 */
class SatLinkResultsDvbRcs2TestCase : public TestCase
{
public:
  /**
   * \param linkResults pre-loaded DVB-RCS2 link results
   * \param waveformId the specific DVB-RCS2's waveform ID to be tested
   * \param sinrDb the SINR (in dB unit) which BLER will be computed from
   * \param bler the reference BLER to be compared with the computed BLER
   */
  SatLinkResultsDvbRcs2TestCase (Ptr<SatLinkResultsDvbRcs2> linkResults,
                                 uint32_t waveformId,
                                 double sinrDb,
                                 double bler);
private:
  virtual void DoRun ();
  Ptr<SatLinkResultsDvbRcs2> m_linkResults;
  uint32_t m_waveformId;
  double m_sinrDb;
  double m_bler;
};


SatLinkResultsDvbRcs2TestCase::SatLinkResultsDvbRcs2TestCase (Ptr<SatLinkResultsDvbRcs2> linkResults,
                                                              uint32_t waveformId,
                                                              double sinrDb,
                                                              double bler)
  : TestCase ("Comparing SatLinkResults for DVB-RCS2 with reference link results"),
    m_linkResults (linkResults),
    m_waveformId (waveformId),
    m_sinrDb (sinrDb),
    m_bler (bler)
{
}


void
SatLinkResultsDvbRcs2TestCase::DoRun ()
{
  NS_LOG_FUNCTION (this << m_waveformId << m_sinrDb << m_bler);
  double actualBler = m_linkResults->GetBler (m_waveformId, m_sinrDb);
  NS_LOG_DEBUG (this << " actualBler=" << actualBler);
  NS_TEST_ASSERT_MSG_EQ_TOL (actualBler, m_bler, SAT_LINK_RESULTS_PRECISION,
                             "Difference of " << (actualBler - m_bler)
                                              << " from reference BLER" );
}



/*
 * DVB-S2 TEST CASE
 */

/**
 * \brief Test case for comparing a BLER value computed by DVB-S2 link results
 *        with a BLER value taken from a reference.
 *
 * The test fails if the difference of both BLER values are greater than
 * define #SAT_LINK_RESULTS_PRECISION.
 */
class SatLinkResultsDvbS2TestCase : public TestCase
{
public:
  /**
   * \param linkResults pre-loaded DVB-S2 link results
   * \param modcod the specific DVB-S2's modulation and coding scheme to be
   *               tested
   * \param sinrDb the SINR (in dB unit) which BLER will be computed from
   * \param bler the reference BLER to be compared with the computed BLER
   */
  SatLinkResultsDvbS2TestCase (Ptr<SatLinkResultsDvbS2> linkResults,
                               SatEnums::SatModcod_t modcod,
                               double sinrDb,
                               double bler);
private:
  virtual void DoRun ();
  Ptr<SatLinkResultsDvbS2> m_linkResults;
  SatEnums::SatModcod_t m_modcod;
  double m_sinrDb;
  double m_bler;
};


SatLinkResultsDvbS2TestCase::SatLinkResultsDvbS2TestCase (Ptr<SatLinkResultsDvbS2> linkResults,
                                                          SatEnums::SatModcod_t modcod,
                                                          double sinrDb,
                                                          double bler)
  : TestCase ("Comparing SatLinkResults for DVB-S2 with reference link results"),
    m_linkResults (linkResults),
    m_modcod (modcod),
    m_sinrDb (sinrDb),
    m_bler (bler)
{
}


void
SatLinkResultsDvbS2TestCase::DoRun ()
{
  NS_LOG_FUNCTION (this << m_modcod << m_sinrDb << m_bler);
  double actualBler = m_linkResults->GetBler (m_modcod, SatEnums::NORMAL_FRAME, m_sinrDb);
  NS_LOG_DEBUG (this << " actualBler=" << actualBler);
  NS_TEST_ASSERT_MSG_EQ_TOL (actualBler, m_bler, SAT_LINK_RESULTS_PRECISION,
                             "Difference of " << (actualBler - m_bler)
                                              << " from reference BLER" );
}



/*
 * TEST SUITE
 */

/// Unit test suite for link results (sat-link-results-test).
static class LinkResultTestSuite : public TestSuite
{
public:
  /**
   * \brief Runs a unit test suite of link results.
   *
   * The test suite consists of approximately 460 test cases for DVB-RCS2
   * (SatLinkResultsDvbRcs2TestCase) and 100 test cases for DVB-S2
   * (SatLinkResultsDvbS2TestCase). These test cases cover all possible
   * waveform ID in DVB-RCS2 and all possible combination of Modcod and burst
   * length in DVB-S2. On top of that, these test cases attempt to cover a large
   * range of SINR (with 0.1 dB resolution), which is deemed relevant to be
   * tested.
   *
   * Each test case consists of a simple query to a satellite link results
   * object and a simple value comparison. The link results object is created
   * and loaded once in the beginning of the test suite.
   *
   * The reference BLER values were generated by a separate Octave script based
   * on the values taken from the link results data file.
   *
   * \note If the link results data files have been modified, you might want to
   *       refresh the reference BLER values here to ensure the test still
   *       passes successfully. See the comments inside the code for more
   *       instructions.
   *
   * \warning Link results data files must be available to be read by the
   *          program (by default, they are located in
   *          contrib/satellite/data/linkresults directory).
   */
  LinkResultTestSuite () : TestSuite ("sat-link-results-test", TestSuite::UNIT)
  {
    //LogComponentEnable ("TestLinkResult", LOG_DEBUG);
    //LogComponentEnable ("TestLinkResult", LOG_FUNCTION);

    /*
     * The following lines of test cases are automatically generated by a
     * supplementary Octave script. Run it from the command line as below:
     *
     *     cd contrib/satellite/test/reference
     *     ./generate_test_data_sat_link_results.m > link-results-test-cases.txt
     *
     * Then copy the content of link-results-test-cases.txt here.
     *
     * IMPORTANT: not to add the link-results-test-cases.txt into the commit.
     *
     * The Octave script covers test cases for all possible Modcod and burst
     * length combination, for both DVB-RCS2 and DVB-S2.
     */

    Ptr<SatLinkResultsDvbRcs2> linkResultsDvbRcs2 = CreateObject<SatLinkResultsDvbRcs2> ();
    linkResultsDvbRcs2->Initialize ();

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.000000, 6.911498e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.100000, 6.292555e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.200000, 5.673612e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.300000, 5.077386e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.400000, 4.487563e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.500000, 3.902318e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.600000, 3.420764e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.700000, 2.939210e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.800000, 2.482094e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 0.900000, 2.046961e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.000000, 1.613129e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.100000, 1.301079e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.200000, 9.890285e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.300000, 7.376512e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.400000, 5.482430e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.500000, 3.639905e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.600000, 2.721280e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.700000, 1.802654e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.800000, 1.157371e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 1.900000, 7.477670e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.000000, 3.450232e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.100000, 2.599292e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.200000, 1.748351e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.300000, 1.117287e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.400000, 7.015003e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.500000, 2.857134e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.600000, 1.909090e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.700000, 9.953392e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.800000, 4.403129e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 2.900000, 2.840476e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 3.000000, 1.314560e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 3.100000, 9.103483e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 3.200000, 5.061370e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 3.300000, 2.620454e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 3.400000, 1.682300e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 2, 3.500000, 7.441453e-07), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.000000, 7.817144e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.100000, 6.790229e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.200000, 5.763314e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.300000, 4.855185e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.400000, 3.980535e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.500000, 3.114067e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.600000, 2.432903e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.700000, 1.751739e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.800000, 1.213212e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 0.900000, 8.030019e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.000000, 3.957427e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.100000, 2.646962e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.200000, 1.336496e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.300000, 5.705397e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.400000, 3.607273e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.500000, 1.509148e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.600000, 9.276227e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.700000, 3.788534e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.800000, 7.066240e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 1.900000, 4.149836e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.000000, 1.285204e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.100000, 8.176593e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.200000, 3.501148e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.300000, 9.789296e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.400000, 5.648927e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.500000, 1.508557e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.600000, 1.065368e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.700000, 6.616786e-08), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 3, 2.800000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 0.300000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 0.400000, 9.994978e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 0.500000, 8.807215e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 0.600000, 7.620785e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 0.700000, 6.643040e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 0.800000, 5.665296e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 0.900000, 4.687552e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.000000, 3.675208e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.100000, 2.674436e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.200000, 1.919215e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.300000, 1.163993e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.400000, 6.740337e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.500000, 4.167046e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.600000, 1.593754e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.700000, 1.032355e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.800000, 5.927523e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 1.900000, 3.030152e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.000000, 1.625113e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.100000, 2.200741e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.200000, 1.224897e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.300000, 4.454513e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.400000, 7.195159e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.500000, 3.963587e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.600000, 7.320148e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.700000, 4.140926e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.800000, 2.382996e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 4, 2.900000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.100000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.200000, 9.276762e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.300000, 8.403927e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.400000, 7.531092e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.500000, 6.658257e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.600000, 5.785422e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.700000, 4.912587e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.800000, 3.754477e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 1.900000, 2.590833e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.000000, 1.685598e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.100000, 1.077532e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.200000, 4.798261e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.300000, 3.172782e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.400000, 1.547303e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.500000, 5.256019e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.600000, 2.932293e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.700000, 6.085671e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.800000, 3.683184e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 2.900000, 1.458392e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.000000, 2.400438e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.100000, 1.310212e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.200000, 2.199867e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.300000, 1.136109e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.400000, 4.840939e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.500000, 1.409282e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.600000, 8.518120e-08), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 5, 3.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 1.800000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 1.900000, 7.818919e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.000000, 6.887204e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.100000, 5.738086e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.200000, 4.588967e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.300000, 3.500039e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.400000, 2.470043e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.500000, 1.440046e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.600000, 1.023284e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.700000, 6.130737e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.800000, 3.284380e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 2.900000, 1.833916e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.000000, 3.834531e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.100000, 2.099480e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.200000, 9.282083e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.300000, 2.291456e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.400000, 1.265581e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.500000, 2.397061e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.600000, 1.564672e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.700000, 7.322823e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.800000, 2.563613e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 3.900000, 1.602584e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 4.000000, 6.415541e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 4.100000, 4.351118e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 4.200000, 2.533831e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 6, 4.300000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 2.600000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 2.700000, 7.093809e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 2.800000, 5.368880e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 2.900000, 4.222257e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.000000, 3.075634e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.100000, 2.136591e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.200000, 1.394021e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.300000, 6.514515e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.400000, 4.673571e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.500000, 2.844459e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.600000, 1.576224e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.700000, 9.106860e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.800000, 2.451480e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 3.900000, 1.425570e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.000000, 5.967532e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.100000, 1.198841e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.200000, 7.262256e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.300000, 2.536099e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.400000, 1.526336e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.500000, 7.772262e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.600000, 3.399393e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.700000, 2.238439e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.800000, 1.077485e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 4.900000, 7.334490e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 5.000000, 4.088632e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 7, 5.100000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 3.300000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 3.400000, 9.969445e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 3.500000, 8.709907e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 3.600000, 7.450368e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 3.700000, 6.190830e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 3.800000, 4.935337e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 3.900000, 3.759177e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.000000, 2.593231e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.100000, 1.795966e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.200000, 1.001076e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.300000, 6.443206e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.400000, 2.875657e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.500000, 1.685191e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.600000, 5.337998e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.700000, 2.811786e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.800000, 6.532904e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 4.900000, 3.665382e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 5.000000, 7.978597e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 5.100000, 4.064018e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 5.200000, 4.876987e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 5.300000, 1.928408e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 8, 5.400000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.100000, 9.981260e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.200000, 8.822477e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.300000, 7.663694e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.400000, 6.504911e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.500000, 5.346128e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.600000, 4.185924e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.700000, 2.979222e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.800000, 1.777755e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 4.900000, 1.218493e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.000000, 6.592320e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.100000, 4.006965e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.200000, 1.462693e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.300000, 8.221097e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.400000, 2.556054e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.500000, 1.432783e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.600000, 3.095115e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.700000, 1.664691e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.800000, 3.411040e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 5.900000, 1.685224e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 6.000000, 3.441459e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 6.100000, 1.488146e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 6.200000, 1.876795e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 6.300000, 1.290193e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 9, 6.400000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.100000, 8.948912e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.200000, 7.883464e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.300000, 6.818017e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.400000, 5.749806e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.500000, 4.428055e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.600000, 3.106305e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.700000, 2.132258e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.800000, 1.162001e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 5.900000, 7.480730e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.000000, 3.561726e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.100000, 2.154960e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.200000, 7.481934e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.300000, 4.435559e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.400000, 1.563486e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.500000, 8.450230e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.600000, 1.371956e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.700000, 7.638821e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.800000, 1.558079e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 6.900000, 9.185678e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.000000, 3.085480e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.100000, 1.846878e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.200000, 7.592521e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.300000, 5.195871e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.400000, 2.799221e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.500000, 1.895215e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.600000, 9.952437e-08), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 10, 7.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.100000, 8.432373e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.200000, 6.864747e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.300000, 5.183391e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.400000, 3.508609e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.500000, 2.420818e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.600000, 1.333028e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.700000, 8.344960e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.800000, 3.359645e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 5.900000, 1.881221e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.000000, 4.681365e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.100000, 2.527062e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.200000, 3.727594e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.300000, 1.925696e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.400000, 1.237984e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.500000, 5.352838e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.600000, 7.308697e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.700000, 3.891336e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.800000, 1.679675e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 6.900000, 1.641372e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 11, 7.000000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.100000, 5.852253e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.200000, 4.239509e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.300000, 2.755347e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.400000, 1.708415e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.500000, 9.284103e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.600000, 4.143385e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.700000, 2.061265e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.800000, 7.621791e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 6.900000, 2.678771e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.000000, 1.246815e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.100000, 4.016561e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.200000, 5.794250e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.300000, 2.097907e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.400000, 5.175603e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.500000, 2.266455e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.600000, 1.334596e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.700000, 6.603639e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.800000, 2.509163e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 12, 7.900000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.000000, 7.441453e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.100000, 6.470834e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.200000, 5.500215e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.300000, 4.279509e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.400000, 2.914384e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.500000, 1.812276e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.600000, 7.151297e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.700000, 3.952982e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.800000, 7.787641e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 0.900000, 4.298915e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 1.000000, 8.101885e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 1.100000, 3.157514e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 1.200000, 2.761747e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 1.300000, 1.420952e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 1.400000, 9.942121e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 1.500000, 6.031138e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 13, 1.600000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 0.300000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 0.400000, 9.996202e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 0.500000, 9.098018e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 0.600000, 8.199834e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 0.700000, 7.301650e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 0.800000, 6.065889e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 0.900000, 4.275675e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.000000, 2.485460e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.100000, 1.335388e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.200000, 8.446907e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.300000, 3.539931e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.400000, 6.482692e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.500000, 1.341227e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.600000, 3.503110e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.700000, 2.223144e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.800000, 9.910793e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 1.900000, 5.222454e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 2.000000, 2.427844e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 14, 2.100000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.100000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.200000, 9.990862e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.300000, 9.270510e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.400000, 8.550157e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.500000, 7.829804e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.600000, 6.522806e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.700000, 4.529593e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.800000, 2.536381e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 1.900000, 1.065576e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.000000, 2.956229e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.100000, 1.175051e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.200000, 2.882795e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.300000, 4.478367e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.400000, 8.773443e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.500000, 5.060753e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.600000, 5.482685e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.700000, 7.064382e-08), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 15, 2.800000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 1.800000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 1.900000, 9.254417e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.000000, 8.375196e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.100000, 6.419553e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.200000, 4.463910e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.300000, 2.508267e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.400000, 1.286721e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.500000, 4.645524e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.600000, 8.276059e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.700000, 3.164645e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.800000, 5.653109e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 2.900000, 4.159425e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 3.000000, 1.513760e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 3.100000, 1.715380e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 3.200000, 8.557124e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 3.300000, 5.408991e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 16, 3.400000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 2.600000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 2.700000, 7.568104e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 2.800000, 5.810214e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 2.900000, 3.839755e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.000000, 2.316564e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.100000, 1.117350e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.200000, 3.272582e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.300000, 1.425231e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.400000, 4.226564e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.500000, 6.101754e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.600000, 1.432208e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.700000, 1.820291e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.800000, 5.273424e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 3.900000, 2.578003e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 17, 4.000000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 3.400000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 3.500000, 9.800221e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 3.600000, 8.452583e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 3.700000, 7.089506e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 3.800000, 5.306269e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 3.900000, 3.015337e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.000000, 1.522633e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.100000, 5.773019e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.200000, 1.824951e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.300000, 7.102659e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.400000, 7.275700e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.500000, 5.600450e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.600000, 1.366185e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.700000, 5.124435e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 18, 4.800000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.100000, 9.987646e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.200000, 9.223754e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.300000, 8.459862e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.400000, 7.695970e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.500000, 5.523655e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.600000, 3.646118e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.700000, 2.084802e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.800000, 1.033381e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 4.900000, 3.586738e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 5.000000, 8.103938e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 5.100000, 2.774608e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 5.200000, 1.942218e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 5.300000, 1.712022e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 5.400000, 2.977814e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 19, 5.500000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.100000, 9.449166e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.200000, 8.890807e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.300000, 7.336935e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.400000, 5.742820e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.500000, 3.773167e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.600000, 1.803514e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.700000, 9.547863e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.800000, 1.451776e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 5.900000, 7.639156e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.000000, 8.476578e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.100000, 4.322740e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.200000, 1.689019e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.300000, 7.061940e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.400000, 5.398445e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.500000, 1.867857e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.600000, 6.645113e-08), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 20, 6.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.100000, 8.863836e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.200000, 7.221056e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.300000, 5.028237e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.400000, 2.832596e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.500000, 1.224264e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.600000, 2.535820e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.700000, 9.574199e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.800000, 1.332408e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 5.900000, 3.738657e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 6.000000, 1.302396e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 21, 6.100000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 5.900000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.000000, 8.438792e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.100000, 6.097244e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.200000, 4.151294e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.300000, 2.611932e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.400000, 1.072569e-01), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.500000, 2.030424e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.600000, 2.195825e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.700000, 6.120170e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.800000, 3.063726e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 6.900000, 1.514538e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 7.000000, 8.829022e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 7.100000, 5.035423e-07), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbRcs2TestCase (linkResultsDvbRcs2, 22, 7.200000, 0.000000e+00), TestCase::QUICK);

    Ptr<SatLinkResultsDvbS2> linkResultsDvbS2 = CreateObject<SatLinkResultsDvbS2> ();
    linkResultsDvbS2->Initialize ();

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_2_TO_3, 8.800000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_2_TO_3, 8.900000, 2.628144e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_2_TO_3, 9.000000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_3_TO_4, 10.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_3_TO_4, 10.100000, 5.916896e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_3_TO_4, 10.200000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_4_TO_5, 10.900000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_4_TO_5, 11.000000, 4.352778e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_4_TO_5, 11.100000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_5_TO_6, 11.400000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_5_TO_6, 11.500000, 1.872091e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_5_TO_6, 11.600000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_8_TO_9, 12.700000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_8_TO_9, 12.800000, 5.153902e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_8_TO_9, 12.900000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_9_TO_10, 13.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_16APSK_9_TO_10, 13.100000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_3_TO_4, 12.500000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_3_TO_4, 12.600000, 7.745869e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_3_TO_4, 12.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_4_TO_5, 13.500000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_4_TO_5, 13.600000, 8.400029e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_4_TO_5, 13.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_5_TO_6, 14.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_5_TO_6, 14.100000, 4.465213e-02), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_5_TO_6, 14.200000, 1.006686e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_5_TO_6, 14.300000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_8_TO_9, 15.500000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_8_TO_9, 15.600000, 3.956813e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_32APSK_8_TO_9, 15.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_2_TO_3, 6.400000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_2_TO_3, 6.500000, 2.259423e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_2_TO_3, 6.600000, 1.165528e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_2_TO_3, 6.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_3_TO_4, 7.700000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_3_TO_4, 7.800000, 2.816085e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_3_TO_4, 7.900000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_3_TO_5, 5.300000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_3_TO_5, 5.400000, 3.203489e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_3_TO_5, 5.500000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_5_TO_6, 9.200000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_5_TO_6, 9.300000, 2.111862e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_5_TO_6, 9.400000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_8_TO_9, 10.500000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_8_TO_9, 10.600000, 1.700524e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_8_TO_9, 10.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_9_TO_10, 10.800000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_9_TO_10, 10.900000, 6.076032e-05), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_8PSK_9_TO_10, 11.000000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_1_TO_2, 0.800000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_1_TO_2, 0.900000, 3.422314e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_1_TO_2, 1.000000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_2_TO_3, 2.900000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_2_TO_3, 3.000000, 2.040436e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_2_TO_3, 3.100000, 1.271999e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_2_TO_3, 3.200000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_3_TO_4, 3.900000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_3_TO_4, 4.000000, 9.711601e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_3_TO_4, 4.100000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_3_TO_5, 2.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_3_TO_5, 2.100000, 3.477788e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_3_TO_5, 2.200000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_4_TO_5, 4.500000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_4_TO_5, 4.600000, 4.260414e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_4_TO_5, 4.700000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_5_TO_6, 5.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_5_TO_6, 5.100000, 9.526409e-04), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_5_TO_6, 5.200000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_8_TO_9, 6.000000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_8_TO_9, 6.100000, 3.650592e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_8_TO_9, 6.200000, 0.000000e+00), TestCase::QUICK);

    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_9_TO_10, 6.200000, 1.000000e+00), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_9_TO_10, 6.300000, 5.528587e-03), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_9_TO_10, 6.400000, 2.488861e-06), TestCase::QUICK);
    AddTestCase (new SatLinkResultsDvbS2TestCase (linkResultsDvbS2, SatEnums::SAT_MODCOD_QPSK_9_TO_10, 6.500000, 0.000000e+00), TestCase::QUICK);

    // END OF AUTO-GENERATED TEST CASES

  } // end of LinkResultTestSuite ()

} g_linkResultTestSuite;
// end of static class LinkResultTestSuite

} // end of namespace ns3

