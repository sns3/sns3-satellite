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

#include <cmath>

#include "ns3/log.h"
#include "ns3/fatal-error.h"
#include "satellite-look-up-table.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatLookUpTable");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatLookUpTable);


SatLookUpTable::SatLookUpTable (std::string linkResultPath)
{
  NS_LOG_FUNCTION (this << linkResultPath);
  Load (linkResultPath);
}


SatLookUpTable::~SatLookUpTable ()
{
  NS_LOG_FUNCTION (this);
}


void
SatLookUpTable::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_esNoDb.clear ();
  m_bler.clear ();

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
SatLookUpTable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatLookUpTable")
    .SetParent<Object> ()
  ;
  return tid;
}



double
SatLookUpTable::GetBler (double esNoDb) const
{
  NS_LOG_FUNCTION (this << esNoDb);

  uint16_t n = m_esNoDb.size ();

  NS_ASSERT (n > 0);
  NS_ASSERT (m_bler.size () == n);

  if (esNoDb < m_esNoDb[0])
    {
      // edge case: very low SINR, return maximum BLER (100% error rate)
      NS_LOG_INFO (this << " Very low SINR -> BLER = 1.0");
      return 1.0;
    }

  uint16_t i = 1;

  while ((i < n) && (esNoDb > m_esNoDb[i]))
    {
      i++;
    }

  NS_LOG_DEBUG (this << " i=" << i << " esno[i]=" << m_esNoDb[i]
                     << " bler[i]=" << m_bler[i]);

  if (i >= n)
    {
      // edge case: very high SINR, return minimum BLER (100% success rate)
      NS_LOG_INFO (this << " Very high SINR -> BLER = 0.0");
      return 0.0;
    }
  else // sinrDb <= m_esNoDb[i]
    {
      // normal case
      NS_ASSERT (i > 0);
      NS_ASSERT (i < n);

      double esno = esNoDb;
      double esno0 = m_esNoDb[i - 1];
      double esno1 = m_esNoDb[i];
      double bler = SatUtils::Interpolate (esno, esno0, esno1, m_bler[i - 1], m_bler[i]);
      NS_LOG_INFO (this << " Interpolate: " << esno << " to BLER = " << bler << "(sinr0: " << esno0 << ", sinr1: " << esno1 << ", bler0: " << m_bler[i - 1] << ", bler1: " << m_bler[i] << ")");

      return bler;
    }

} // end of double SatLookUpTable::GetBler (double sinrDb) const


double
SatLookUpTable::GetEsNoDb (double blerTarget) const
{
  NS_LOG_FUNCTION (this << blerTarget);

  uint16_t n = m_bler.size ();

  NS_ASSERT (n > 0);
  NS_ASSERT (m_esNoDb.size () == n);

  // If the requested BLER is smaller than the smallest BLER entry
  // in the look-up-table
  if (blerTarget < m_bler[n - 1])
    {
      return m_esNoDb[n - 1];
    }

  // The requested BLER is higher than the highest BLER entry
  // in the look-up-table
  if (blerTarget > m_bler[1])
    {
      NS_FATAL_ERROR ("The BLER target is set to be too high!");
    }

  double sinr = 0.0;
  // Go through the list from end to beginning
  for (uint32_t i = 0; i < n; ++i)
    {
      if (blerTarget >= m_bler[i])
        {
          sinr = SatUtils::Interpolate (blerTarget, m_bler[i - 1], m_bler[i], m_esNoDb[i - 1], m_esNoDb[i]);
          NS_LOG_INFO (this << " Interpolate: " << blerTarget << " to SINR = " << sinr << "(bler0: " << m_bler[i - 1] << ", bler1: " << m_bler[i] << ", sinr0: " << m_esNoDb[i - 1] << ", sinr1: " << m_esNoDb[i] << ")");
          return sinr;
        }
    }

  return sinr;
} // end of double SatLookUpTable::GetSinr (double bler) const


void
SatLookUpTable::Load (std::string linkResultPath)
{
  NS_LOG_FUNCTION (this << linkResultPath);

  // READ FROM THE SPECIFIED INPUT FILE

  m_ifs = new std::ifstream (linkResultPath.c_str (), std::ifstream::in);

  if (!m_ifs->is_open ())
    {
      // script might be launched by test.py, try a different base path
      delete m_ifs;
      linkResultPath = "../../" + linkResultPath;
      m_ifs = new std::ifstream (linkResultPath.c_str (), std::ifstream::in);

      if (!m_ifs->is_open ())
        {
          NS_FATAL_ERROR ("The file " << linkResultPath << " is not found.");
        }
    }

  double lastEsNoDb = -100.0; // very low value
  double lastBler = 1.0; // maximum value

  double esNoDb, bler;
  *m_ifs >> esNoDb >> bler;

  while (m_ifs->good ())
    {
      NS_LOG_DEBUG (this << " sinrDb=" << esNoDb << ", bler=" << bler);

      // SANITY CHECK PART I
      if ((esNoDb <= lastEsNoDb) || (bler > lastBler))
        {
          NS_FATAL_ERROR ("The file " << linkResultPath << " is not properly sorted.");
        }

      // record the values
      m_esNoDb.push_back (esNoDb);
      m_bler.push_back (bler);
      lastEsNoDb = esNoDb;
      lastBler = bler;

      // get next row
      *m_ifs >> esNoDb >> bler;
    }

  m_ifs->close ();
  delete m_ifs;
  m_ifs = 0;

  // SANITY CHECK PART II

  // at least contains one row
  if (m_esNoDb.empty ())
    {
      NS_FATAL_ERROR ("Error reading data from file " << linkResultPath << ".");
    }

  // SINR and BLER have same size
  NS_ASSERT (m_esNoDb.size () == m_bler.size ());

} // end of void Load (std::string linkResultPath)


} // end of namespace ns3
