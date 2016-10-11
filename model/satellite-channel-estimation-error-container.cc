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

#include <string>
#include <sstream>
#include "ns3/log.h"
#include "satellite-channel-estimation-error-container.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"

NS_LOG_COMPONENT_DEFINE ("SatFwdLinkChannelEstimationErrorContainer");

namespace ns3 {


/**
 * SatChannelEstimationErrorContainer
 */

SatChannelEstimationErrorContainer::SatChannelEstimationErrorContainer ()
{
  NS_LOG_FUNCTION (this);
}

SatChannelEstimationErrorContainer::~SatChannelEstimationErrorContainer ()
{

}

double
SatChannelEstimationErrorContainer::AddError (double sinrIn, uint32_t wfId) const
{
  NS_LOG_FUNCTION (this << sinrIn << wfId);

  return DoAddError (sinrIn, wfId);
}


/**
 * SatSimpleChannelEstimationErrorContainer
 */

SatSimpleChannelEstimationErrorContainer::SatSimpleChannelEstimationErrorContainer ()
{
  NS_LOG_FUNCTION (this);
}

SatSimpleChannelEstimationErrorContainer::~SatSimpleChannelEstimationErrorContainer ()
{

}

double
SatSimpleChannelEstimationErrorContainer::DoAddError (double sinrIn, uint32_t /*wfId*/) const
{
  NS_LOG_FUNCTION (this << sinrIn);

  return sinrIn;
}

/**
 * SatFwdLinkChannelEstimationErrorContainer
 */

SatFwdLinkChannelEstimationErrorContainer::SatFwdLinkChannelEstimationErrorContainer ()
{
  NS_LOG_FUNCTION (this);

  /**
   * Currently only one set of channel estimation error parameters
   * for forward link are created.
   */
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->LocateDataDirectory ();
  std::string filePathName = dataPath + "/sinrmeaserror/ChannelEstimationErrorFwdLink.txt";
  m_channelEstimationError = CreateObject<SatChannelEstimationError> (filePathName);
}

SatFwdLinkChannelEstimationErrorContainer::~SatFwdLinkChannelEstimationErrorContainer ()
{

}

double
SatFwdLinkChannelEstimationErrorContainer::DoAddError (double sinrIn, uint32_t /*wfId*/) const
{
  NS_LOG_FUNCTION (this << sinrIn);

  return m_channelEstimationError->AddError (sinrIn);
}

/**
 * SatFwdLinkChannelEstimationErrorContainer
 */

SatRtnLinkChannelEstimationErrorContainer::SatRtnLinkChannelEstimationErrorContainer (uint32_t minWfId, uint32_t maxWfId)
{
  NS_LOG_FUNCTION (this);

  /**
   * Short burst waveforms 3-12 are using different set of channel estimation error
   * parameters than longer burst waveforms 13-22.
   */
  std::string filePathName;
  std::string dataPath = Singleton<SatEnvVariables>::Get ()->LocateDataDirectory ();
  Ptr<SatChannelEstimationError> ce;

  for (uint32_t i = minWfId; i <= maxWfId; ++i)
    {
      std::ostringstream ss;
      ss << i;
      filePathName = dataPath + "/sinrmeaserror/ChannelEstimationErrorRtnLinkWf_" + ss.str () + ".txt";
      ce = CreateObject<SatChannelEstimationError> (filePathName);
      m_channelEstimationErrors.insert (std::make_pair (i, ce));
    }
}

SatRtnLinkChannelEstimationErrorContainer::~SatRtnLinkChannelEstimationErrorContainer ()
{

}

double
SatRtnLinkChannelEstimationErrorContainer::DoAddError (double sinrIn, uint32_t wf) const
{
  NS_LOG_FUNCTION (this << wf << sinrIn);

  if (m_channelEstimationErrors.find (wf) != m_channelEstimationErrors.end ())
    {
      return m_channelEstimationErrors.at (wf)->AddError (sinrIn);
    }
  else
    {
      NS_FATAL_ERROR ("Non-supported waveform id: " << wf);
    }
  return 0.0;
}

}
