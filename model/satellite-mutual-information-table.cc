/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Joaquin Muguerza <joaquin.muguerza@viveris.fr>
 *
 */

#include <cmath>

#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/fatal-error.h"
#include "satellite-mutual-information-table.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatMutualInformationTable");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMutualInformationTable);


SatMutualInformationTable::SatMutualInformationTable (std::string mutualInformationPath) :
  m_beta (1.0)
{
  NS_LOG_FUNCTION (this << mutualInformationPath);
  Load (mutualInformationPath);
}


SatMutualInformationTable::~SatMutualInformationTable ()
{
  NS_LOG_FUNCTION (this);
}


void
SatMutualInformationTable::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_snirDb.clear ();
  m_symbolInformation.clear ();

  if (m_ifs != 0)
    {
      if (m_ifs->is_open ())
        {
          m_ifs->close ();
        }

      delete m_ifs;
      m_ifs = 0;
    }

  Object::DoDispose ();
}


TypeId
SatMutualInformationTable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatMutualInformationTable")
    .SetParent<Object> ()
    .AddAttribute ( "Beta",
                    "The adjusting factor for the MI function",
                    DoubleValue (1.0),
                    MakeDoubleAccessor (&SatMutualInformationTable::m_beta),
                    MakeDoubleChecker<double> ())
  ;
  return tid;
}



double
SatMutualInformationTable::GetNormalizedSymbolInformation (double snirDb) const
{
  NS_LOG_FUNCTION (this << snirDb);

  uint16_t n = m_snirDb.size ();

  NS_ASSERT (n > 0);
  NS_ASSERT (m_symbolInformation.size () == n);

  if (snirDb < m_snirDb[0])
    {
      // edge case: very low SNIR, return minimum Symbol Information
      NS_LOG_INFO (this << " Very low SNIR -> Symbol Information = 0.0");
      return 0.0;
    }

  uint16_t i = 1;

  while ((i < n) && (snirDb > m_snirDb[i]))
    {
      i++;
    }

  NS_LOG_DEBUG (this << " i=" << i << " snir[i]=" << m_snirDb[i]
                     << " symbolInformation[i]=" << m_symbolInformation[i]);

  if (i >= n)
    {
      // edge case: very high SNIR, return maximum Symbol Information
      NS_LOG_INFO (this << " Very high SNIR -> Symbol Information = 1.0");
      return 1.0;
    }
  else // snirDb <= m_snirDb[i]
    {
      // normal case
      NS_ASSERT (i > 0);
      NS_ASSERT (i < n);

      double snir = snirDb;
      double snir0 = m_snirDb[i - 1];
      double snir1 = m_snirDb[i];
      double symbolInformation = SatUtils::Interpolate (snir, snir0, snir1, m_symbolInformation[i - 1], m_symbolInformation[i]);
      NS_LOG_INFO (this << " Interpolate: " << snir << " to Symbol Information = " << symbolInformation << "(snir0: " << snir0 << ", snir1: " << snir1 << ", symbolInformation0: " << m_symbolInformation[i - 1] << ", symbolInformation1: " << m_symbolInformation[i] << ")");

      return symbolInformation;
    }

} // end of double SatMutualInformationTable::GetNormalizedSymbolInformation (double snirDb) const


double
SatMutualInformationTable::GetSnirDb (double symbolInformationTarget) const
{
  NS_LOG_FUNCTION (this << symbolInformationTarget);

  uint16_t n = m_symbolInformation.size ();

  NS_ASSERT (n > 0);
  NS_ASSERT (m_snirDb.size () == n);

  // If the requested Symbol Information is smaller than the smallest Symbol Information entry
  // in the look-up-table
  if (symbolInformationTarget < m_symbolInformation[0])
    {
      return m_snirDb[0];
    }

  // The requested Symbol Information is higher than the highest Symbol Information entry
  // in the look-up-table
  if (symbolInformationTarget > 1.0)
    {
      NS_FATAL_ERROR ("The Symbol Information target is set to be too high!");
    }

  // The requested Symbol Information is higher than the highest Symbol Information entry
  // in the look-up-table
  if (symbolInformationTarget > m_symbolInformation[n - 1])
    {
      return m_snirDb[n - 1];
    }

  double snir = 0.0;
  // Go through the list from end to beginning
  for (uint32_t i = 0; i < n; ++i)
    {
      if (symbolInformationTarget >= m_symbolInformation[i])
        {
          snir = SatUtils::Interpolate (symbolInformationTarget, m_symbolInformation[i - 1], m_symbolInformation[i], m_snirDb[i - 1], m_snirDb[i]);
          NS_LOG_INFO (this << " Interpolate: " << symbolInformationTarget << " to snir = " << snir << "(symbolInformation0: " << m_symbolInformation[i - 1] << ", symbolInformation1: " << m_symbolInformation[i] << ", snir0: " << m_snirDb[i - 1] << ", snir1: " << m_snirDb[i] << ")");
          return snir;
        }
    }

  return snir;
} // end of double SatMutualInformationTable::GetSnir (double symbolInformationTarget) const


void
SatMutualInformationTable::Load (std::string mutualInformationPath)
{
  NS_LOG_FUNCTION (this << mutualInformationPath);

  // READ FROM THE SPECIFIED INPUT FILE

  m_ifs = new std::ifstream (mutualInformationPath.c_str (), std::ifstream::in);

  if (!m_ifs->is_open ())
    {
      // script might be launched by test.py, try a different base path
      delete m_ifs;
      mutualInformationPath = "../../" + mutualInformationPath;
      m_ifs = new std::ifstream (mutualInformationPath.c_str (), std::ifstream::in);

      if (!m_ifs->is_open ())
        {
          NS_FATAL_ERROR ("The file " << mutualInformationPath << " is not found.");
        }
    }

  double lastSnirDb = -100.0; // very low value
  double lastSymbolInformation = 0.0; // minimum value

  double snirDb, symbolInformation;
  *m_ifs >> snirDb >> symbolInformation;

  while (m_ifs->good ())
    {
      NS_LOG_DEBUG (this << " snirDb=" << snirDb << ", symbolInformation=" << symbolInformation);

      // SANITY CHECK PART I
      if ((snirDb <= lastSnirDb) || (symbolInformation < lastSymbolInformation))
        {
          NS_FATAL_ERROR ("The file " << mutualInformationPath << " is not properly sorted.");
        }

      // record the values
      m_snirDb.push_back (snirDb);
      m_symbolInformation.push_back (symbolInformation);
      lastSnirDb = snirDb;
      lastSymbolInformation = symbolInformation;

      // get next row
      *m_ifs >> snirDb >> symbolInformation;
    }

  m_ifs->close ();
  delete m_ifs;
  m_ifs = 0;

  // SANITY CHECK PART II

  // at least contains one row
  if (m_snirDb.empty ())
    {
      NS_FATAL_ERROR ("Error reading data from file " << mutualInformationPath << ".");
    }

  // SNIR and BLER have same size
  NS_ASSERT (m_snirDb.size () == m_symbolInformation.size ());

} // end of void Load (std::string mutualInformationPath)


} // end of namespace ns3
