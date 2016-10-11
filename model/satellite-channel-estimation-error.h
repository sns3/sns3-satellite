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


#ifndef SATELLITE_CHANNEL_ESTIMATION_ERROR_H_
#define SATELLITE_CHANNEL_ESTIMATION_ERROR_H_

#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief SatChannelEstimatorError reads from file and stores the channel
 * estimation error mean and standard deviation values for a set of SINR values.
 * Channel estimation error mean and standard deviation is dependent on calculated
 * SINR. A proper error for a given SINR is interpolated between two closest SINR
 * points. The channel estimation error is added to a given measurement by using
 * the AddError method.
 */
class SatChannelEstimationError : public Object
{
public:

  /**
   * Default constructor.
   */
  SatChannelEstimationError ();

  /**
   * Constructor
   * \param filePathName A file containing the gaussian
   * gaussian distribution mean and STD.
   */
  SatChannelEstimationError (std::string filePathName);

  /**
   * Destructor for SatChannelEstimationError
   */
  virtual ~SatChannelEstimationError ();


  /**
   * inherited from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Dispose of this class instance
   */
  virtual void DoDispose ();

  /**
   * \brief Add channel estimation error to SINR.
   * \param sinrInDb Measured SINR in dB
   * \return SINR including channel estimation error in dB
   */
  double AddError (double sinrInDb) const;

private:
  /**
   * \brief Read the distribution mean and STD values from file.
   * \param filePathName File name
   */
  void ReadFile (std::string filePathName);

  /**
   * Last sample index of the containers
   */
  uint32_t m_lastSampleIndex;

  /**
   * Normal random variable used to calculate the
   * channel estimation error.
   */
  Ptr<NormalRandomVariable> m_normalRandomVariable;

  /**
   * SINR values
   */
  std::vector<double> m_sinrsDb;

  /**
   * Mean values
   */
  std::vector<double> m_mueCesDb;

  /**
   * Standard deviation values
   */
  std::vector<double> m_stdCesDb;

};

}

#endif /* SATELLITE_CHANNEL_ESTIMATION_ERROR_H_ */
