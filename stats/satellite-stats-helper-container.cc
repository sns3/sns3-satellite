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
#include <ns3/satellite-stats-delay-helper.h>
#include <ns3/satellite-stats-throughput-helper.h>

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


/*
 * The macro definitions following this comment block are used to define most
 * attributes of this class. Below is the list of attributes created using this
 * C++ pre-processing approach.
 *
 * - [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppDelay
 * - [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Delay
 * - [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppThroughput
 * - [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Throughput
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                    \
  MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",             \
                   SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",      \
                   SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",     \
                   SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT"))

#define ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                             \
  MakeEnumChecker (SatStatsHelper::OUTPUT_NONE,           "NONE",             \
                   SatStatsHelper::OUTPUT_SCALAR_FILE,    "SCALAR_FILE",      \
                   SatStatsHelper::OUTPUT_SCATTER_FILE,   "SCATTER_FILE",     \
                   SatStatsHelper::OUTPUT_HISTOGRAM_FILE, "HISTOGRAM_FILE",   \
                   SatStatsHelper::OUTPUT_PDF_FILE,       "PDF_FILE",         \
                   SatStatsHelper::OUTPUT_CDF_FILE,       "CDF_FILE",         \
                   SatStatsHelper::OUTPUT_SCATTER_PLOT,   "SCATTER_PLOT",     \
                   SatStatsHelper::OUTPUT_HISTOGRAM_PLOT, "HISTOGRAM_PLOT",   \
                   SatStatsHelper::OUTPUT_PDF_PLOT,       "PDF_PLOT",         \
                   SatStatsHelper::OUTPUT_CDF_PLOT,       "CDF_PLOT"))

#define ADD_SAT_STATS_ATTRIBUTE_HEAD(id, desc)                                \
  .AddAttribute (#id,                                                         \
                 std::string ("Enable the output of ") + desc,                \
                 EnumValue (SatStatsHelper::OUTPUT_NONE),                     \
                 MakeEnumAccessor (&SatStatsHelperContainer::Add ## id),      \

#define ADD_SAT_STATS_ATTRIBUTES_BASIC_SET(id, desc)                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id,                                 \
                                std::string ("global ") + desc)               \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id,                                  \
                                std::string ("per GW ") + desc)               \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id,                                \
                                std::string ("per beam ") + desc)             \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER                                          \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id,                                  \
                                std::string ("per UT ") + desc)               \
  ADD_SAT_STATS_BASIC_OUTPUT_CHECKER

#define ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET(id, desc)                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (Global ## id,                                 \
                                std::string ("global ") + desc)               \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerGw ## id,                                  \
                                std::string ("per GW ") + desc)               \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerBeam ## id,                                \
                                std::string ("per beam ") + desc)             \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER                                   \
  ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUt ## id,                                  \
                                std::string ("per UT ") + desc)               \
  ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

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

    // Forward link application-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (FwdAppDelay,
                                               "forward link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppDelay,
                                  "per UT user forward link application-level delay statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

    // Forward link device-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdDevDelay,
                                        "forward link device-level delay statistics")

    // Forward link MAC-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdMacDelay,
                                        "forward link MAC-level delay statistics")

    // Forward link PHY-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdPhyDelay,
                                        "forward link PHY-level delay statistics")

    // Forward link application-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdAppThroughput,
                                        "forward link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserFwdAppThroughput,
                                  "per UT user forward link application-level throughput statistics")
    ADD_SAT_STATS_BASIC_OUTPUT_CHECKER

    // Forward link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdDevThroughput,
                                        "forward link device-level throughput statistics")

    // Forward link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdMacThroughput,
                                        "forward link MAC-level throughput statistics")

    // Forward link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (FwdPhyThroughput,
                                        "forward link PHY-level throughput statistics")

    // Return link application-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_DISTRIBUTION_SET (RtnAppDelay,
                                               "return link application-level delay statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppDelay,
                                  "per UT user return link application-level delay statistics")
    ADD_SAT_STATS_DISTRIBUTION_OUTPUT_CHECKER

    // Return link device-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnDevDelay,
                                        "return link device-level delay statistics")

    // Return link MAC-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnMacDelay,
                                        "return link MAC-level delay statistics")

    // Return link PHY-level packet delay statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnPhyDelay,
                                        "return link PHY-level delay statistics")

    // Return link application-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnAppThroughput,
                                        "return link application-level throughput statistics")
    ADD_SAT_STATS_ATTRIBUTE_HEAD (PerUtUserRtnAppThroughput,
                                  "per UT user return link application-level throughput statistics")
    ADD_SAT_STATS_BASIC_OUTPUT_CHECKER

    // Return link device-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnDevThroughput,
                                        "return link device-level throughput statistics")

    // Return link MAC-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnMacThroughput,
                                        "return link MAC-level throughput statistics")

    // Return link PHY-level throughput statistics.
    ADD_SAT_STATS_ATTRIBUTES_BASIC_SET (RtnPhyThroughput,
                                        "return link PHY-level throughput statistics")
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


/*
 * The macro definitions following this comment block are used to declare the
 * majority of methods in this class. Below is the list of the class methods
 * created using this C++ pre-processing approach.
 *
 * - Add [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppDelay
 * - Add [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Delay
 * - Add [Global,PerGw,PerBeam,PerUt,PerUtUser] [Fwd,Rtn] AppThroughput
 * - Add [Global,PerGw,PerBeam,PerUt] [Fwd,Rtn] [Dev,Mac,Phy] Throughput
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define SAT_STATS_GLOBAL_METHOD_DEFINITION(id, name)                          \
void                                                                          \
SatStatsHelperContainer::AddGlobal ## id (SatStatsHelper::OutputType_t type)  \
{                                                                             \
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
  if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = Create<SatStats ## id ## Helper> (m_satHelper);                     \
      stat->SetName (m_name + "-global-" + name                               \
                            + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);            \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
}

#define SAT_STATS_PER_GW_METHOD_DEFINITION(id, name)                          \
void                                                                          \
SatStatsHelperContainer::AddPerGw ## id (SatStatsHelper::OutputType_t type)   \
{                                                                             \
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
  if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = Create<SatStats ## id ## Helper> (m_satHelper);                     \
      stat->SetName (m_name + "-per-gw-" + name                               \
                            + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_GW);                \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
}

#define SAT_STATS_PER_BEAM_METHOD_DEFINITION(id, name)                        \
void                                                                          \
SatStatsHelperContainer::AddPerBeam ## id (SatStatsHelper::OutputType_t type) \
{                                                                             \
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
  if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = Create<SatStats ## id ## Helper> (m_satHelper);                     \
      stat->SetName (m_name + "-per-beam-" + name                             \
                            + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_BEAM);              \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
}

#define SAT_STATS_PER_UT_METHOD_DEFINITION(id, name)                          \
void                                                                          \
SatStatsHelperContainer::AddPerUt ## id (SatStatsHelper::OutputType_t type)   \
{                                                                             \
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
  if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = Create<SatStats ## id ## Helper> (m_satHelper);                     \
      stat->SetName (m_name + "-per-ut-" + name                               \
                            + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT);                \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
}

#define SAT_STATS_PER_UT_USER_METHOD_DEFINITION(id, name)                     \
void                                                                          \
SatStatsHelperContainer::AddPerUtUser ## id (SatStatsHelper::OutputType_t type) \
{                                                                             \
  NS_LOG_FUNCTION (this << SatStatsHelper::GetOutputTypeName (type));         \
  if (type != SatStatsHelper::OUTPUT_NONE)                                    \
    {                                                                         \
      Ptr<SatStats ## id ## Helper> stat                                      \
        = Create<SatStats ## id ## Helper> (m_satHelper);                     \
      stat->SetName (m_name + "-per-ut-user_" + name                          \
                            + GetOutputTypeSuffix (type));                    \
      stat->SetIdentifierType (SatStatsHelper::IDENTIFIER_UT_USER);           \
      stat->SetOutputType (type);                                             \
      stat->Install ();                                                       \
      m_stats.push_back (stat);                                               \
    }                                                                         \
}

// Forward link application-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdAppDelay, "fwd-app-delay")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (FwdAppDelay, "fwd-app-delay")

// Forward link device-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdDevDelay, "fwd-dev-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdDevDelay, "fwd-dev-delay")

// Forward link MAC-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdMacDelay, "fwd-mac-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdMacDelay, "fwd-mac-delay")

// Forward link PHY-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdPhyDelay, "fwd-phy-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdPhyDelay, "fwd-phy-delay")

// Forward link application-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdAppThroughput, "fwd-app-throughput")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (FwdAppThroughput, "fwd-app-throughput")

// Forward link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdDevThroughput, "fwd-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdDevThroughput, "fwd-dev-throughput")

// Forward link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdMacThroughput, "fwd-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdMacThroughput, "fwd-mac-throughput")

// Forward link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (FwdPhyThroughput, "fwd-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (FwdPhyThroughput, "fwd-phy-throughput")

// Return link application-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnAppDelay, "rtn-app-delay")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (RtnAppDelay, "rtn-app-delay")

// Return link device-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnDevDelay, "rtn-dev-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnDevDelay, "rtn-dev-delay")

// Return link MAC-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnMacDelay, "rtn-mac-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnMacDelay, "rtn-mac-delay")

// Return link PHY-level packet delay statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnPhyDelay, "rtn-phy-delay")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnPhyDelay, "rtn-phy-delay")

// Return link application-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnAppThroughput, "rtn-app-throughput")
SAT_STATS_PER_UT_USER_METHOD_DEFINITION (RtnAppThroughput, "rtn-app-throughput")

// Return link device-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnDevThroughput, "rtn-dev-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnDevThroughput, "rtn-dev-throughput")

// Return link MAC-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnMacThroughput, "rtn-mac-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnMacThroughput, "rtn-mac-throughput")

// Return link PHY-level throughput statistics.
SAT_STATS_GLOBAL_METHOD_DEFINITION      (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_PER_GW_METHOD_DEFINITION      (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_PER_BEAM_METHOD_DEFINITION    (RtnPhyThroughput, "rtn-phy-throughput")
SAT_STATS_PER_UT_METHOD_DEFINITION      (RtnPhyThroughput, "rtn-phy-throughput")


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
