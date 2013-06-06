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

#include "satellite-look-up-table.h"
#include <ns3/log.h>
#include <ns3/fatal-error.h>

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

  m_sinrDb.clear ();
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
SatLookUpTable::GetBler (double sinrDb) const
{
  NS_LOG_FUNCTION (this << sinrDb);

  uint16_t n = m_sinrDb.size ();

  NS_ASSERT (n > 0);
  NS_ASSERT (m_bler.size () == n);

  if (sinrDb < m_sinrDb[0])
    {
      // edge case: very low SINR, return maximum BLER (100% error rate)
      NS_LOG_LOGIC (this << " Very low SINR -> BLER = 1.0");
      return 1.0;
    }

  uint16_t i = 1;

  while ((i < n) && (sinrDb > m_sinrDb[i]))
    {
      i++;
    }

  NS_LOG_DEBUG (this << " i=" << i << " sinr[i]=" << m_sinrDb[i]
                     << " bler[i]=" << m_bler[i]);

  if (i >= n)
    {
      // edge case: very high SINR, return minimum BLER (100% success rate)
      NS_LOG_LOGIC (this << " Very high SINR -> BLER = 0.0");
      return 0.0;
    }
  else // sinrDb <= m_sinrDb[i]
    {
      // normal case
      NS_ASSERT (i > 0);
      NS_ASSERT (i < n);
      double bler = Interpolate (sinrDb, m_sinrDb[i - 1], m_sinrDb[i],
                                m_bler[i - 1], m_bler[i]);
      NS_LOG_LOGIC (this << " Interpolate to BLER = " << bler);
      return bler;
    }

}


double
SatLookUpTable::Interpolate (double x, double x0, double x1, double y0,
                             double y1)
{
  NS_LOG_FUNCTION (x << x0 << x1 << y0 << y1);

  double dX = x1 - x0;
  double dY = y1 - y0;
  double relX = x - x0;
  double relY = (dY / dX) * relX;
  return y0 + relY;
}


void
SatLookUpTable::Load (std::string linkResultPath)
{
  // READ FROM THE SPECIFIED INPUT FILE

  m_ifs = new std::ifstream (linkResultPath.c_str (), std::ifstream::in);

  if (!m_ifs->is_open ())
    {
      NS_FATAL_ERROR ("The file " << linkResultPath << " is not found.");
    }

  double lastSinrDb = -100.0; // very low value
  double lastBler = 1.0; // maximum value

  double sinrDb, bler;
  *m_ifs >> sinrDb >> bler;

  while (m_ifs->good ())
    {
      NS_LOG_DEBUG (this << " sinrDb=" << sinrDb << ", bler=" << bler);

      // SANITY CHECK PART I
      if ((sinrDb <= lastSinrDb) || (bler > lastBler))
        {
          NS_FATAL_ERROR ("The file " << linkResultPath << " is not properly sorted.");
        }

      // record the values
      m_sinrDb.push_back (sinrDb);
      m_bler.push_back (bler);
      lastSinrDb = sinrDb;
      lastBler = bler;

      // get next row
      *m_ifs >> sinrDb >> bler;
    }

  m_ifs->close ();
  delete m_ifs;
  m_ifs = 0;

  // SANITY CHECK PART II

  // at least contains one row
  if (m_sinrDb.empty ())
    {
      NS_FATAL_ERROR ("Error reading data from file " << linkResultPath << ".");
    }

  // SINR and BLER have same size
  NS_ASSERT (m_sinrDb.size () == m_bler.size ());

} // end of void Load (std::string linkResultPath)


} // end of namespace ns3
