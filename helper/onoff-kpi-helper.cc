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


#include <algorithm>

#include "ns3/onoff-application.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-flow-classifier.h"

#include "onoff-kpi-helper.h"

NS_LOG_COMPONENT_DEFINE ("OnOffKpiHelper");


namespace ns3 {


OnOffKpiHelper::OnOffKpiHelper (KpiHelper::KpiMode_t mode)
  : KpiHelper (mode)
{
  NS_LOG_FUNCTION (this);
}

OnOffKpiHelper::~OnOffKpiHelper ()
{

}


void
OnOffKpiHelper::AddSender (ApplicationContainer apps)
{
  NS_LOG_FUNCTION (this << apps.GetN ());

  for (ApplicationContainer::Iterator it = apps.Begin ();
       it != apps.End (); it++)
    {
      AddSender (*it);
    }
}

void
OnOffKpiHelper::AddSender (Ptr<Application> app)
{
  NS_LOG_FUNCTION (this << app);

  Ptr<OnOffApplication> onoff = app->GetObject<OnOffApplication> ();

  Ipv4Address address;

  if (m_mode == KpiHelper::KPI_FWD)
    {
      // use the client's IP address as the context
      AddressValue remoteAddress;
      onoff->GetAttribute ("Remote", remoteAddress);
      address = InetSocketAddress::ConvertFrom (remoteAddress.Get ()).GetIpv4 ();
      ConfigureAsServer (app);

      NS_LOG_INFO ("Adding OnOff application as server with remote address: " << address);
    }
  else if (m_mode == KpiHelper::KPI_RTN)
    {
      // use the client's IP address as the context
      address = GetAddress (app->GetNode ());
      ConfigureAsClient (app);

      NS_LOG_INFO ("Adding OnOff application as client with local address: " << address);
    }
  else
    {
      NS_FATAL_ERROR ("Unsupported KpiMode!");
    }

  AddClientCounter (address);
  const std::string context = AddressToString (address);
  onoff->TraceConnect ("Tx", context,
                       MakeCallback (&KpiHelper::TxCallback, this));
}



} // end of `namespace ns3`
