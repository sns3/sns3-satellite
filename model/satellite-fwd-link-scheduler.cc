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

#include <algorithm>
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/double.h"

#include "satellite-enums.h"
#include "satellite-mac-tag.h"
#include "satellite-scheduling-object.h"
#include "satellite-fwd-link-scheduler.h"


NS_LOG_COMPONENT_DEFINE ("SatFwdLinkScheduler");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatFwdLinkScheduler);

//#define SAT_FWD_LINK_SCHEDULER_PRINT_SORT_RESULT

#ifdef SAT_FWD_LINK_SCHEDULER_PRINT_SORT_RESULT
static void PrintSoContent (std::string context, std::vector< Ptr<SatSchedulingObject> >& so)
{
  std::cout << context << std::endl;

  for ( std::vector< Ptr<SatSchedulingObject> >::const_iterator it = so.begin ();
        it != so.end (); it++ )
    {
      std::cout << "So-Content (ptr, priority, load, hol): "
                << (*it) << ", "
                << (*it)->GetPriority () << ", "
                << (*it)->GetBufferedBytes () << ", "
                << (*it)->GetHolDelay () << std::endl;
    }

  std::cout << std::endl;
}
#endif

bool
SatFwdLinkScheduler::CompareSoFlowId (Ptr<SatSchedulingObject> obj1, Ptr<SatSchedulingObject> obj2)
{
  return (bool) (obj1->GetFlowId () < obj2->GetFlowId ());
}

bool
SatFwdLinkScheduler::CompareSoPriorityLoad (Ptr<SatSchedulingObject> obj1, Ptr<SatSchedulingObject> obj2)
{
  bool result = CompareSoFlowId (obj1, obj2);

  if ( obj1->GetFlowId () == obj2->GetFlowId () )
    {
      result = (bool) ( obj1->GetBufferedBytes () > obj2->GetBufferedBytes () );
    }

  return result;
}

bool
SatFwdLinkScheduler::CompareSoPriorityHol (Ptr<SatSchedulingObject> obj1, Ptr<SatSchedulingObject> obj2)
{
  bool result = CompareSoFlowId (obj1, obj2);

  if ( obj1->GetFlowId () == obj2->GetFlowId () )
    {
      result = (bool) ( obj1->GetHolDelay () > obj2->GetHolDelay () );
    }

  return result;
}

TypeId
SatFwdLinkScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFwdLinkScheduler")
    .SetParent<Object> ()
    .AddConstructor<SatFwdLinkScheduler> ()
    .AddAttribute ("Interval",
                   "The time for periodic scheduling",
                   TimeValue (MilliSeconds (20)),
                   MakeTimeAccessor (&SatFwdLinkScheduler::m_periodicInterval),
                   MakeTimeChecker ())
    .AddAttribute ("BBFrameConf",
                   "BB Frame configuration for this scheduler.",
                   PointerValue (),
                   MakePointerAccessor (&SatFwdLinkScheduler::m_bbFrameConf),
                   MakePointerChecker<SatBbFrameConf> ())
    .AddAttribute ("DummyFrameSendingEnabled",
                   "Flag to tell, if dummy frames are sent or not.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatFwdLinkScheduler::m_dummyFrameSendingEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("SchedulingStartThresholdTime",
                   "Threshold time of total transmissions in BB Frame container to trigger a scheduling round.",
                   TimeValue (MilliSeconds (5)),
                   MakeTimeAccessor (&SatFwdLinkScheduler::m_schedulingStartThresholdTime),
                   MakeTimeChecker ())
    .AddAttribute ("SchedulingStopThresholdTime",
                   "Threshold time of total transmissions in BB Frame container to stop a scheduling round.",
                   TimeValue (MilliSeconds (15)),
                   MakeTimeAccessor (&SatFwdLinkScheduler::m_schedulingStopThresholdTime),
                   MakeTimeChecker ())
    .AddAttribute ("AdditionalSortCriteria",
                   "Sorting criteria after priority for scheduling objects from LLC.",
                   EnumValue (SatFwdLinkScheduler::NO_SORT),
                   MakeEnumAccessor (&SatFwdLinkScheduler::m_additionalSortCriteria),
                   MakeEnumChecker (SatFwdLinkScheduler::NO_SORT, "NoSorting",
                                    SatFwdLinkScheduler::BUFFERING_DELAY_SORT, "DelaySort",
                                    SatFwdLinkScheduler::BUFFERING_LOAD_SORT, "LoadSort"))
    .AddAttribute ("CnoEstimationMode",
                   "Mode of the C/N0 estimator",
                   EnumValue (SatCnoEstimator::LAST),
                   MakeEnumAccessor (&SatFwdLinkScheduler::m_cnoEstimatorMode),
                   MakeEnumChecker (SatCnoEstimator::LAST, "LastValueInWindow",
                                    SatCnoEstimator::MINIMUM, "MinValueInWindow",
                                    SatCnoEstimator::AVERAGE, "AverageValueInWindow"))
    .AddAttribute ( "CnoEstimationWindow",
                    "Time window for C/N0 estimation.",
                    TimeValue (Seconds (5000)),
                    MakeTimeAccessor (&SatFwdLinkScheduler::m_cnoEstimationWindow),
                    MakeTimeChecker ())
    .AddAttribute ( "BBFrameContainer",
                    "BB frame container of this scheduler.",
                    PointerValue (),
                    MakePointerAccessor (&SatFwdLinkScheduler::m_bbFrameContainer),
                    MakePointerChecker<SatBbFrameContainer> ())
  ;
  return tid;
}

SatFwdLinkScheduler::SatFwdLinkScheduler ()
  : m_additionalSortCriteria (SatFwdLinkScheduler::NO_SORT),
    m_cnoEstimatorMode (SatCnoEstimator::LAST),
    m_carrierBandwidthInHz (0.0)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor for SatFwdLinkScheduler not supported");
}

SatFwdLinkScheduler::SatFwdLinkScheduler (Ptr<SatBbFrameConf> conf, Mac48Address address, double carrierBandwidthInHz)
  : m_macAddress (address),
    m_bbFrameConf (conf),
    m_additionalSortCriteria (SatFwdLinkScheduler::NO_SORT),
    m_cnoEstimatorMode (SatCnoEstimator::LAST),
    m_carrierBandwidthInHz (carrierBandwidthInHz)
{
  NS_LOG_FUNCTION (this);

  //
  std::vector<SatEnums::SatModcod_t> modCods;
  SatEnums::GetAvailableModcodsFwdLink (modCods);

  m_bbFrameContainer = CreateObject<SatBbFrameContainer> (modCods, m_bbFrameConf);

  // Random variable used in scheduling
  m_random = CreateObject<UniformRandomVariable> ();

  Simulator::Schedule (m_periodicInterval, &SatFwdLinkScheduler::PeriodicTimerExpired, this);
}

SatFwdLinkScheduler::~SatFwdLinkScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
SatFwdLinkScheduler::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_schedContextCallback.Nullify ();
  m_txOpportunityCallback.Nullify ();
  m_bbFrameContainer = NULL;
  m_cnoEstimatorContainer.clear ();
}

void
SatFwdLinkScheduler::SetSchedContextCallback (SatFwdLinkScheduler::SchedContextCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_schedContextCallback = cb;
}

void
SatFwdLinkScheduler::SetTxOpportunityCallback (SatFwdLinkScheduler::TxOpportunityCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_txOpportunityCallback = cb;
}


std::pair<Ptr<SatBbFrame>, const Time>
SatFwdLinkScheduler::GetNextFrame ()
{
  NS_LOG_FUNCTION (this);

  if ( m_bbFrameContainer->GetTotalDuration () < m_schedulingStartThresholdTime )
    {
      ScheduleBbFrames ();
    }

  Ptr<SatBbFrame> frame = m_bbFrameContainer->GetNextFrame ();
  Time frameDuration;

  // If no bb frame available and dummy frames enabled
  if (m_dummyFrameSendingEnabled && frame == NULL)
    {
      frame = Create<SatBbFrame> (m_bbFrameConf->GetDefaultModCod (), SatEnums::DUMMY_FRAME, m_bbFrameConf);

      // create dummy packet
      Ptr<Packet> dummyPacket = Create<Packet> (1);

      // Add MAC tag
      SatMacTag tag;
      tag.SetDestAddress (Mac48Address::GetBroadcast ());
      tag.SetSourceAddress (m_macAddress);
      dummyPacket->AddPacketTag (tag);

      // Add dummy packet to dummy frame
      frame->AddPayload (dummyPacket);

      frameDuration = frame->GetDuration ();
    }
  // If no bb frame available and dummy frames disabled
  else if (frame == NULL)
    {
      frameDuration = m_bbFrameConf->GetDummyBbFrameDuration ();
    }
  // If bb frame available
  else
    {
      frameDuration = frame->GetDuration ();
    }

  return std::make_pair(frame, frameDuration);
}

void
SatFwdLinkScheduler::CnoInfoUpdated (Mac48Address utAddress, double cnoEstimate)
{
  NS_LOG_FUNCTION (this << utAddress << cnoEstimate);

  CnoEstimatorMap_t::const_iterator it = m_cnoEstimatorContainer.find (utAddress);

  if ( it == m_cnoEstimatorContainer.end ())
    {
      Ptr<SatCnoEstimator> estimator = CreateCnoEstimator ();

      std::pair<CnoEstimatorMap_t::const_iterator, bool> result = m_cnoEstimatorContainer.insert (std::make_pair (utAddress, estimator));
      it = result.first;

      if ( result.second == false )
        {
          NS_FATAL_ERROR ("Estimator cannot be added to container!!!");
        }
    }

  it->second->AddSample (cnoEstimate);
}

Time
SatFwdLinkScheduler::GetDefaultFrameDuration () const
{
  NS_LOG_FUNCTION (this);

  return m_bbFrameConf->GetBbFrameDuration (m_bbFrameConf->GetDefaultModCod (), SatEnums::NORMAL_FRAME);
}

void
SatFwdLinkScheduler::PeriodicTimerExpired ()
{
  NS_LOG_FUNCTION (this);

  ScheduleBbFrames ();

  Simulator::Schedule (m_periodicInterval, &SatFwdLinkScheduler::PeriodicTimerExpired, this);
}

void
SatFwdLinkScheduler::ScheduleBbFrames ()
{
  NS_LOG_FUNCTION (this);

  // Get scheduling objects from LLC
  std::vector< Ptr<SatSchedulingObject> > so;
  GetSchedulingObjects (so);

  for ( std::vector< Ptr<SatSchedulingObject> >::const_iterator it = so.begin ();
        ( it != so.end () ) && ( m_bbFrameContainer->GetTotalDuration () < m_schedulingStopThresholdTime ); it++ )
    {
      uint32_t currentObBytes = (*it)->GetBufferedBytes ();
      uint32_t currentObMinReqBytes = (*it)->GetMinTxOpportunityInBytes ();
      uint8_t flowId = (*it)->GetFlowId ();
      SatEnums::SatModcod_t modcod = m_bbFrameContainer->GetModcod ( flowId, GetSchedulingObjectCno (*it));

      uint32_t frameBytes = m_bbFrameContainer->GetBytesLeftInTailFrame (flowId, modcod);

      while ( ( (m_bbFrameContainer->GetTotalDuration () < m_schedulingStopThresholdTime ))
              && (currentObBytes > 0) )
        {
          if ( frameBytes < currentObMinReqBytes)
            {
              frameBytes = m_bbFrameContainer->GetMaxFramePayloadInBytes (flowId, modcod);

              // if frame bytes still too small, we must have too long control message, so let's crash
              if ( frameBytes < currentObMinReqBytes )
                {
                  NS_FATAL_ERROR ("Control package too probably too long!!!");
                }
            }

          Ptr<Packet> p = m_txOpportunityCallback (frameBytes, (*it)->GetMacAddress (), flowId, currentObBytes, currentObMinReqBytes);

          if ( p )
            {
              m_bbFrameContainer->AddData (flowId, modcod, p);
              frameBytes = m_bbFrameContainer->GetBytesLeftInTailFrame (flowId, modcod);
            }
          else if ( m_bbFrameContainer->GetMaxFramePayloadInBytes (flowId, modcod ) != m_bbFrameContainer->GetBytesLeftInTailFrame (flowId, modcod))
            {
              frameBytes = m_bbFrameContainer->GetMaxFramePayloadInBytes (flowId, modcod);
            }
          else
            {
              NS_FATAL_ERROR ("Packet does not fit in empty BB Frame. Control package too long or fragmentation problem in user package!!!");
            }
        }

      m_bbFrameContainer->MergeBbFrames (m_carrierBandwidthInHz);
    }
}

void
SatFwdLinkScheduler::GetSchedulingObjects (std::vector< Ptr<SatSchedulingObject> > & output)
{
  NS_LOG_FUNCTION (this);

  if ( m_bbFrameContainer->GetTotalDuration () < m_schedulingStopThresholdTime )
    {
      // Get scheduling objects from LLC
      m_schedContextCallback (output);

      SortSchedulingObjects (output);
    }
}

void
SatFwdLinkScheduler::SortSchedulingObjects (std::vector< Ptr<SatSchedulingObject> >& so)
{
  NS_LOG_FUNCTION (this);

  // sort only if there is need to sort
  if ( ( so.empty () == false ) && ( so.size () > 1 ) )
    {
#ifdef SAT_FWD_LINK_SCHEDULER_PRINT_SORT_RESULT
      PrintSoContent ("Before sort",  so);
#endif

      switch (m_additionalSortCriteria)
        {
        case SatFwdLinkScheduler::NO_SORT:
          std::sort (so.begin (), so.end (), CompareSoFlowId);
          break;

        case SatFwdLinkScheduler::BUFFERING_DELAY_SORT:
          std::sort (so.begin (), so.end (), CompareSoPriorityHol);
          break;

        case SatFwdLinkScheduler::BUFFERING_LOAD_SORT:
          std::sort (so.begin (), so.end (), CompareSoPriorityLoad);
          break;

        default:
          NS_FATAL_ERROR ("Not supported sorting criteria!!!");
          break;
        }

#ifdef SAT_FWD_LINK_SCHEDULER_PRINT_SORT_RESULT
      PrintSoContent ("After sort",  so);
#endif
    }
}

bool
SatFwdLinkScheduler::CnoMatchWithFrame (double cno, Ptr<SatBbFrame> frame) const
{
  NS_LOG_FUNCTION (this << cno << frame);

  bool match = false;

  SatEnums::SatModcod_t modCod = m_bbFrameConf->GetBestModcod (cno, frame->GetFrameType ());

  if ( modCod >= frame->GetModcod () )
    {
      match = true;
    }

  return match;
}

double
SatFwdLinkScheduler::GetSchedulingObjectCno (Ptr<SatSchedulingObject> ob)
{
  NS_LOG_FUNCTION (this << ob);

  double cno = NAN;

  CnoEstimatorMap_t::const_iterator it = m_cnoEstimatorContainer.find (ob->GetMacAddress ());

  if ( it != m_cnoEstimatorContainer.end () )
    {
      cno = it->second->GetCnoEstimation ();
    }

  return cno;
}

Ptr<SatCnoEstimator>
SatFwdLinkScheduler::CreateCnoEstimator ()
{
  NS_LOG_FUNCTION (this);

  Ptr<SatCnoEstimator> estimator = NULL;

  switch (m_cnoEstimatorMode)
    {
    case SatCnoEstimator::LAST:
    case SatCnoEstimator::MINIMUM:
    case SatCnoEstimator::AVERAGE:
      estimator = Create<SatBasicCnoEstimator> (m_cnoEstimatorMode, m_cnoEstimationWindow);
      break;

    default:
      NS_FATAL_ERROR ("Not supported C/N0 estimation mode!!!");
      break;

    }

  return estimator;
}


} // namespace ns3
