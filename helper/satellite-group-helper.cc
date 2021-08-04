/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#include "satellite-group-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGroupHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGroupHelper);

TypeId
SatGroupHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGroupHelper")
    .SetParent<Object> ()
    .AddConstructor<SatGroupHelper> ()
  ;
  return tid;
}

TypeId
SatGroupHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatGroupHelper::SatGroupHelper ()
{
  NS_LOG_FUNCTION (this);
}

void
SatGroupHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_groupsMap.clear ();
}

void
SatGroupHelper::AddUtNodeToGroup (uint32_t groupId, Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << groupId << node);

  if (IsGroupExisting (groupId) == false)
  {
    m_groupsMap[groupId] = std::set<Ptr<Node> > ();
  }
  m_groupsMap[groupId].insert (node);
}

void
SatGroupHelper::AddUtNodesToGroup (uint32_t groupId, NodeContainer nodes)
{
  NS_LOG_FUNCTION (this << groupId);

  for (NodeContainer::Iterator it = nodes.Begin (); it != nodes.End (); it++)
    {
      AddUtNodeToGroup (groupId, *it);
    }
}

NodeContainer
SatGroupHelper::GetUtNodes (uint32_t groupId) const
{
  NS_LOG_FUNCTION (this << groupId);

  if (IsGroupExisting (groupId) == false)
  {
    return NodeContainer ();
  }

  NodeContainer utNodes;
  std::set<Ptr<Node> > nodes = m_groupsMap.at (groupId);

  for (std::set<Ptr<Node> >::const_iterator i = nodes.begin (); i != nodes.end (); i++)
    {
      utNodes.Add (*i);
    }

  return utNodes;
}

uint32_t
SatGroupHelper::GetN ()
{
  NS_LOG_FUNCTION (this);

  return m_groupsMap.size ();
}

bool
SatGroupHelper::IsGroupExisting (uint32_t groupId) const
{
  return m_groupsMap.find(groupId) != m_groupsMap.end();
}

} // namespace ns3
