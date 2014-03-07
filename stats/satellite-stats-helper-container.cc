/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "satellite-stats-helper-container.h"
#include <ns3/log.h>
#include <ns3/enum.h>
#include <ns3/string.h>
#include <ns3/satellite-helper.h>
#include <ns3/satellite-stats-fwd-app-delay-helper.h>
#include <ns3/satellite-stats-fwd-throughput-helper.h>
#include <ns3/satellite-stats-rtn-throughput-helper.h>

NS_LOG_COMPONENT_DEFINE ("SatStatsHelperContainer");


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatStatsHelperContainer);


SatStatsHelperContainer::SatStatsHelperContainer (Ptr<const SatHelper> satHelper)
  : m_satHelper (satHelper)
{
  NS_LOG_FUNCTION (this);
}


void
SatStatsHelperContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}


TypeId // static
SatStatsHelperContainer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatStatsHelperContainer")
    .SetParent<Object> ()
    .AddAttribute ("Name",
                   "String to be prepended on every output file name",
                   StringValue ("stat"),
                   MakeStringAccessor (&SatStatsHelperContainer::SetName,
                                       &SatStatsHelperContainer::GetName),
                   MakeStringChecker ())
    // FORWARD LINK APPLICATION-LEVEL PACKET DELAY STATISTICS /////////////////
    .AddAttribute ("GlobalFwdAppDelay",
                   "Enable the output of per GW forward link application-level delay statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddGlobalFwdAppDelay),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerGwFwdAppDelay",
                   "Enable the output of per GW forward link application-level delay statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerGwFwdAppDelay),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerBeamFwdAppDelay",
                   "Enable the output of per beam forward link application-level delay statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerBeamFwdAppDelay),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerUtFwdAppDelay",
                   "Enable the output of per UT forward link application-level delay statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerUtFwdAppDelay),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerUtUserFwdAppDelay",
                   "Enable the output of per UT user forward link application-level delay statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerUtUserFwdAppDelay),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    // FORWARD LINK THROUGHPUT STATISTICS ATTRIBUTE ///////////////////////////
    .AddAttribute ("GlobalFwdThroughput",
                   "Enable the output of per GW forward link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddGlobalFwdThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerGwFwdThroughput",
                   "Enable the output of per GW forward link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerGwFwdThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerBeamFwdThroughput",
                   "Enable the output of per beam forward link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerBeamFwdThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerUtFwdThroughput",
                   "Enable the output of per UT forward link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerUtFwdThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerUtUserFwdThroughput",
                   "Enable the output of per UT user forward link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerUtUserFwdThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    // RETURN LINK THROUGHPUT STATISTICS ATTRIBUTE ////////////////////////////
    .AddAttribute ("GlobalRtnThroughput",
                   "Enable the output of per GW return link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddGlobalRtnThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerGwRtnThroughput",
                   "Enable the output of per GW return link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerGwRtnThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerBeamRtnThroughput",
                   "Enable the output of per beam return link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerBeamRtnThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerUtRtnThroughput",
                   "Enable the output of per UT return link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerUtRtnThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
    .AddAttribute ("PerUtUserRtnThroughput",
                   "Enable the output of per UT user return link throughput statistics",
                   EnumValue (SatStatsHelper::OUTPUT_NONE),
                   MakeEnumAccessor (&SatStatsHelperContainer::AddPerUtUserRtnThroughput),
                   MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",
                                    SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",
                                    SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",
                                    SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",
                                    SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",
                                    SatStatsHelper::OUTPUT_SCALAR_PLOT,    "SCALAR_PLOT",
                                    SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",
                                    SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",
                                    SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",
                                    SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))
  ;
  return tid;
}


void
SatStatsHelperContainer::SetName (std::string name)
{
  NS_LOG_FUNCTION (this << name);

  // convert all spaces and slashes in the name to underscores
  for (size_t pos = name.find_first_of (" /");
       pos != std::string::npos;
       pos = name.find_first_of (" /", pos + 1, 1))
    {
      name[pos] = '_';
    }

  m_name = name;
}


std::string
SatStatsHelperContainer::GetName () const
{
  return m_name;
}


// FORWARD LINK APPLICATION-LEVEL PACKET DELAY STATISTICS /////////////////////

void
SatStatsHelperContainer::AddGlobalFwdAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppDelayHelper> stat = Create<SatStatsFwdAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-global-fwd-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerGwFwdAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppDelayHelper> stat = Create<SatStatsFwdAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-gw-fwd-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerBeamFwdAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppDelayHelper> stat = Create<SatStatsFwdAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-beam-fwd-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtFwdAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppDelayHelper> stat = Create<SatStatsFwdAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-fwd-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtUserFwdAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppDelayHelper> stat = Create<SatStatsFwdAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-user-fwd-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


// FORWARD LINK THROUGHPUT STATISTICS /////////////////////////////////////////

void
SatStatsHelperContainer::AddGlobalFwdThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-global-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerGwFwdThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-gw-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerBeamFwdThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-beam-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtFwdThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtUserFwdThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdThroughputHelper> stat = Create<SatStatsFwdThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-user-fwd-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


// RETURN LINK THROUGHPUT STATISTICS //////////////////////////////////////////

void
SatStatsHelperContainer::AddGlobalRtnThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnThroughputHelper> stat = Create<SatStatsRtnThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-global-rtn-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerGwRtnThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnThroughputHelper> stat = Create<SatStatsRtnThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-gw-rtn-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerBeamRtnThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnThroughputHelper> stat = Create<SatStatsRtnThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-beam-rtn-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtRtnThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnThroughputHelper> stat = Create<SatStatsRtnThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-rtn-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtUserRtnThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnThroughputHelper> stat = Create<SatStatsRtnThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-user-rtn-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


std::string // static
SatStatsHelperContainer::GetOutputTypeSuffix (SatStatsHelper::OutputType_t outputType)
{
  switch (outputType)
    {
    case SatStatsHelper::OUTPUT_NONE:
      return "";

    case SatStatsHelper::OUTPUT_SCALAR_FILE:
    case SatStatsHelper::OUTPUT_SCALAR_PLOT:
      return "-scalar";

    case SatStatsHelper::OUTPUT_SCATTER_FILE:
    case SatStatsHelper::OUTPUT_SCATTER_PLOT:
      return "-scatter";

    case SatStatsHelper::OUTPUT_HISTOGRAM_FILE:
    case SatStatsHelper::OUTPUT_HISTOGRAM_PLOT:
      return "-histogram";

    case SatStatsHelper::OUTPUT_PDF_FILE:
    case SatStatsHelper::OUTPUT_PDF_PLOT:
      return "-pdf";

    case SatStatsHelper::OUTPUT_CDF_FILE:
    case SatStatsHelper::OUTPUT_CDF_PLOT:
      return "-cdf";

    default:
      NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsHelper - Invalid output type");
  return "";
}


} // end of namespace ns3
