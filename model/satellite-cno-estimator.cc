/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include <numeric>
#include <math.h>
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-cno-estimator.h"

NS_LOG_COMPONENT_DEFINE ("SatCnoEstimator");

namespace ns3 {

// interface class for C/N0 estimators

//static uint32_t pair_add (uint32_t i, const std::pair<Time, uint32_t>& x)
//{
//  return i + x.second;
//}

SatCnoEstimator::SatCnoEstimator ()
{
  NS_LOG_FUNCTION (this);
}

SatCnoEstimator::~SatCnoEstimator ()
{
  NS_LOG_FUNCTION (this);
}

void
SatCnoEstimator::AddSample (double sample)
{
  NS_LOG_FUNCTION (this << sample);

  DoAddSample (sample);
}

double
SatCnoEstimator::GetCnoEstimation ()
{
  NS_LOG_FUNCTION (this);

  return DoGetCnoEstimation ();
}

// class for Basic C/N0 estimator

SatBasicCnoEstimator::SatBasicCnoEstimator ()
  : m_mode (LAST)
{
  NS_LOG_FUNCTION (this);
}

SatBasicCnoEstimator::SatBasicCnoEstimator (SatCnoEstimator::EstimationMode_t mode, Time window)
  : m_window (window),
    m_mode (mode)

{
  NS_LOG_FUNCTION (this);
}

SatBasicCnoEstimator::~SatBasicCnoEstimator ()
{
  NS_LOG_FUNCTION (this);
}

void
SatBasicCnoEstimator::DoAddSample (double sample)
{
  NS_LOG_FUNCTION (this << sample);

  switch (m_mode)
    {
    case LAST:
      m_samples.clear ();
      m_samples.insert (std::make_pair (Simulator::Now (), sample) );
      break;

    case MINIMUM:
      ClearOutdatedSamples ();
      m_samples.insert (std::make_pair (Simulator::Now (), sample) );
      break;

    case AVERAGE:
      ClearOutdatedSamples ();
      m_samples.insert (std::make_pair (Simulator::Now (), sample) );
      break;

    default:
      NS_FATAL_ERROR ("Not supported estimation mode!!!");
      break;
    }
}

double
SatBasicCnoEstimator::DoGetCnoEstimation ()
{
  NS_LOG_FUNCTION (this);

  double estimatedCno = NAN;

  ClearOutdatedSamples ();

  if (  m_samples.empty () == false )
    {
      switch (m_mode)
        {
        case LAST:
          estimatedCno = m_samples.begin ()->second;
          break;

        case MINIMUM:
          for ( SampleMap_t::const_iterator it = m_samples.begin (); it != m_samples.end (); it++ )
            {
              if ( std::isnan (estimatedCno) || (( !std::isnan (it->second)) && (it->second < estimatedCno)) )
                {
                  estimatedCno = it->second;
                }
            }
          break;

        case AVERAGE:
          estimatedCno = std::accumulate (m_samples.begin (), m_samples.end (), 0.0, SatBasicCnoEstimator::AddToSum ) / m_samples.size ();
          break;

        default:
          NS_FATAL_ERROR ("Not supported estimation mode!!!");
          break;
        }
    }

  return estimatedCno;
}

void
SatBasicCnoEstimator::ClearOutdatedSamples ()
{
  NS_LOG_FUNCTION (this);
  SampleMap_t::iterator itLastValid = m_samples.lower_bound (Simulator::Now () - m_window );

  m_samples.erase (m_samples.begin (), itLastValid );
}

} // namespace ns3
