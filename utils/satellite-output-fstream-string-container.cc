/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#include "satellite-output-fstream-string-container.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("SatOutputFileStreamStringContainer");

namespace ns3 {

TypeId
SatOutputFileStreamStringContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatOutputFileStreamStringContainer")
    .SetParent<Object> ()
    .AddConstructor<SatOutputFileStreamStringContainer> ();
  return tid;
}

SatOutputFileStreamStringContainer::SatOutputFileStreamStringContainer (std::string filename, std::ios::openmode filemode)
  : m_outputFileStreamWrapper (),
    m_outputFileStream (),
    m_container (),
    m_fileName (filename),
    m_fileMode (filemode)
{
  NS_LOG_FUNCTION (this << m_fileName << m_fileMode);
}

SatOutputFileStreamStringContainer::SatOutputFileStreamStringContainer ()
  : m_outputFileStreamWrapper (),
    m_outputFileStream (),
    m_container (),
    m_fileName (),
    m_fileMode ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("SatOutputFileStreamStringContainer::SatOutputFileStreamStringContainer - Constructor not in use");
}

SatOutputFileStreamStringContainer::~SatOutputFileStreamStringContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatOutputFileStreamStringContainer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
  Object::DoDispose ();
}

void
SatOutputFileStreamStringContainer::WriteContainerToFile ()
{
  NS_LOG_FUNCTION (this);

  OpenStream ();

  if (m_outputFileStream->is_open ())
    {
      for (uint32_t i = 0; i < m_container.size (); i++)
        {
          *m_outputFileStream <<  m_container[i] << std::endl;
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
SatOutputFileStreamStringContainer::AddToContainer (std::string newLine)
{
  NS_LOG_FUNCTION (this);

  m_container.push_back (newLine);
}

void
SatOutputFileStreamStringContainer::OpenStream ()
{
  NS_LOG_FUNCTION (this);

  m_outputFileStreamWrapper = new SatOutputFileStreamWrapper (m_fileName,m_fileMode);
  m_outputFileStream = m_outputFileStreamWrapper->GetStream ();
}

void
SatOutputFileStreamStringContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  ResetStream ();
  ClearContainer ();
}

void
SatOutputFileStreamStringContainer::ResetStream ()
{
  NS_LOG_FUNCTION (this);

  if (m_outputFileStreamWrapper != NULL)
    {
      delete m_outputFileStreamWrapper;
      m_outputFileStreamWrapper = 0;
    }
  m_outputFileStream = 0;

  m_fileName = "";
  m_fileMode = std::ofstream::out;
}

void
SatOutputFileStreamStringContainer::ClearContainer ()
{
  NS_LOG_FUNCTION (this);

  if (!m_container.empty ())
    {
      m_container.clear ();
    }
}

} // namespace ns3
