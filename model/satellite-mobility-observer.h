/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#ifndef SATELLITE_MOBILITY_OBSERVER_H
#define SATELLITE_MOBILITY_OBSERVER_H

#include "ns3/object.h"
#include "satellite-mobility-model.h"
#include "satellite-propagation-delay-model.h"
#include "geo-coordinate.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief Observes given mobilities and keeps track of properties wanted to observe related to mobilities.
 * Properties can be observed are currently the timing advance and elevation angle. (elevation angel always available).
 *
 * Observing of timing advance is set by @method ObserveTimingAdvance
 *
 * This is a base class for all satellite mobility models.
 */
class SatMobilityObserver : public Object
{
public:
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor of the mobility observer (should not be called)
   */
  SatMobilityObserver ();

  /**
   * Constructor for mobility observer
   *
   * @param ownMobility Own mobility to observe
   * @param geoSatMobility Satellite mobility to observe
   */
  SatMobilityObserver (Ptr<SatMobilityModel> ownMobility, Ptr<SatMobilityModel> geoSatMobility);

  /**
   * Destructor of the mobility observer
   */
  virtual ~SatMobilityObserver ();

  /**
   * Enable observing of the timing advance.
   *
   * \param ownDelayModel Own propagation delay model.
   * \param anotherDelayModel Propagation delay model of the another end.
   * \param anotherMobility Mobility of the another end.
   */
  void ObserveTimingAdvance ( Ptr<PropagationDelayModel> ownDelayModel,
                              Ptr<PropagationDelayModel> anotherDelayModel,
                              Ptr<SatMobilityModel> anotherMobility);

  /**
   * Get elevation angle.
   *
   * @return the current elevation angle as degrees.
   */
  double GetElevationAngle (void);

  /**
   * Get timing advance.
   *
   * @return the current timing advance.
   */
  Time GetTimingAdvance (void);

private:

  /**
   * Notify listeners about some property is changed
   */
  void NotifyPropertyChange (void) const;

  /**
   * Update elevation angle.
   */
  void UpdateElevationAngle ();

  /**
   * Update timing advance.
   */
  void UpdateTimingAdvance ();

  /**
   * Do actions needed when satellite position is changed.
   */
  void SatelliteStatusChanged();

  /**
   * Listener (callback) for mobility position changes
   *
   * @param context Context of the mobility whose position is changed
   * @param position Mobility whose position is changed
   */
  void PositionChanged (std::string context, Ptr<const SatMobilityModel> position);

  /**
   * Used to alert subscribers that a change in some observed property has occurred.
   */
  TracedCallback<Ptr<const SatMobilityObserver> > m_propertyChangeTrace;

  Ptr<SatMobilityModel> m_ownMobility;
  Ptr<SatMobilityModel> m_anotherMobility;
  Ptr<SatMobilityModel> m_geoSatMobility;
  Ptr<PropagationDelayModel> m_ownProgDelayModel;
  Ptr<PropagationDelayModel> m_anotherProgDelayModel;

  bool m_initialized;  // flag for GetElevationAngle
  bool m_updateElevationAngle;  // flag for GetElevationAngle
  bool m_updateTimingAdvance;   // flag for GetTimingAdvance
  double m_minAltitude;
  double m_maxAltitude;
  double m_elevationAngle;
  double m_earthRadius;
  double m_radiusRatio;
  double m_maxDistanceToSatellite;
  Time m_timingAdvance_s;
};

} // namespace ns3

#endif /* SATELLITE_MOBILITY_OBSERVER_H */
