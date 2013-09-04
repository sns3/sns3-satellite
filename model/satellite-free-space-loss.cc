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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include <cmath>
#include "ns3/double.h"
#include "ns3/string.h"
#include "satellite-free-space-loss.h"

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatFreeSpaceLoss);

TypeId
SatFreeSpaceLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFreeSpaceLoss")
    .SetParent<Object> ()
    .AddConstructor<SatFreeSpaceLoss> ();
  return tid;
}

SatFreeSpaceLoss::SatFreeSpaceLoss ()
{
}

double
SatFreeSpaceLoss::GetFsldB (Ptr<MobilityModel> a, Ptr<MobilityModel> b, double frequency) const
{
  double fsl;

  fsl = 10.0 * log10( GetFsl(a,b, frequency) );

  return fsl;
}

double
SatFreeSpaceLoss::GetFsl (Ptr<MobilityModel> a, Ptr<MobilityModel> b, double frequency) const
{
  double fsl;
  double distance = a->GetDistanceFrom(b);

  fsl = std::pow( ( (4.0 * M_PI * distance * frequency ) / C ), 2.0 );

  return fsl;
}

int64_t
SatFreeSpaceLoss::DoAssignStreams (int64_t stream)
{
  return 0;
}


} // namespace ns3
