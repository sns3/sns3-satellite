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

#include <ns3/core-module.h>
#include <ns3/satellite-look-up-table.h>
#include <ns3/gnuplot.h>
#include <fstream>

using namespace ns3;


// forward declaration
int main (int argc, char *argv[]);


/**
 * This example will generate several Gnuplot files (.plt) as output. Each of
 * these files can be converted to PNG, for example by this command:
 *
 *     gnuplot s2-32apsk.plt
 *
 * which will produce `s2-32apsk.png` file. To convert all the Gnuplot files in
 * the directory, the command below can be used:
 *
 *     gnuplot *.plt
 *
 */
class SatLinkResultsPlot : public Object
{
public:
  SatLinkResultsPlot ();
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


NS_LOG_COMPONENT_DEFINE ("SatLinkResultsPlot");
NS_OBJECT_ENSURE_REGISTERED (SatLinkResultsPlot);


int
main (int argc, char *argv[])
{
  Ptr<SatLinkResultsPlot> stub = CreateObject<SatLinkResultsPlot> ();
  Config::RegisterRootNamespaceObject (stub);
  stub->Run ();

  return 0;
}


SatLinkResultsPlot::SatLinkResultsPlot ()
{
  m_resolutionDb = 0.01;
  m_minSinrDb = 0.0;
  m_maxSinrDb = 20.0;
  m_inputPath = "src/satellite/data/linkresults/";
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
  // TODO
}


void
SatLinkResultsPlot::RunDvbRcs2Psk8 ()
{
  // TODO
}


void
SatLinkResultsPlot::RunDvbRcs2Qam16 ()
{
  // TODO
}


void
SatLinkResultsPlot::RunDvbS2Qpsk ()
{
  // TODO
}


void
SatLinkResultsPlot::RunDvbS2Psk8 ()
{
  // TODO
}


void
SatLinkResultsPlot::RunDvbS2Apsk16 ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_2_to_3.txt");;
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_3_to_4.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_4_to_5.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_5_to_6.txt");
  Ptr<SatLookUpTable> table5 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_8_to_9.txt");
  Ptr<SatLookUpTable> table6 = CreateObject<SatLookUpTable> (m_inputPath + "s2_16apsk_9_to_10.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "S2 16APSK 2/3");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "S2 16APSK 3/4");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "S2 16APSK 4/5");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "S2 16APSK 5/6");
  Gnuplot2dDataset dataset5 = GetGnuplotDataset (table5, "S2 16APSK 8/9");
  Gnuplot2dDataset dataset6 = GetGnuplotDataset (table6, "S2 16APSK 9/10");

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
}


void
SatLinkResultsPlot::RunDvbS2Apsk32 ()
{
  Ptr<SatLookUpTable> table1 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_3_to_4.txt");;
  Ptr<SatLookUpTable> table2 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_4_to_5.txt");
  Ptr<SatLookUpTable> table3 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_5_to_6.txt");
  Ptr<SatLookUpTable> table4 = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_8_to_9.txt");

  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table1, "S2 32APSK 3/4");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table2, "S2 32APSK 4/5");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table3, "S2 32APSK 5/6");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table4, "S2 32APSK 8/9");

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

}


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
          if (bler < 10e-15)
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
  return ret;
}
