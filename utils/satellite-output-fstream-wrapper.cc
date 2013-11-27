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

#include "satellite-output-fstream-wrapper.h"
#include "ns3/log.h"
#include "ns3/fatal-impl.h"
#include "ns3/abort.h"
#include <fstream>

NS_LOG_COMPONENT_DEFINE ("SatOutputFileStreamWrapper");

namespace ns3 {

SatOutputFileStreamWrapper::SatOutputFileStreamWrapper (std::string filename, std::ios::openmode filemode)
  : m_destroyable (true)
{
  NS_LOG_FUNCTION (this << filename << filemode);

  m_ofstream = new std::ofstream (filename.c_str (), filemode);

  NS_ABORT_MSG_UNLESS (m_ofstream->is_open (), "SatOutputFileStreamWrapper::SatOutputFileStreamWrapper():  " <<
                       "Unable to Open " << filename << " for mode " << filemode);

  FatalImpl::RegisterStream (m_ofstream);
}

SatOutputFileStreamWrapper::~SatOutputFileStreamWrapper ()
{
  NS_LOG_FUNCTION (this);

  FatalImpl::UnregisterStream (m_ofstream);

  if (m_destroyable)
    {
      delete m_ofstream;
      m_ofstream = 0;
    }
  m_destroyable = false;
}

std::ofstream *
SatOutputFileStreamWrapper::GetStream (void)
{
  NS_LOG_FUNCTION (this);

  return m_ofstream;
}

} // namespace ns3
