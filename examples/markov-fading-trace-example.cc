/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include <ns3/gnuplot.h>
#include <fstream>
#include <vector>

NS_LOG_COMPONENT_DEFINE ("markov-trace-example");

namespace ns3 {

/**
* \ingroup satellite
*
* \brief Example for Markov fading calculations. Can be used to produce simple fading traces.
*
* This example can be run as it is, without any argument, i.e.:
*
*     ./waf --run="src/satellite/examples/markov-trace-example"
*
* Gnuplot file (markov_fading_trace.plt) will be generated as output. This
* file can be converted to a PNG file, for example by this command:
*
*     gnuplot markov_fading_trace.plt
*
* which will produce `markov_fading_trace.png` file in the same directory.
*/

class SatMarkovFadingExamplePlot : public Object
{
public:
  SatMarkovFadingExamplePlot ();
  static TypeId GetTypeId ();
  void Run ();

private:
  Gnuplot2dDataset GetGnuplotDataset (std::string title);
  Gnuplot GetGnuplot (std::string outputName, std::string title);
  void FadingTraceCb (std::string context, double time, SatEnums::ChannelType_t chType, double fadingValue);
  double GetElevation ();
  double GetVelocity ();

  double m_elevation;
  double m_velocity;
  std::vector <std::pair<double,double> > m_fadingValues;
};

NS_OBJECT_ENSURE_REGISTERED (SatMarkovFadingExamplePlot);

TypeId
SatMarkovFadingExamplePlot::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatMarkovFadingExamplePlot")
    .SetParent<Object> ()
    .AddConstructor<SatMarkovFadingExamplePlot> ();
  return tid;
}

SatMarkovFadingExamplePlot::SatMarkovFadingExamplePlot ()
{
  m_elevation = 45;
  m_velocity = 0;
}

void
SatMarkovFadingExamplePlot::FadingTraceCb (std::string context, double time, SatEnums::ChannelType_t chType, double fadingValue)
{
  std::cout << time << " " << chType << " " << 20 * log10 (fadingValue) << std::endl;
  m_fadingValues.push_back (std::make_pair (time,20 * log10 (fadingValue)));
}

double
SatMarkovFadingExamplePlot::GetElevation ()
{
   return m_elevation;
}

double
SatMarkovFadingExamplePlot::GetVelocity ()
{
   return m_velocity;
}

Gnuplot2dDataset
SatMarkovFadingExamplePlot::GetGnuplotDataset (std::string title)
{
  Gnuplot2dDataset ret;
  ret.SetTitle (title);
  ret.SetStyle (Gnuplot2dDataset::LINES);

  for (uint32_t i = 0; i < m_fadingValues.size (); i++)
    {
      ret.Add (m_fadingValues[i].first, m_fadingValues[i].second);
    }
  return ret;
}

Gnuplot
SatMarkovFadingExamplePlot::GetGnuplot (std::string outputName, std::string title)
{
  Gnuplot ret (outputName + ".png");
  ret.SetTitle (title);
  ret.SetTerminal ("png");
  ret.SetLegend ("Time (s)", "Fading (dB)");
  ret.AppendExtra ("set key top right");
  ret.AppendExtra ("set grid xtics mxtics ytics");
  return ret;
}


void
SatMarkovFadingExamplePlot::Run ()
{
  /// create default Markov & Loo configurations
  Ptr<SatMarkovConf> markovConf = CreateObject<SatMarkovConf> ();

  SatBaseFading::ElevationCallback elevationCb = MakeCallback (&SatMarkovFadingExamplePlot::GetElevation,this);
  SatBaseFading::VelocityCallback velocityCb = MakeCallback (&SatMarkovFadingExamplePlot::GetVelocity,this);

  /// create fading container based on default configuration
  Ptr<SatMarkovContainer> markovContainer = CreateObject<SatMarkovContainer> (markovConf,elevationCb,velocityCb);

  markovContainer->TraceConnect ("FadingTrace","The trace for fading values",MakeCallback (&SatMarkovFadingExamplePlot::FadingTraceCb,this));

  Address macAddress;

  /// run simulation
  for (uint32_t i = 0; i < 100000; i++)
    {
      Simulator::Schedule (MilliSeconds (1 * i), &SatMarkovContainer::DoGetFading, markovContainer, macAddress, SatEnums::FORWARD_USER_CH);
    }

  Simulator::Schedule (MilliSeconds (0), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,0);
  Simulator::Schedule (MilliSeconds (20000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,1);
  Simulator::Schedule (MilliSeconds (40000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,2);
  Simulator::Schedule (MilliSeconds (60000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,0);
  Simulator::Schedule (MilliSeconds (80000), &SatMarkovContainer::LockToSetAndState, markovContainer, 0,1);

  Simulator::Run ();

  Gnuplot2dDataset dataset = GetGnuplotDataset ("Markov Fading Trace");
  Gnuplot plot = GetGnuplot ("markov_fading_trace",
                             "Markov Fading Trace");
  plot.AddDataset (dataset);

  std::string plotFileName = "markov_fading_trace.plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;

  int result = system ("gnuplot markov_fading_trace.plt");

  if (result < 0)
    {
      std::cout << "Unable to open shell process for Gnuplot file conversion, conversion not done!" << std::endl;
    }
  else
    {
      std::cout << "Output file converted to: markov_fading_trace.png" << std::endl;
    }


  Simulator::Destroy ();
}

}

int
main (int argc, char *argv[])
{
  ns3::Ptr<ns3::SatMarkovFadingExamplePlot> stub;
  stub = ns3::CreateObject<ns3::SatMarkovFadingExamplePlot> ();
  ns3::Config::RegisterRootNamespaceObject (stub);
  stub->Run ();

  return 0;
}
