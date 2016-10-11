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


#ifndef SATELLITE_CHANNEL_ESTIMATION_ERROR_CONTAINER_H_
#define SATELLITE_CHANNEL_ESTIMATION_ERROR_CONTAINER_H_

#include <map>
#include "ns3/object.h"
#include "satellite-channel-estimation-error.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatChannelEstimationErrorContainer is responsible of adding a channel
 * estimation error on top of raw measured SINR. Channel estimation error
 * functions in logarithmic (dB) domain. Abstract base class SatChannelEstimationErrorContainer
 * defines the interface method, but the actual implementation is in inherited
 * classes:
 * - SatSimpleChannelEstimatinoErrorContainer - returns always SINR without errors
 * - SatFwdLinkChannelEstimationErrorContainer - uses FWD channel estimation error tables
 * - SatRtnLinkChannelEstimationErrorContainer - uses RTN channel estimation error tables
 */

class SatChannelEstimationErrorContainer : public SimpleRefCount<SatChannelEstimationErrorContainer>
{
public:

  /**
   * Default constructor.
   */
  SatChannelEstimationErrorContainer ();

  /**
   * Destructor for SatChannelEstimationErrorContainer
   */
  virtual ~SatChannelEstimationErrorContainer ();

  /**
   * \brief Add channel estimation error to SINR. Base class does not actually
   * do anything, but returns only the sinrInDb as sinrOutDb.
   * \param sinrInDb Measured SINR in dB
   * \param wfId Waveform id
   * \return SINR including channel estimation error in dB
   */
  double AddError (double sinrInDb, uint32_t wfId = 0) const;

protected:
  /**
   * \brief Pure virtual method for the implementation in derived classes.
   * \param sinrInDb Measured SINR in dB
   * \param wfId Waveform id
   * \return SINR including channel estimation error in dB
   */
  virtual double DoAddError (double sinrInDb, uint32_t wfId) const = 0;

private:
};


class SatSimpleChannelEstimationErrorContainer : public SatChannelEstimationErrorContainer
{
public:

  /**
   * Default constructor.
   */
  SatSimpleChannelEstimationErrorContainer ();

  /**
   * Destructor for SatSimpleChannelEstimationErrorContainer
   */
  virtual ~SatSimpleChannelEstimationErrorContainer ();

protected:
  /**
   * \brief Simple channel estimation error does nothing to the SINR
   * \param sinrInDb Measured SINR in dB
   * \param wfId Waveform id
   * \return SINR including channel estimation error in dB
   */
  virtual double DoAddError (double sinrInDb, uint32_t wfId) const;

private:
};

class SatFwdLinkChannelEstimationErrorContainer : public SatChannelEstimationErrorContainer
{
public:

  /**
   * Default constructor.
   */
  SatFwdLinkChannelEstimationErrorContainer ();

  /**
   * Destructor for SatFwdLinkChannelEstimationErrorContainer
   */
  virtual ~SatFwdLinkChannelEstimationErrorContainer ();

protected:
  /**
   * \brief Add channel estimation error to SINR in FWD link
   * Waveform id does not have any significance here.
   * \param sinrInDb Measured SINR in dB
   * \param wfId Waveform id
   * \return SINR including channel estimation error in dB
   */
  virtual double DoAddError (double sinrInDb, uint32_t wfId) const;

private:
  /**
   * Only one channel estimator error configuration for
   * forward link.
   */
  Ptr<SatChannelEstimationError> m_channelEstimationError;

};

class SatRtnLinkChannelEstimationErrorContainer : public SatChannelEstimationErrorContainer
{
public:

  /**
   * Constructor with initialization parameters.
   * \param minWfId 
   * \param maxWfId 
   */
  SatRtnLinkChannelEstimationErrorContainer (uint32_t minWfId, uint32_t maxWfId);

  /**
   * Destructor for SatRtnLinkChannelEstimationErrorContainer
   */
  virtual ~SatRtnLinkChannelEstimationErrorContainer ();

protected:
  /**
   * \brief Add channel estimation error to SINR in RTN link.
   * \param sinrInDb Measured SINR in dB
   * \param wfId Waveform id
   * \return SINR including channel estimation error in dB
   */
  virtual double DoAddError (double sinrInDb, uint32_t wfId) const;

private:
  /**
   * Map of channel estimation errors for each waveform id
   */
  std::map<uint32_t, Ptr<SatChannelEstimationError> > m_channelEstimationErrors;
};

}



#endif /* SATELLITE_CHANNEL_ESTIMATION_ERROR_CONTAINER_H_ */
