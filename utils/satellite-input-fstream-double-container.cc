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

#include "satellite-input-fstream-double-container.h"
#include "ns3/log.h"
#include "ns3/abort.h"

NS_LOG_COMPONENT_DEFINE ("SatInputFileStreamDoubleContainer");

namespace ns3 {

TypeId
SatInputFileStreamDoubleContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatInputFileStreamDoubleContainer")
    .SetParent<Object> ()
    .AddConstructor<SatInputFileStreamDoubleContainer> ();
  return tid;
}

SatInputFileStreamDoubleContainer::SatInputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode) :
    m_inputFileStreamWrapper (),
    m_inputFileStream (),
    m_container ()
{
  NS_LOG_FUNCTION (this << filename << filemode);

  UpdateContainer (filename, filemode);
}

SatInputFileStreamDoubleContainer::SatInputFileStreamDoubleContainer () :
    m_inputFileStreamWrapper (),
    m_inputFileStream (),
    m_container ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (0);
}

SatInputFileStreamDoubleContainer::~SatInputFileStreamDoubleContainer ()
{
  NS_LOG_FUNCTION (this);

  Reset ();
}

void
SatInputFileStreamDoubleContainer::UpdateContainer (std::string filename, std::ios::openmode filemode)
{
  NS_LOG_FUNCTION (this);

  if (m_inputFileStream != 0)
    {
      Reset ();
    }

  m_inputFileStreamWrapper = new SatInputFileStreamWrapper (filename,filemode);
  m_inputFileStream = m_inputFileStreamWrapper->GetStream ();

  if (m_inputFileStream->is_open())
  {
    std::string line;
    while ( std::getline (*m_inputFileStream,line) )
    {
      std::cout << line << std::endl;
    }
    m_inputFileStream->close ();
  }
  else
    {
      NS_ABORT_MSG ("Input stream is not valid for reading.");
    }

  Reset ();
}

void
SatInputFileStreamDoubleContainer::Reset ()
{
  NS_LOG_FUNCTION (this);

  delete m_inputFileStreamWrapper;
  m_inputFileStream = 0;
}

} // namespace ns3
