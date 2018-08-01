/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 */

#ifndef SATELLITE_UT_HANDOVER_MODULE_H
#define SATELLITE_UT_HANDOVER_MODULE_H

#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/callback.h>

namespace ns3 {

/**
 * \ingroup satellite
 * \brief UT handover module
 */
class SatUtHandoverModule : public Object
{
public:
  /**
   * Derived from Object
   */
  static TypeId GetTypeId (void);

  /**
   * Derived from Object
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor, which is not used.
   */
  SatUtHandoverModule ();

  /**
   * Construct a SatUtHandoverModule
   * \param
   */
  // SatUtHandoverModule ();

  /**
   * Destroy a SatUtHandoverModule
   */
  ~SatUtHandoverModule ();
};

}

#endif /* SATELLITE_UT_HANDOVER_MODULE_H */
