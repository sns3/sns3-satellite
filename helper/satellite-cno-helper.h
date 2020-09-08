/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2020 CNES
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef __SATELLITE_CNO_HELPER_H__
#define __SATELLITE_CNO_HELPER_H__

#include <ns3/object.h>
#include <ns3/type-id.h>
#include <ns3/log.h>

#include <ns3/satellite-helper.h>

namespace ns3 {

/**
 * \brief TODO
 */
class SatCnoHelper : public Object
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of object instance
   * \return the TypeId of object instance
   */
  TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Default constructor.
   */
  SatCnoHelper ();

  /**
   * \brief Create a base SatCnoHelper.
   * \param satHelper The satellite Helper.
   */
  SatCnoHelper (Ptr<SatHelper> satHelper);

  /**
   * Destructor for SatCnoHelper.
   */
  virtual ~SatCnoHelper ()
  {
  }

  /**
   * Set m_useTraces attribute.
   */
  void SetUseTraces (bool useTraces);

  /**
   * Apply configuration to all the satellite channels.
   * Needs to be done after node creation.
   */
  void ApplyConfiguration ();

private:

  /**
   * \brief Pointer to the SatHelper objet
   */

  Ptr<SatHelper> m_satHelper;

  /**
   * \brief Use C/N0 input traces instead of power calculation from antenna gain
   */
  bool m_useTraces;
  
};

} // namespace ns3

#endif /* __SATELLITE_CNO_HELPER_H__ */