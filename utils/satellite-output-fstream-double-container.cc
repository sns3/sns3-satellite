/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Washington
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */

#include "satellite-output-fstream-double-container.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("SatOutputFileStreamDoubleContainer");

namespace ns3 {

TypeId
SatOutputFileStreamDoubleContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatOutputFileStreamDoubleContainer")
    .SetParent<Object> ()
    .AddConstructor<SatOutputFileStreamDoubleContainer> ();
  return tid;
}

SatOutputFileStreamDoubleContainer::SatOutputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow) :
    m_outputFileStreamWrapper (),
    m_outputFileStream (),
    m_container (),
    m_fileName (filename),
    m_fileMode (filemode),
    m_valuesInRow (valuesInRow)
{
  NS_LOG_FUNCTION (this << m_fileName << m_fileMode);

}

SatOutputFileStreamDoubleContainer::SatOutputFileStreamDoubleContainer () :
    m_outputFileStreamWrapper (),
    m_outputFileStream (),
    m_container (),
    m_fileName (),
    m_fileMode (),
    m_valuesInRow ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (0);
}

SatOutputFileStreamDoubleContainer::~SatOutputFileStreamDoubleContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatOutputFileStreamDoubleContainer::WriteContainerToFile ()
{
  OpenStream ();

  if (m_outputFileStream->is_open ())
  {
      for (uint32_t i = 0; i < m_container.size (); i++)
        {
          for( uint32_t j = 0; j < m_valuesInRow; j++ )
            {
              *m_outputFileStream << m_container[i].at (j);
            }
        }
    m_outputFileStream->close ();
  }
  else
    {
      NS_ABORT_MSG ("Output stream is not valid for writing.");
    }

  Reset ();
}

void
SatOutputFileStreamDoubleContainer::AddToContainer (std::vector<double> newItem)
{
  NS_ASSERT (newItem.size () == m_valuesInRow);

  m_container.push_back (newItem);
}

void
SatOutputFileStreamDoubleContainer::OpenStream ()
{
  m_outputFileStreamWrapper = new SatOutputFileStreamWrapper (m_fileName,m_fileMode);
  m_outputFileStream = m_outputFileStreamWrapper->GetStream ();
}

void
SatOutputFileStreamDoubleContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  ResetStream ();
  ClearContainer ();
}

void
SatOutputFileStreamDoubleContainer::ResetStream ()
{
  NS_LOG_FUNCTION (this);

  delete m_outputFileStreamWrapper;
  m_outputFileStream = 0;

  m_fileName = "";
  m_fileMode = std::ofstream::out;
}

void
SatOutputFileStreamDoubleContainer::ClearContainer ()
{
  NS_LOG_FUNCTION (this);

  for( uint32_t i = 0; i < m_container.size (); i++ )
    {
      m_container[i].clear ();
    }
  m_container.clear ();

  m_valuesInRow = 0;
}

} // namespace ns3
