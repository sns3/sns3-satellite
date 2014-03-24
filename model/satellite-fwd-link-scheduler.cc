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

  for ( std::vector< Ptr<SatSchedulingObject> >::const_iterator it = so.begin();
        it != so.end(); it++ )
    {
      std::cout << "So-Content (ptr, priority, load, hol): "
                << (*it) << ", "
                << (*it)->GetPriority() << ", "
                << (*it)->GetBufferedBytes() << ", "
                << (*it)->GetHolDelay() << std::endl;
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
      result = (bool) ( obj1->GetBufferedBytes() > obj2->GetBufferedBytes() );
    }

  return result;
}

bool
SatFwdLinkScheduler::CompareSoPriorityHol (Ptr<SatSchedulingObject> obj1, Ptr<SatSchedulingObject> obj2)
{
  bool result = CompareSoFlowId (obj1, obj2);

  if ( obj1->GetFlowId () == obj2->GetFlowId () )
    {
      result = (bool) ( obj1->GetHolDelay() > obj2->GetHolDelay() );
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
                    PointerValue(),
                    MakePointerAccessor (&SatFwdLinkScheduler::m_bbFrameConf),
                    MakePointerChecker<SatBbFrameConf> ())
    .AddAttribute ("BBFrameUsageMode",
                   "Mode for selecting used BBFrames.",
                    EnumValue (SatFwdLinkScheduler::NORMAL_FRAMES),
                    MakeEnumAccessor (&SatFwdLinkScheduler::m_bbFrameUsageMode),
                    MakeEnumChecker (SatFwdLinkScheduler::SHORT_FRAMES, "Only short frames used.",
                                     SatFwdLinkScheduler::NORMAL_FRAMES, "Only normal frames used",
                                     SatFwdLinkScheduler::SHORT_AND_NORMAL_FRAMES, "Both short and normal frames used."))
    .AddAttribute ("SchedulingStartThresholdTime",
                   "Threshold time of total transmissions in BB Frame container to trigger a scheduling round.",
                    TimeValue (Seconds (0.005)),
                    MakeTimeAccessor (&SatFwdLinkScheduler::m_schedulingStartThresholdTime),
                    MakeTimeChecker ())
    .AddAttribute ("SchedulingStopThresholdTime",
                   "Threshold time of total transmissions in BB Frame container to stop a scheduling round.",
                    TimeValue (Seconds (0.015)),
                    MakeTimeAccessor (&SatFwdLinkScheduler::m_schedulingStopThresholdTime),
                    MakeTimeChecker ())
    .AddAttribute ("AdditionalSortCriteria",
                   "Sorting criteria after priority for scheduling objects from LLC.",
                    EnumValue (SatFwdLinkScheduler::NO_SORT),
                    MakeEnumAccessor (&SatFwdLinkScheduler::m_additionalSortCriteria),
                    MakeEnumChecker (SatFwdLinkScheduler::NO_SORT, "No sorting",
                                     SatFwdLinkScheduler::BUFFERING_DELAY_SORT, "Sorting by delay in buffer",
                                     SatFwdLinkScheduler::BUFFERING_LOAD_SORT, "Sorting by load in buffer"))
    .AddAttribute ("CnoEstimationMode",
                   "Mode of the C/N0 estimator",
                   EnumValue (SatCnoEstimator::LAST),
                   MakeEnumAccessor (&SatFwdLinkScheduler::m_cnoEstimatorMode),
                   MakeEnumChecker (SatCnoEstimator::LAST, "Last value in window used.",
                                    SatCnoEstimator::MINIMUM, "Minimum value in window used.",
                                    SatCnoEstimator::AVERAGE, "Average value in window used."))
    .AddAttribute( "CnoEstimationWindow",
                   "Time window for C/N0 estimation.",
                   TimeValue (MilliSeconds (500)),
                   MakeTimeAccessor (&SatFwdLinkScheduler::m_cnoEstimationWindow),
                   MakeTimeChecker ())


  ;
  return tid;
}

SatFwdLinkScheduler::SatFwdLinkScheduler ()
: m_additionalSortCriteria (SatFwdLinkScheduler::NO_SORT),
  m_bbFrameUsageMode (SatFwdLinkScheduler::NORMAL_FRAMES),
  m_cnoEstimatorMode (SatCnoEstimator::LAST)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor for SatFwdLinkScheduler not supported");
}

SatFwdLinkScheduler::SatFwdLinkScheduler (Ptr<SatBbFrameConf> conf, Mac48Address address)
 : m_macAddress (address),
   m_bbFrameConf (conf),
   m_additionalSortCriteria (SatFwdLinkScheduler::NO_SORT),
   m_bbFrameUsageMode (SatFwdLinkScheduler::NORMAL_FRAMES),
   m_cnoEstimatorMode (SatCnoEstimator::LAST)
{
  NS_LOG_FUNCTION (this);

  // TODO: Currently we assume that there is two priority classes in use
  // control and normal. This is needed to configure according to priorities used by LLC
  m_bbFrameContainer = Create<SatBbFrameContainer> (2);

  // Random variable used in scheduling
  m_random = CreateObject<UniformRandomVariable> ();

  // create dummy frame
  m_dummyFrame = Create<SatBbFrame> (m_bbFrameConf->GetDefaultModCod (), SatEnums::DUMMY_FRAME, m_bbFrameConf);

  Ptr<Packet> dummyPacket = Create<Packet> (1);

  // Add MAC tag
  SatMacTag tag;
  tag.SetDestAddress (Mac48Address::GetBroadcast ());
  tag.SetSourceAddress (m_macAddress);
  dummyPacket->AddPacketTag (tag);

  // Add dummy packet to dummy frame
  m_dummyFrame->AddPayload (dummyPacket);

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
  m_dummyFrame = NULL;
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


Ptr<SatBbFrame>
SatFwdLinkScheduler::GetNextFrame ()
{
  NS_LOG_FUNCTION (this);

  if ( m_bbFrameContainer->GetTotalDuration () < m_schedulingStartThresholdTime )
    {
        ScheduleBbFrames ();
    }

  Ptr<SatBbFrame> frame = m_bbFrameContainer->GetNextFrame ();

  if ( frame == NULL )
    {
      frame = m_dummyFrame;
    }

  return frame;
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

  uint32_t frameBytes = 0;
  Ptr<SatBbFrame> frame = NULL;
  uint8_t flowId = 0;

  // Get scheduling objects from LLC
  std::vector< Ptr<SatSchedulingObject> > so =  GetSchedulingObjects ();

  for ( std::vector< Ptr<SatSchedulingObject> >::const_iterator it = so.begin () ;it != so.end(); it++ )
    {
      uint32_t currentObBytes = (*it)->GetBufferedBytes ();
      double cno = GetSchedulingObjectCno (*it);
      uint32_t currentObMinReqBytes = (*it)->GetMinTxOpportunityInBytes ();
      flowId = (*it)->GetFlowId ();

      while ( (m_bbFrameContainer->GetTotalDuration () < m_schedulingStopThresholdTime ) &&
               (currentObBytes > 0) )
        {
          if ( frame == NULL )
            {
              frame = CreateFrame (cno, currentObBytes);
              frameBytes = frame->GetSpaceLeftInBytes ();
            }
          else if ( CnoMatchWithFrame ( cno, frame ) == false )
            {
              // finish with this frame if MODCOD is more robust than we are currently using
              // TODO: we need to check rest of object left in list too
              AddFrameToContainer (flowId, frame);
              frame = NULL;
            }
          else
            {
              if ( frameBytes < currentObMinReqBytes )
                {
                  AddFrameToContainer (flowId, frame);
                  frame = NULL;
                }
              else
                {
                  uint32_t bytesLeft = 0;

                  frameBytes = AddPacketToFrame (frameBytes, frame, (*it)->GetMacAddress (), (*it)->GetFlowId (), bytesLeft);
                  currentObBytes = bytesLeft;
                }
            }
        }
    }

  if ( frame )
    {
      AddFrameToContainer (flowId, frame);
    }
}

std::vector< Ptr<SatSchedulingObject> >
SatFwdLinkScheduler::GetSchedulingObjects ()
{
  NS_LOG_FUNCTION (this);

  std::vector< Ptr<SatSchedulingObject> > so;

  if ( m_bbFrameContainer->GetTotalDuration () < m_schedulingStopThresholdTime )
    {
      // Get scheduling objects from LLC
      so = m_schedContextCallback ();

      SortSchedulingObjects (so);
    }

  return so;
}

void
SatFwdLinkScheduler::SortSchedulingObjects (std::vector< Ptr<SatSchedulingObject> >& so)
{
  NS_LOG_FUNCTION (this);

  // sort only if there is need to sort
  if ( ( so.empty () == false ) && ( so.size() > 1 ) )
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

Ptr<SatBbFrame>
SatFwdLinkScheduler::CreateFrame (double cno, uint32_t byteCount) const
{
  NS_LOG_FUNCTION (this << cno << byteCount);

  // TODO: If frame is needed to optimize based on total data in scheduling objects
  // possibly it can be done here and also taken into account when sorting objects

  Ptr<SatBbFrame> frame = NULL;

  // set default MODCOD first
  SatEnums::SatModcod_t modCod = m_bbFrameConf->GetDefaultModCod ();

  if ( isnan (cno) == false )
    {
      // use MODCOD based on C/N0 for normal frame first
      modCod = m_bbFrameConf->GetBestModcod (cno, SatEnums::NORMAL_FRAME);
    }

  switch (m_bbFrameUsageMode)
  {
    case SHORT_FRAMES:
      if ( isnan (cno) == false )
        {
          // use MODCOD based on C/N0 for short frame
          modCod = m_bbFrameConf->GetBestModcod (cno, SatEnums::SHORT_FRAME);
        }

      frame = Create<SatBbFrame> (modCod, SatEnums::SHORT_FRAME, m_bbFrameConf);
      break;

    case NORMAL_FRAMES:
      frame = Create<SatBbFrame> (modCod, SatEnums::NORMAL_FRAME, m_bbFrameConf);
      break;

    case SHORT_AND_NORMAL_FRAMES:
      {
        uint32_t bytesInNormalFrame = m_bbFrameConf->GetBbFramePayloadBits (modCod, SatEnums::NORMAL_FRAME) / 8;

        if (byteCount >= bytesInNormalFrame)
          {
            frame = Create<SatBbFrame> (modCod, SatEnums::NORMAL_FRAME, m_bbFrameConf);
          }
        else
          {
            if ( isnan (cno) == false )
              {
                // use MODCOD based on C/N0 for short frame
                modCod = m_bbFrameConf->GetBestModcod (cno, SatEnums::SHORT_FRAME);
              }

            frame = Create<SatBbFrame> (modCod, SatEnums::SHORT_FRAME, m_bbFrameConf);
          }
      }
      break;

    default:
      NS_FATAL_ERROR ("Invalid BBFrame usage mode!!!");
      break;

  }

  return frame;
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

bool
SatFwdLinkScheduler::AddFrameToContainer (uint8_t flowId, Ptr<SatBbFrame> frame )
{
  NS_LOG_FUNCTION (this);

  bool limitReached = false;

  m_bbFrameContainer->AddFrame (flowId, frame);

  if ( m_bbFrameContainer->GetTotalDuration () >= m_schedulingStopThresholdTime )
    {
      limitReached = true;
    }

  return limitReached;
}

uint32_t
SatFwdLinkScheduler::AddPacketToFrame (uint32_t bytesToReq, Ptr<SatBbFrame> frame, Mac48Address address, uint8_t flowId, uint32_t &bytesLeft )
{
  NS_LOG_FUNCTION (this);

  uint32_t frameBytesLeft = 0;

  bytesLeft = 0;
  Ptr<Packet> p = m_txOpportunityCallback (bytesToReq, address, flowId, bytesLeft);

  if ( p )
    {
      frameBytesLeft = frame->AddPayload (p);
    }
  else if ( frame->GetSpaceLeftInBytes () == frame->GetMaxSpaceInBytes () )
    {
      NS_FATAL_ERROR ("Packet does not fit in empty BB Frame. Control package too long or fragmentation problem in user package!!!");
    }
  // TODO: else should we try to fill frame with some other data?

  return frameBytesLeft;
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
