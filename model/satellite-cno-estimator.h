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

#ifndef SAT_CNO_ESTIMATOR
#define SAT_CNO_ESTIMATOR

#include <map>

#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"
#include "satellite-cno-estimator.h"

namespace ns3 {


/**
 * \ingroup satellite
 * \brief SatCnoEstimator class defines interface for C/N0 estimators.
 *
 */
class SatCnoEstimator : public SimpleRefCount<SatCnoEstimator>
{
public:
  /**
   * Definition of modes for estimator
   */
  typedef enum
  {
    LAST,   //!< Last value in the given window returned
    MINIMUM, //!< Minimum value in the given window returned
    AVERAGE //!< Average value in the given window returned
  } EstimationMode_t;

  /**
   * Default construct a SatCnoEstimator.
   */
  SatCnoEstimator ();

  /**
   * Destroy a SatCnoEstimator
   *
   * This is the destructor for the SatCnoEstimator.
   */
  virtual ~SatCnoEstimator ();

  /**
   * Add a C/N0 sample to estimator.
   * Calls the method DoAddSample.
   *
   * \param cno C/N0 sample value
   */
  void AddSample (double cno);

  /**
   * Estimate C/N0 value of the samples.
   * Calls the method DoAddSample.
   *
   * \return Estimated value of the C/N0,
   * in case that estimation cannot be done (e.g. no samples) NAN is returned.
   */
  double GetCnoEstimation ();

private:
  /**
   * Add a C/N0 sample to estimator.
   * Method must be implemented by inheriting classes.
   *
   * \param cno C/N0 sample value
   */
  virtual void DoAddSample (double cno) = 0;

  /**
   * Estimate C/N0 value of the samples.
   * Method must be implemented by inheriting classes.
   *
   * \return Estimated value of the C/N0,
   * in case that estimation cannot be done (e.g. no samples) NAN is returned.
   */
  virtual double DoGetCnoEstimation () = 0;
};

/**
 * \ingroup satellite
 * \brief class for module SatCnoEstimator.
 *
 * This SatCnoEstimator class holds information of a satellite DAMA entry.
 * It's is created and used by NCC.
 *
 * It supports three dirrent modes:
 *  - LAST: The last value in the window given when requested.
 *  - MINIMUM: The minimum value in the window given when requested.
 *  - AVERAGE: The average of the samples in window given when requested.
 *
 */
class SatBasicCnoEstimator : public SatCnoEstimator
{
public:
  /**
   * Method to add a sample value to current sum.
   *
   * \param currentSum
   * \param sample
   * \return New sum after addition.
   */
  static inline double AddToSum (double currentSum, const std::pair<Time, double>& sample)
  {
    double newSum = currentSum;

    if ( !std::isnan (sample.second) )
      {
        newSum += sample.second;
      }

    return newSum;
  }

  /**
   * Default construct a SatCnoEstimator.
   */
  SatBasicCnoEstimator ();

  /**
   * Construct a SatCnoEstimator with given estimation mode type.
   */
  SatBasicCnoEstimator (SatCnoEstimator::EstimationMode_t mode, Time window);

  /**
   * Destroy a SatCnoEstimator
   *
   * This is the destructor for the SatCnoEstimator.
   */
  ~SatBasicCnoEstimator ();

private:
  typedef std::map<Time, double> SampleMap_t;

  SampleMap_t       m_samples;
  Time              m_window;
  EstimationMode_t  m_mode;

  /**
   * Add a C/N0 sample to estimator.
   *
   * \param cno C/N0 sample value
   */
  virtual void DoAddSample (double cno);

  /**
   * Estimate C/N0 value of the samples in window.
   *
   * \return Estimated value of the C/N0,
   * in case that estimation cannot be done (e.g. no samples) NAN is returned.
   */
  virtual double DoGetCnoEstimation ();

  /**
   * Clear outdated samples from storage.
   */
  void ClearOutdatedSamples ();
};

} // namespace ns3

#endif /* SAT_CNO_ESTIMATOR */
