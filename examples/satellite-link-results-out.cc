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


/**
 * Calling this program without arguments will produce no effect. The possible
 * arguments are:
 *  - ``--gnuplot=true``
 *  - ``--testwriter=tests.txt``
 *
 * For example:
 * \code
 *   ./waf --run="src/satellite/examples/satellite-link-results-out --gnuplot=true"
 * \endcode
 */
int main (int argc, char *argv[]);


class SatLinkResultsOut : public Object
{
public:
  SatLinkResultsOut (int argc, char *argv[]);
  void Run ();

private:
  Gnuplot2dDataset GetGnuplotDataset (Ptr<SatLookUpTable> table,
                                      std::string title);

  bool m_isGnuplotEnabled;
  bool m_isTestWriterEnabled;

  double m_resolutionDb;
  double m_minSinrDb;
  double m_maxSinrDb;
  std::string m_inputPath;
}; // end of class SatLinkResultsPlot


NS_LOG_COMPONENT_DEFINE ("SatLinkResultsOut");
NS_OBJECT_ENSURE_REGISTERED (SatLinkResultsOut);


int
main (int argc, char *argv[])
{
  Ptr<SatLinkResultsOut> stub = CreateObject<SatLinkResultsOut> (argc, argv);
  Config::RegisterRootNamespaceObject (stub);
  stub->Run ();

  return 0;
}


SatLinkResultsOut::SatLinkResultsOut (int argc, char *argv[])
{
  // DEFAULT VALUES FOR ARGUMENTS

  m_isGnuplotEnabled = false;
  m_isTestWriterEnabled = false;

  // REPLACING DEFAULT VALUES WITH COMMAND LINE ARGUMENTS

  CommandLine commandLine;
  commandLine.AddValue<bool> ("gnuplot", "", m_isGnuplotEnabled);
  commandLine.AddValue<bool> ("testwriter", "", m_isTestWriterEnabled);
  commandLine.AddValue<double> ("resolution", "", m_resolutionDb);
  commandLine.AddValue<double> ("resolution", "", m_resolutionDb);
  commandLine.Parse (argc, argv);

  // LESS IMPORTANT VARIABLE

  m_resolutionDb = 0.01;
  m_minSinrDb = 0.0;
  m_maxSinrDb = 20.0;
  m_inputPath = "src/satellite/data/linkresults/";
}


void
SatLinkResultsOut::Run ()
{
  Ptr<SatLookUpTable> table;

  table = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_3_to_4.txt");
  Gnuplot2dDataset dataset1 = GetGnuplotDataset (table, "S2 32APSK 3/4");

  table = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_4_to_5.txt");
  Gnuplot2dDataset dataset2 = GetGnuplotDataset (table, "S2 32APSK 4/5");

  table = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_5_to_6.txt");
  Gnuplot2dDataset dataset3 = GetGnuplotDataset (table, "S2 32APSK 5/6");

  table = CreateObject<SatLookUpTable> (m_inputPath + "s2_32apsk_8_to_9.txt");
  Gnuplot2dDataset dataset4 = GetGnuplotDataset (table, "S2 32APSK 8/9");

  std::string outputName = "s2_32apsk";
  Gnuplot plot (outputName + ".png");
  plot.SetTitle ("Link Results for DVB-S2 with 32APSK");
  plot.SetTerminal ("png");
  plot.SetLegend ("SINR (in dB)", "BLER");
  plot.AddDataset (dataset1);
  plot.AddDataset (dataset2);
  plot.AddDataset (dataset3);
  plot.AddDataset (dataset4);
  plot.AppendExtra ("set key bottom left");
  plot.AppendExtra ("set logscale y 10");
  plot.AppendExtra ("set ytics 10");
  plot.AppendExtra ("set mxtics 5");
  plot.AppendExtra ("set grid xtics mxtics ytics");

  std::string plotFileName = outputName + ".plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

}


Gnuplot2dDataset
SatLinkResultsOut::GetGnuplotDataset (Ptr<SatLookUpTable> table,
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
