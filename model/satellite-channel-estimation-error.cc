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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <fstream>
#include <cmath>

#include "ns3/log.h"
#include "satellite-channel-estimation-error.h"
#include "satellite-utils.h"

NS_LOG_COMPONENT_DEFINE ("SatChannelEstimationError");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatChannelEstimationError);

SatChannelEstimationError::SatChannelEstimationError ()
  : m_lastSampleIndex (0),
    m_normalRandomVariable (),
    m_sinrsDb (),
    m_mueCesDb (),
    m_stdCesDb ()
{
  m_normalRandomVariable = CreateObject<NormalRandomVariable> ();
}

SatChannelEstimationError::SatChannelEstimationError (std::string filePathName)
  : m_lastSampleIndex (0),
    m_normalRandomVariable (),
    m_sinrsDb (),
    m_mueCesDb (),
    m_stdCesDb ()
{
  m_normalRandomVariable = CreateObject<NormalRandomVariable> ();
  ReadFile (filePathName);
}


SatChannelEstimationError::~SatChannelEstimationError ()
{

}

TypeId
SatChannelEstimationError::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMeasurementError")
    .SetParent<Object> ()
    .AddConstructor<SatChannelEstimationError> ()
  ;
  return tid;
}


void SatChannelEstimationError::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_normalRandomVariable = NULL;
  Object::DoDispose ();
}

void SatChannelEstimationError::ReadFile (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

  if (!ifs->is_open ())
    {
      // script might be launched by test.py, try a different base path
      delete ifs;
      filePathName = "../../" + filePathName;
      ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

      if (!ifs->is_open ())
        {
          NS_FATAL_ERROR ("The file " << filePathName << " is not found.");
        }
    }

  // Start conditions
  double sinrDb, mueCe, stdCe;

  // Read a row
  *ifs >> sinrDb >> mueCe >> stdCe;

  while (ifs->good ())
    {
      m_sinrsDb.push_back (sinrDb);
      m_mueCesDb.push_back (mueCe);
      m_stdCesDb.push_back (stdCe);

      // get next row
      *ifs >> sinrDb >> mueCe >> stdCe;
    }

  NS_ASSERT (m_sinrsDb.size () == m_mueCesDb.size ());
  NS_ASSERT (m_mueCesDb.size () == m_stdCesDb.size ());

  m_lastSampleIndex = m_sinrsDb.size () - 1;

  ifs->close ();
  delete ifs;
}

double
SatChannelEstimationError::AddError (double sinrInDb) const
{
  NS_LOG_FUNCTION (this << sinrInDb);

  // 1. Find the proper SINR grid point
  // 2. Interpolate the mueCe
  // 3. Interpolate the stdCe
  // 4. Through a random number with mean and std deviation
  // 5. Add the error from the SINR in
  // 6. Correct with mueCe

  double mueCe (0.0);
  double stdCe (0.0);

  // If smaller than minimum SINR
  if (sinrInDb <= m_sinrsDb[0])
    {
      mueCe = m_mueCesDb[0];
      stdCe = m_stdCesDb[0];
    }
  // If larger than maximum SINR
  else if (sinrInDb >= m_sinrsDb[m_lastSampleIndex])
    {
      mueCe = m_mueCesDb[m_lastSampleIndex];
      stdCe = m_stdCesDb[m_lastSampleIndex];
    }
  // Else find proper point and interpolate
  else
    {
      for (uint32_t i = 0; i < m_sinrsDb.size (); ++i)
        {
          // Trigger the first bigger threshold
          if (sinrInDb < m_sinrsDb[i])
            {
              /**
               * Interpolate the proper mean and std values
               */
              mueCe = SatUtils::Interpolate (sinrInDb, m_sinrsDb[i - 1], m_sinrsDb[i], m_mueCesDb[i - 1], m_mueCesDb[i]);
              stdCe = SatUtils::Interpolate (sinrInDb, m_sinrsDb[i - 1], m_sinrsDb[i], m_stdCesDb[i - 1], m_stdCesDb[i]);
              break;
            }
        }
    }

  // Convert standard deviation to variance
  double varCe = pow (stdCe, 2);

  NS_LOG_INFO ("mueCe: " << mueCe << ", stdCe: " << stdCe << ", varCe: " << varCe);

  // Get normal random variable error
  double error = m_normalRandomVariable->GetValue (mueCe, varCe);

  // Add error and correct with
  double sinrOutDb = sinrInDb + error - mueCe;

  NS_LOG_INFO ("sinrIn: " << sinrInDb << ", sinrOut: " << sinrOutDb);

  return sinrOutDb;
}

}
