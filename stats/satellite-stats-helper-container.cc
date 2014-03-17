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
#include <ns3/satellite-stats-fwd-app-throughput-helper.h>
#include <ns3/satellite-stats-rtn-app-delay-helper.h>
#include <ns3/satellite-stats-rtn-app-throughput-helper.h>

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


#define ADD_SAT_STATS_BASIC_OUTPUT_CHECKER \
  MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE", \
                   SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE", \
                   SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE", \
                   SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT"))

#define ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER \
  MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE", \
                   SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE", \
                   SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE", \
                   SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE", \
                   SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE", \
                   SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE", \
                   SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT", \
                   SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT", \
                   SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT", \
                   SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))

#define ADD_SAT_STATS_ATTRIBUTE_HEAD(id, shortName, longName) \
  .AddAttribute (shortName, \
                 std::string ("Enable the output of ") + longName, \
                 EnumValue (SatStatsHelper::OUTPUT_NONE), \
                 MakeEnumAccessor (&SatStatsHelperContainer::Add ## id), \

#define ADD_SAT_STATS_ATTRIBUTES_BASIC_SET(id, shortName, longName) \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id, \
                                std::string ("Global") + shortName, \
                                std::string ("global ") + longName) \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id, \
                                std::string ("PerGw") + shortName, \
                                std::string ("per GW ") + longName) \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id, \
                                std::string ("PerBeam") + shortName, \
                                std::string ("per beam ") + longName) \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id, \
                                std::string ("PerUt") + shortName, \
                                std::string ("per UT ") + longName) \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER

#define ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET(id, shortName, longName) \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id, \
                                std::string ("Global") + shortName, \
                                std::string ("global ") + longName) \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id, \
                                std::string ("PerGw") + shortName, \
                                std::string ("per GW ") + longName) \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id, \
                                std::string ("PerBeam") + shortName, \
                                std::string ("per beam ") + longName) \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id, \
                                std::string ("PerUt") + shortName, \
                                std::string ("per UT ") + longName) \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

/*
 * Attributes of this class are defined using pre-processing macros above. The
 * following is the complete list of names of created attributes:
 * - GlobalFwdAppDelay
 * - PerGwFwdAppDelay
 * - PerBeamFwdAppDelay
 * - PerUtFwdAppDelay
 * - PerUtUserFwdAppDelay
 * - GlobalFwdAppThroughput
 * - PerGwFwdAppThroughput
 * - PerBeamFwdAppThroughput
 * - PerUtFwdAppThroughput
 * - PerUtUserFwdAppThroughput
 * - GlobalRtnAppDelay
 * - PerGwRtnAppDelay
 * - PerBeamRtnAppDelay
 * - PerUtRtnAppDelay
 * - PerUtUserRtnAppDelay
 * - GlobalRtnAppThroughput
 * - PerGwRtnAppThroughput
 * - PerBeamRtnAppThroughput
 * - PerUtRtnAppThroughput
 * - PerUtUserRtnAppThroughput
 * Also check the Doxygen documentation of this class for more information.
 */

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
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdAppDelay, "FwdAppDelay",
                                               "forward link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppDelay, "PerUtUserFwdAppDelay",
                                  "per UT user forward link application-level delay statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

    // FORWARD LINK APPLICATION-LEVEL THROUGHPUT STATISTICS ATTRIBUTE /////////
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdAppThroughput, "FwdAppThroughput",
                                        "forward link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppThroughput, "PerUtUserFwdAppThroughput",
                                  "per UT user forward link application-level throughput statistics")
    ADD_SAT_STATS_BASIC_OUTPUT_CHECKER

    // RETURN LINK APPLICATION-LEVEL PACKET DELAY STATISTICS //////////////////
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnAppDelay, "RtnAppDelay",
                                               "return link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppDelay, "PerUtUserRtnAppDelay",
                                  "per UT user return link application-level delay statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

    // RETURN LINK APPLICATION-LEVEL THROUGHPUT STATISTICS ATTRIBUTE //////////
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnAppThroughput, "RtnAppThroughput",
                                        "return link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppThroughput, "PerUtUserRtnAppThroughput",
                                  "per UT user return link application-level throughput statistics")
    ADD_SAT_STATS_BASIC_OUTPUT_CHECKER
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
SatStatsHelperContainer::AddGlobalFwdAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppThroughputHelper> stat = Create<SatStatsFwdAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-global-fwd-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerGwFwdAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppThroughputHelper> stat = Create<SatStatsFwdAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-gw-fwd-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerBeamFwdAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppThroughputHelper> stat = Create<SatStatsFwdAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-beam-fwd-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtFwdAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppThroughputHelper> stat = Create<SatStatsFwdAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-fwd-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtUserFwdAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsFwdAppThroughputHelper> stat = Create<SatStatsFwdAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-user-fwd-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


// RETURN LINK APPLICATION-LEVEL PACKET DELAY STATISTICS //////////////////////

void
SatStatsHelperContainer::AddGlobalRtnAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppDelayHelper> stat = Create<SatStatsRtnAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-global-rtn-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerGwRtnAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppDelayHelper> stat = Create<SatStatsRtnAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-gw-rtn-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerBeamRtnAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppDelayHelper> stat = Create<SatStatsRtnAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-beam-rtn-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtRtnAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppDelayHelper> stat = Create<SatStatsRtnAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-rtn-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtUserRtnAppDelay (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppDelayHelper> stat = Create<SatStatsRtnAppDelayHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-user-rtn-app-delay"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


// RETURN LINK THROUGHPUT STATISTICS //////////////////////////////////////////

void
SatStatsHelperContainer::AddGlobalRtnAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppThroughputHelper> stat = Create<SatStatsRtnAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-global-rtn-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerGwRtnAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppThroughputHelper> stat = Create<SatStatsRtnAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-gw-rtn-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerBeamRtnAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppThroughputHelper> stat = Create<SatStatsRtnAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-beam-rtn-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtRtnAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppThroughputHelper> stat = Create<SatStatsRtnAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-rtn-app-throughput"
                            + GetOutputTypeSuffix (outputType));
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);
      stat->SetOutputType (outputType);
      stat->Install ();
      m_stats.push_back (stat);
    }
}


void
SatStatsHelperContainer::AddPerUtUserRtnAppThroughput (SatStatsHelper::OutputType_t outputType)
{
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (outputType));

  if (outputType != SatStatsHelper::OUTPUT_NONE)
    {
      Ptr<SatStatsRtnAppThroughputHelper> stat = Create<SatStatsRtnAppThroughputHelper> (m_satHelper);
      stat->SetName (m_name + "-per-ut-user-rtn-app-throughput"
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
      NS_FATAL_ERROR ("SatStatsHelperContainer - Invalid output type");
      break;
    }

  NS_FATAL_ERROR ("SatStatsHelperContainer - Invalid output type");
  return "";
}


} // end of namespace ns3
