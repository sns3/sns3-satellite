/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#include "satellite-random-access-container.h"
#include <set>

NS_LOG_COMPONENT_DEFINE ("SatRandomAccess");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRandomAccess);

TypeId 
SatRandomAccess::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRandomAccess")
    .SetParent<Object> ();
  return tid;
}

SatRandomAccess::SatRandomAccess () :
  m_randomAccessModel (RA_ANY_AVAILABLE)
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Constructor not in use");
}

SatRandomAccess::SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, RandomAccessModel_t randomAccessModel) :
  m_randomAccessModel (randomAccessModel),
  m_randomAccessConf (randomAccessConf)
{
  NS_LOG_FUNCTION (this);

  if (m_randomAccessConf == NULL)
    {
      NS_FATAL_ERROR ("SatRandomAccess::SatRandomAccess - Configuration object is NULL");
    }
  SetRandomAccessModel (randomAccessModel);
}

SatRandomAccess::~SatRandomAccess ()
{
  NS_LOG_FUNCTION (this);
}

/// TODO: implement this
bool
SatRandomAccess::IsDamaAvailable ()
{
  NS_LOG_FUNCTION (this);

  return false;
}

/// TODO: implement this
bool
SatRandomAccess::IsDataAvailable ()
{
  NS_LOG_FUNCTION (this);

  return true;
}

void
SatRandomAccess::SetRandomAccessModel (RandomAccessModel_t randomAccessModel)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Setting Random Access model to: " << randomAccessModel);

  /// TODO: implement this
  if (randomAccessModel == RA_CRDSA || randomAccessModel == RA_ANY_AVAILABLE)
    {
      if (m_crdsaModel == NULL)
        {
          m_crdsaModel = CreateObject<SatCrdsa> (m_randomAccessConf);
          NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - CRDSA model created");
        }
      m_randomAccessModel = randomAccessModel;
    }

  if (randomAccessModel == RA_SLOTTED_ALOHA || randomAccessModel == RA_ANY_AVAILABLE)
    {
      if (m_slottedAlohaModel == NULL)
        {
          m_slottedAlohaModel = CreateObject<SatSlottedAloha> (m_randomAccessConf);
          NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Slotted ALOHA model created");
        }
      m_randomAccessModel = randomAccessModel;
    }

  NS_LOG_INFO ("SatRandomAccess::SetRandomAccessModel - Random Access model updated");
}

void
SatRandomAccess::DoRandomAccess (bool isFrameStart)
{
  if (isFrameStart)
    {
      if (m_randomAccessModel == RA_CRDSA || m_randomAccessModel == RA_ANY_AVAILABLE)
        {
          DoCrdsa ();
        }
      else if (m_randomAccessModel == RA_SLOTTED_ALOHA)
        {
          DoSlottedAloha ();
        }
    }
  else
    {
      if (m_randomAccessModel == RA_SLOTTED_ALOHA || m_randomAccessModel == RA_ANY_AVAILABLE)
        {
          DoSlottedAloha ();
        }
    }
}

void
SatRandomAccess::DoSlottedAloha ()
{
  NS_LOG_FUNCTION (this);

  double time = 0.0;

  NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - Evaluating Slotted ALOHA, checking for DAMA allocations...");

  // Check if we have known DAMA allocations
  if (!IsDamaAvailable ())
    {
      NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - No DAMA -> Running Slotted ALOHA");

      // Evaluate Slotted ALOHA
      time = m_slottedAlohaModel->DoSlottedAloha ();
    }

  NS_LOG_INFO ("SatRandomAccess::DoSlottedAloha - Minimum time to wait: " << time << " seconds");
}

/// TODO: implement this
void
SatRandomAccess::DoCrdsa ()
{
  NS_LOG_FUNCTION (this);

  std::set<uint32_t> txOpportunities;

  NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Evaluating CRDSA, checking for DAMA allocations...");

  if (!IsDamaAvailable ())
    {
      NS_LOG_INFO ("SatRandomAccess::DoCrdsa - No DAMA, checking buffer status...");

      if (IsDataAvailable ())
        {
          NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Data available -> Running CRDSA");

          // Evaluate CRDSA
          txOpportunities = m_crdsaModel->DoCrdsa ();
        }
    }

  std::set<uint32_t>::iterator iter;

  for (iter = txOpportunities.begin (); iter != txOpportunities.end (); iter++ )
    {
      NS_LOG_INFO ("SatRandomAccess::DoCrdsa - Transmission opportunity at slot: " << (*iter));
    }
}

} // namespace ns3
