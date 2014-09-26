/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/gnuplot.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"
#include "ns3/satellite-look-up-table.h"

/**
 * \file sat-link-results-plot.cc
 * \ingroup satellite
 * \brief Example for plotting satellite link results data.
 *
 * \sa SatLinkResultsPlot
 */
NS_LOG_COMPONENT_DEFINE ("SatLinkResultsPlot");


namespace ns3 {


/**
 * \ingroup satellite
 * \brief Example for plotting satellite link results data.
 *
 * This example can be run as it is, without any argument, i.e.:
 *
 *     $ ./waf --run="link-results-plot"
 *
 * Several Gnuplot files (.plt) will be generated as output. Each of these
 * files can be converted to a PNG file, for example by this command:
 *
 *     $ gnuplot s2-32apsk.plt
 *
 * which will produce `s2-32apsk.png` file in the same directory. To convert all
 * the Gnuplot files in the directory, the command below can be used:
 *
 *     $ gnuplot *.plt
 *
 */
class SatLinkResultsPlot : public SimpleRefCount<SatLinkResultsPlot>
{
public:
  /// Creates an empty instance.
  SatLinkResultsPlot ();
  /// Load link results data files and generate plots from them.
  void Run ();

private:
  void RunDvbRcs2Qpsk ();
  void RunDvbRcs2Psk8 ();
  void RunDvbRcs2Qam16 ();
  void RunDvbS2Qpsk ();
  void RunDvbS2Psk8 ();
  void RunDvbS2Apsk16 ();
  void RunDvbS2Apsk32 ();

  Gnuplot2dDataset GetGnuplotDataset (Ptr<SatLookUpTable> table,
                                      std::string title);
  Gnuplot GetGnuplot (std::string outputName, std::string title);

  double m_resolutionDb;
  double m_minSinrDb;
  double m_maxSinrDb;
  std::string m_inputPath;

}; // end of class SatLinkResultsPlot

SatLinkResultsPlot::SatLinkResultsPlot ()
{
  m_resolutionDb = 0.01;
  m_minSinrDb = 0.0;
  m_maxSinrDb = 20.0;
  m_inputPath = Singleton<SatEnvVariables>::Get ()->GetDataPath () + "/linkresults/";
}

void
SatLinkResultsPlot::Run ()
{
  RunDvbRcs2Qpsk ();
  RunDvbRcs2Psk8 ();
  RunDvbRcs2Qam16 ();
  RunDvbS2Qpsk ();
  RunDvbS2Psk8 ();
  RunDvbS2Apsk16 ();
  RunDvbS2Apsk32 ();
}

void
SatLinkResultsPlot::RunDvbRcs2Qpsk ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat2.txt");
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat3.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat4.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat5.txt");
  Ptr<SatLookUpTable> table5 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat6.txt");
  Ptr<SatLookUpTable> table6 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat7.txt");
  Ptr<SatLookUpTable> table7 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat13.txt");
  Ptr<SatLookUpTable> table8 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat14.txt");
  Ptr<SatLookUpTable> table9 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat15.txt");
  Ptr<SatLookUpTable> table10 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat16.txt");
  Ptr<SatLookUpTable> table11 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat17.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "Waveform 2: 112 bits, QPSK 1/3");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "Waveform 3: 304 bits, QPSK 1/3");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "Waveform 4: 472 bits, QPSK 1/2");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "Waveform 5: 680 bits, QPSK 2/3");
  Gnuplot2dDataset dataset5 = GetGnuplotDataset (table5, "Waveform 6: 768 bits, QPSK 3/4");
  Gnuplot2dDataset dataset6 = GetGnuplotDataset (table6, "Waveform 7: 864 bits, QPSK 5/6");
  Gnuplot2dDataset dataset7 = GetGnuplotDataset (table7, "Waveform 13: 984 bits, QPSK 1/3");
  Gnuplot2dDataset dataset8 = GetGnuplotDataset (table8, "Waveform 14: 1504 bits, QPSK 1/2");
  Gnuplot2dDataset dataset9 = GetGnuplotDataset (table9, "Waveform 15: 2112 bits, QPSK 2/3");
  Gnuplot2dDataset dataset10 = GetGnuplotDataset (table10, "Waveform 16: 2384 bits, QPSK 3/4");
  Gnuplot2dDataset dataset11 = GetGnuplotDataset (table11, "Waveform 17: 2664 bits, QPSK 5/6");

  Gnuplot plot = GetGnuplot ("rcs2_qpsk",
                             "Link Results for DVB-RCS2 with QPSK");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);
  plot.AddDataset (dataset5);
  plot.AddDataset (dataset6);
  plot.AddDataset (dataset7);
  plot.AddDataset (dataset8);
  plot.AddDataset (dataset9);
  plot.AddDataset (dataset10);
  plot.AddDataset (dataset11);

  std::string plotFileName = "rcs2_qpsk.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

} // end of void SatLinkResultsPlot::RunDvbRcs2Qpsk ()

void
SatLinkResultsPlot::RunDvbRcs2Psk8 ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat8.txt");
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat9.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat10.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat18.txt");
  Ptr<SatLookUpTable> table5 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat19.txt");
  Ptr<SatLookUpTable> table6 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat20.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "Waveform 8: 920 bits, 8PSK 2/3");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "Waveform 9: 1040 bits, 8PSK 3/4");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "Waveform 10: 1152 bits, 8PSK 5/6");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "Waveform 18: 2840 bits, 8PSK 2/3");
  Gnuplot2dDataset dataset5 = GetGnuplotDataset (table5, "Waveform 19: 3200 bits, 8PSK 3/4");
  Gnuplot2dDataset dataset6 = GetGnuplotDataset (table6, "Waveform 20: 3552 bits, 8PSK 5/6");

  Gnuplot plot = GetGnuplot ("rcs2_8psk",
                             "Link Results for DVB-RCS2 with 8PSK");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);
  plot.AddDataset (dataset5);
  plot.AddDataset (dataset6);

  std::string plotFileName = "rcs2_8psk.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

} // end of void SatLinkResultsPlot::RunDvbRcs2Psk8 ()

void
SatLinkResultsPlot::RunDvbRcs2Qam16 ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat11.txt");
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat12.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat21.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "rcs2_waveformat22.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "Waveform 11: 1400 bits, 16QAM 3/4");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "Waveform 12: 1552 bits, 16QAM 5/6");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "Waveform 21: 4312 bits, 16QAM 3/4");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "Waveform 22: 4792 bits, 16QAM 5/6");

  Gnuplot plot = GetGnuplot ("rcs2_16qam",
                             "Link Results for DVB-RCS2 with 16QAM");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);

  std::string plotFileName = "rcs2_16qam.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

} // end of void SatLinkResultsPlot::RunDvbRcs2Qam16 ()

void
SatLinkResultsPlot::RunDvbS2Qpsk ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_1_to_2.txt");
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_2_to_3.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_3_to_4.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_3_to_5.txt");
  Ptr<SatLookUpTable> table5 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_4_to_5.txt");
  Ptr<SatLookUpTable> table6 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_5_to_6.txt");
  Ptr<SatLookUpTable> table7 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_8_to_9.txt");
  Ptr<SatLookUpTable> table8 = CreateObject<SatLookUpTable> (m_inputPath + "s2_qpsk_9_to_10.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "QPSK 1/2");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "QPSK 2/3");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "QPSK 3/4");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "QPSK 3/5");
  Gnuplot2dDataset dataset5 = GetGnuplotDataset (table5, "QPSK 4/5");
  Gnuplot2dDataset dataset6 = GetGnuplotDataset (table6, "QPSK 5/6");
  Gnuplot2dDataset dataset7 = GetGnuplotDataset (table7, "QPSK 8/9");
  Gnuplot2dDataset dataset8 = GetGnuplotDataset (table8, "QPSK 9/10");

  Gnuplot plot = GetGnuplot ("s2_qpsk", "Link Results for DVB-S2 with QPSK");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);
  plot.AddDataset (dataset5);
  plot.AddDataset (dataset6);
  plot.AddDataset (dataset7);
  plot.AddDataset (dataset8);

  std::string plotFileName = "s2_qpsk.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

} // end of void SatLinkResultsPlot::RunDvbS2Qpsk ()

void
SatLinkResultsPlot::RunDvbS2Psk8 ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_2_to_3.txt");
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_3_to_4.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_3_to_5.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_5_to_6.txt");
  Ptr<SatLookUpTable> table5 = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_8_to_9.txt");
  Ptr<SatLookUpTable> table6 = CreateObject<SatLookUpTable> (m_inputPath + "s2_8psk_9_to_10.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "8PSK 2/3");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "8PSK 3/4");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "8PSK 3/5");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "8PSK 5/6");
  Gnuplot2dDataset dataset5 = GetGnuplotDataset (table5, "8PSK 8/9");
  Gnuplot2dDataset dataset6 = GetGnuplotDataset (table6, "8PSK 9/10");

  Gnuplot plot = GetGnuplot ("s2_8psk", "Link Results for DVB-S2 with 8PSK");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);
  plot.AddDataset (dataset5);
  plot.AddDataset (dataset6);

  std::string plotFileName = "s2_8psk.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

} // end of void SatLinkResultsPlot::RunDvbS2Psk8 ()

void
SatLinkResultsPlot::RunDvbS2Apsk16 ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_2_to_3.txt");
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_3_to_4.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_4_to_5.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_5_to_6.txt");
  Ptr<SatLookUpTable> table5 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_8_to_9.txt");
  Ptr<SatLookUpTable> table6 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_9_to_10.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "16APSK 2/3");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "16APSK 3/4");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "16APSK 4/5");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "16APSK 5/6");
  Gnuplot2dDataset dataset5 = GetGnuplotDataset (table5, "16APSK 8/9");
  Gnuplot2dDataset dataset6 = GetGnuplotDataset (table6, "16APSK 9/10");

  Gnuplot plot = GetGnuplot ("s2_16apsk",
                             "Link Results for DVB-S2 with 16APSK");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);
  plot.AddDataset (dataset5);
  plot.AddDataset (dataset6);

  std::string plotFileName = "s2_16apsk.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

} // end of void SatLinkResultsPlot::RunDvbS2Apsk16 ()

void
SatLinkResultsPlot::RunDvbS2Apsk32 ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_3_to_4.txt");
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_4_to_5.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_5_to_6.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_8_to_9.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "32APSK 3/4");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "32APSK 4/5");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "32APSK 5/6");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "32APSK 8/9");

  Gnuplot plot = GetGnuplot ("s2_32apsk",
                             "Link Results for DVB-S2 with 32APSK");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);

  std::string plotFileName = "s2_32apsk.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

} // end of void SatLinkResultsPlot::RunDvbS2Apsk32 ()

Gnuplot2dDataset
SatLinkResultsPlot::GetGnuplotDataset (Ptr<SatLookUpTable> table,
                                       std::string title)
{
  Gnuplot2dDataset ret;
  ret.SetTitle (title);
  ret.SetStyle (Gnuplot2dDataset::LINES);

  bool writeFlag = false;
  double bler;

  for (double sinr = m_minSinrDb; sinr < m_maxSinrDb; sinr += m_resolutionDb)
    {
      bler = table->GetBler (sinr);

      if (!writeFlag)
        {
          if (bler < 1)
            {
              // first time error rate is not 100%
              writeFlag = true; // start writing the plot
              ret.Add (sinr - m_resolutionDb, 1.0); // also add the previous point
              ret.Add (sinr, bler);
            }
        }
      else
        {
          if (bler < 1e-10)
            {
              // close enough to 0% error rate
              writeFlag = false; // stop writing the plot
              sinr = m_maxSinrDb; // exit for
            }
          else
            {
              ret.Add (sinr, bler);
            }
        }

    } // end of for (sinr = m_minSinrDb; sinr < m_maxSinrDb; sinr += m_resolutionDb)

  return ret;

} // end of GetGnuplotDataset

Gnuplot
SatLinkResultsPlot::GetGnuplot (std::string outputName, std::string title)
{
  Gnuplot ret (outputName + ".png");
  ret.SetTitle (title);
  ret.SetTerminal ("png");
  ret.SetLegend ("SINR (in dB)", "BLER");
  ret.AppendExtra ("set key bottom left");
  ret.AppendExtra ("set logscale y 10");
  ret.AppendExtra ("set ytics 10");
  ret.AppendExtra ("set mxtics 5");
  ret.AppendExtra ("set grid xtics mxtics ytics");
  // TODO probably better if the X axis and its labels are on top and maybe emphasize major grid lines more than minor ones
  return ret;
}


} // end of namespace ns3

int
main (int argc, char *argv[])
{
  ns3::Ptr<ns3::SatLinkResultsPlot> stub;
  stub = ns3::Create<ns3::SatLinkResultsPlot> ();
  stub->Run ();

  return 0;
}
