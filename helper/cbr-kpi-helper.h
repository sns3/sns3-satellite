/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
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
 *
 */

#ifndef CBR_KPI_HELPER_H
#define CBR_KPI_HELPER_H

#include "kpi-helper.h"
#include "ns3/ptr.h"
#include "ns3/application-container.h"
#include "ns3/cbr-application.h"


namespace ns3 {

/**
 * \ingroup satellite
 *
 * KpiHelper is a base class for gathering application layer statistics from
 * simulations. KpiHelper is capable of tracing the transmitted packets/bytes,
 * received packets/bytes, packet delay and received IP level packets. This
 * information may be used to calculate the application throughput and error rate.
 * KpiHelper is using the packet trace callbacks defined in the applications and
 * currently supports CbrApplication, OnOffApplication and PacketSink application.
 * KpiHelper is not used by itself, but actual used KpiHelper is inherited from it,
 * so that it implements proper trace callback set functions to the actual applications.
 *
 * CbrKpiHelper implements the KpiHelper functionality related to CbrApplication.
 */
class CbrKpiHelper : public KpiHelper
{
public:
  CbrKpiHelper (KpiHelper::KpiMode_t mode);
  virtual ~CbrKpiHelper ();

  /**
   * Add sender applications
   * \param apps ApplicationContainer
   */
  virtual void AddSender (ApplicationContainer apps);

  /**
   * Add sender application
   * \param application
   */
  virtual void AddSender (Ptr<Application> app);

};


} // end of `namespace ns3`


#endif /* CBR_KPI_HELPER_H */
