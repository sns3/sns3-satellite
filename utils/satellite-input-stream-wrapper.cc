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
 * Modified by: Frans Laakso <frans.laakso@magister.fi>
 */

#include "satellite-input-stream-wrapper.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include <fstream>

NS_LOG_COMPONENT_DEFINE ("SatInputStreamWrapper");

namespace ns3 {

SatInputStreamWrapper::SatInputStreamWrapper (std::string filename, std::ios::openmode filemode)
  : m_destroyable (true)
{
  NS_LOG_FUNCTION (this << filename << filemode);

  std::ifstream* is = new std::ifstream ();
  is->open (filename.c_str (), filemode);
  m_istream = is;

  NS_ABORT_MSG_UNLESS (is->is_open (), "SatOutputStreamWrapper::OutputStreamWrapper():  " <<
                       "Unable to Open " << filename << " for mode " << filemode);
}

SatInputStreamWrapper::SatInputStreamWrapper (std::istream* is)
  : m_istream (is), m_destroyable (false)
{
  NS_LOG_FUNCTION (this << is);

  NS_ABORT_MSG_UNLESS (m_istream->good (), "Input stream is not valid for reading.");
}

SatInputStreamWrapper::~SatInputStreamWrapper ()
{
  NS_LOG_FUNCTION (this);

  if (m_destroyable)
    {
      delete m_istream;
    }

  m_istream = 0;
}

std::istream *
SatInputStreamWrapper::GetStream (void)
{
  NS_LOG_FUNCTION (this);

  return m_istream;
}

} // namespace ns3
