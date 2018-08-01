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

#include <ns3/log.h>

#include "satellite-ut-handover-module.h"


NS_LOG_COMPONENT_DEFINE ("SatUtHandoverModule");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatUtHandoverModule);


TypeId
SatUtHandoverModule::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatUtHandoverModule")
    .SetParent<Object> ()
    .AddConstructor<SatUtHandoverModule> ()
  ;
  return tid;
}


TypeId
SatUtHandoverModule::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}


SatUtHandoverModule::SatUtHandoverModule ()
{
  NS_LOG_FUNCTION (this);
}


SatUtHandoverModule::~SatUtHandoverModule ()
{
  NS_LOG_FUNCTION (this);
}

}
