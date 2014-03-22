/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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

#ifndef SAT_FRAME_ALLOCATOR_H
#define SAT_FRAME_ALLOCATOR_H

#include "ns3/simple-ref-count.h"
#include "ns3/address.h"
#include "ns3/satellite-frame-conf.h"
#include "satellite-control-message.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief helper class for Satellite Beam Scheduler.
 *
 * SatFrameAllocator class is used by SatBeamScheduler to maintain information
 * of the pre-allocated symbols per Capacity Category (CC) in frame.
 * It also knows physical constrains of the frames.
 *
 * SatFrameAllocator is created and used by SatBeamScheduler.
 *
 */
class SatFrameAllocator : public Object
{
public:
  class SatFrameAllocReqItem
  {
  public:
    uint32_t  m_craInKbps;
    uint32_t  m_minRbdcInKbps;
    uint32_t  m_rbdcInKbps;
    uint32_t  m_vbdcBytes;
  };

  class SatFrameAllocInfoItem
  {
  public:
    double  m_craSymbols;
    double  m_minRbdcSymbols;
    double  m_rbdcSymbols;
    double  m_vbdcSymbols;

    SatFrameAllocInfoItem ()
     : m_craSymbols (0.0),
       m_minRbdcSymbols (0.0),
       m_rbdcSymbols (0.0),
       m_vbdcSymbols (0.0) {}

    double GetTotalSymbols () { return (m_craSymbols + m_rbdcSymbols + m_vbdcSymbols);}
  };

  typedef std::vector<SatFrameAllocReqItem>   SatFrameAllocReqItemContainer_t;
  typedef std::vector<SatFrameAllocInfoItem>  SatFrameAllocInfoItemContainer_t;

  /**
   * SatFrameAllocReq is used to define frame allocation parameters when
   * calling AllocateToFrame.
   */
  class SatFrameAllocReq
  {
  public:
    Address                           m_address;
    SatFrameAllocReqItemContainer_t   m_reqPerRc;
    /**
     * Construct SatFrameAllocReq
     *
     * \param req Allocation request per RC/CC
     */
    SatFrameAllocReq (SatFrameAllocReqItemContainer_t req) : m_reqPerRc (req) { }
  };

  /**
   * SatFrameAllocInfo is used to hold frame allocation info in symbols.
   *
   * It is used for both requested and actual allocations.
   */
  class SatFrameAllocInfo
  {
  public:
    double  m_craSymbols;
    double  m_minRbdcSymbols;
    double  m_rbdcSymbols;
    double  m_vbdcSymbols;

    SatFrameAllocInfoItemContainer_t  m_allocInfoPerRc;

    SatFrameAllocInfo ()
     : m_craSymbols (0.0),
       m_minRbdcSymbols (0.0),
       m_rbdcSymbols (0.0),
       m_vbdcSymbols (0.0)
    {
    }

    SatFrameAllocInfo (uint8_t countOfRcs)
     : m_craSymbols (0.0),
       m_minRbdcSymbols (0.0),
       m_rbdcSymbols (0.0),
       m_vbdcSymbols (0.0)
    {
      m_allocInfoPerRc = SatFrameAllocInfoItemContainer_t (countOfRcs, SatFrameAllocInfoItem ());
    }

    /**
     * Construct SatFrameAllocReqInSymbols from SatFrameAllocReq.
     *
     * \param req Frame allocation request parameters
     * \param waveForm  Waveform to use in allocation.
     * \param frameDuration Frame duration
     */
    SatFrameAllocInfo (SatFrameAllocReqItemContainer_t &req, Ptr<SatWaveform> waveForm, Time frameDuration)
    : m_craSymbols (0.0),
      m_minRbdcSymbols (0.0),
      m_rbdcSymbols (0.0),
      m_vbdcSymbols (0.0)
    {
      double byteInSymbols = waveForm->GetBurstLengthInSymbols () / (waveForm->GetPayloadInBytes ());
      double bitInSymbols = byteInSymbols / 8.0;

      for (SatFrameAllocReqItemContainer_t::const_iterator it = req.begin (); it != req.end (); it++ )
        {
          SatFrameAllocInfoItem  reqInSymbols;

          reqInSymbols.m_craSymbols  = bitInSymbols * (it->m_craInKbps * 1000.0) * frameDuration.GetSeconds ();
          reqInSymbols.m_minRbdcSymbols = bitInSymbols * it->m_minRbdcInKbps * frameDuration.GetSeconds ();
          reqInSymbols.m_rbdcSymbols = bitInSymbols * it->m_rbdcInKbps * frameDuration.GetSeconds ();
          reqInSymbols.m_vbdcSymbols = byteInSymbols * it->m_vbdcBytes * frameDuration.GetSeconds ();

          m_craSymbols += reqInSymbols.m_craSymbols;
          m_minRbdcSymbols += reqInSymbols.m_minRbdcSymbols;
          m_rbdcSymbols += reqInSymbols.m_rbdcSymbols;
          m_vbdcSymbols += reqInSymbols.m_vbdcSymbols;

          m_allocInfoPerRc.push_back (reqInSymbols);
        }
    }

    SatFrameAllocInfoItem UpdateTotalCounts ()
    {
      m_craSymbols = 0.0;
      m_minRbdcSymbols = 0.0;
      m_rbdcSymbols = 0.0;
      m_vbdcSymbols = 0.0;

      SatFrameAllocInfoItem totalReqs;

      for (SatFrameAllocInfoItemContainer_t::const_iterator it = m_allocInfoPerRc.begin (); it != m_allocInfoPerRc.end (); it++ )
        {
          SatFrameAllocInfoItem  reqInSymbols;

          m_craSymbols += it->m_craSymbols;
          m_minRbdcSymbols += it->m_minRbdcSymbols;
          m_rbdcSymbols += it->m_rbdcSymbols;
          m_vbdcSymbols += it->m_vbdcSymbols;
        }

      totalReqs.m_craSymbols = m_craSymbols;
      totalReqs.m_minRbdcSymbols = m_minRbdcSymbols;
      totalReqs.m_rbdcSymbols = m_rbdcSymbols;
      totalReqs.m_vbdcSymbols = m_vbdcSymbols;

      return totalReqs;
    }

    double GetTotalSymbols ()
    {
      return (m_craSymbols + m_rbdcSymbols + m_vbdcSymbols);
    }
  };

  /**
   * SatFrameAllocResp is used by SatFrameAllocator, when information of a allocation is returned.
   */
  class SatFrameAllocResp
  {
  public:
    uint8_t   m_frameId;
    uint32_t  m_waveformId;

    /**
     * Construct SatFrameAllocResp
     */
    SatFrameAllocResp ()
      : m_frameId (0),
        m_waveformId (0) {}
  };

  /**
   * Construct SatFrameAllocator
   * \param superFrameConf Super frame configuration
   * \param waveformConf Waveform configuration
   * \param maxRcCount Maximum number of the RCs
   */
  SatFrameAllocator (Ptr<SatSuperframeConf> superFrameConf, Ptr<SatWaveformConf> waveformConf, uint8_t maxRcCount);

  /**
   * Destruct SatFrameAllocator
   */
  ~SatFrameAllocator ();

  // derived from object
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Allocate to the frame.
   * \param cno C/N0 estimation to use in allocation
   * \param allocReq  Allocation request parameters for RC/CCs
   * \param allocResp Frame information when allocation is successful
   * \return true when allocation is successful, false otherwise
   */
  bool AllocateToFrame (double cno, SatFrameAllocReq &allocReq, SatFrameAllocResp &allocResp);

  /**
   * Remove allocations from all frames maintained by frame helper.
   */
  void RemoveAllocations ();

  /**
   * Allocate symbols in all frame to UTs allocated a frame.
   * Allocation is done in fairly manner between UTs and RCs.
   */
  void AllocateSymbols ();

  /**
   * Generate time slot for the UT/RC.
   *
   * \param tbtp TBTP message to add generated time slots.
   */
  void GenerateTimeSlots (Ptr<SatTbtpMessage> tbtp);


private:

  /**
   * SatFrameInfo is used by SatFrameAllocator to maintain information of a specific frame.
   */
  class SatFrameInfo
  {
    public:
      typedef enum
      {
        CC_LEVEL_CRA,
        CC_LEVEL_CRA_MIN_RBDC,
        CC_LEVEL_CRA_RBDC,
        CC_LEVEL_CRA_RBDC_VBDC,
      } CcLevel_t;

      /**
       * Construct frame info
       *
       * \param frameConf Frame configuration for the frame info
       * \param waveformConf Waveform configuration
       * \param frameId Id of the frame
       * \param m_configTypeIndex Index of the configuration type (0-2 supported)
       */
      SatFrameInfo (Ptr<SatFrameConf> frameConf, Ptr<SatWaveformConf> waveformConf, uint8_t frameId, uint32_t m_configTypeIndex);

      /**
       * Reset load counters in frame info.
       */
      void ResetCounters ();

      /**
       * Allocate symbols to all UTs with RCs allocated to the frame.
       * \param targetLoad Target load limits upper bound of the symbols in the frame. Valid values in range 0 and 1.
       * \param fcaEnabled FCA (free capacity allocation) enable status
       */
      void AllocateSymbols (double targetLoad, bool fcaEnabled);

      /**
       * Generate time slots for UT/RC.
       *
       * \param tbtp TBTP message to add generated time slots.
       */
      void GenerateTimeSlots (Ptr<SatTbtpMessage> tbtp);

      /**
       * Create time slot according to configuration type.
       *
       * \param carrierId Id of the carrier into create timeslot
       * \param carrierSymbols Symbols left in carrier
       * \param utSymbols Symbols left for the UT
       * \param rcSymbols Symbols left for RC
       * \return Create time slot configuration
       */
      Ptr<SatTimeSlotConf> CreateTimeSlot (uint16_t carrierId, uint32_t& carrierSymbols, uint32_t& utSymbols, uint32_t& rcSymbols);

      /**
       * Get frame load by requested CC
       * \param ccLevel CC of the request
       * \return Load of the requested CC.
       */
      double GetCcLoad (CcLevel_t ccLevel);

      /**
       * Accept UT/RC requests of the frame according to given CC level.
       *
       * \param ccLevel CC level for the acceptance
       */
      void AcceptRequests (CcLevel_t ccLevel);

      /**
       * Allocate symbols to this frame, if criteria are fulfilled
       *
       * \param ccLevel CC level of the request
       * \param address Address (ID) of the UT allocated
       * \param req Requested symbols
       * \return true allocation done, otherwise false
       */
      bool Allocate (CcLevel_t ccLevel, Address address, double cno, SatFrameAllocInfo &req);

      /**
       * Update RC/CC requested according to carrier limit
       * \param req
       */
      void UpdateAllocReq (SatFrameAllocInfo &req);

      /**
       * Update total request counters in frame info.
       */
      SatFrameAllocInfoItem UpdateTotalRequests ();

      /**
       * Get Maximum symbols per carrier.
       *
       * \return Maximum symbols per carrier.
       */
      double GetMaxSymbolsPerCarrier () const { return m_maxSymbolsPerCarrier; }

      /**
       * Get frame duration.
       *
       * \return Frame duration.
       */
      Time GetDuration () const { return m_frameConf->GetDuration (); }

      /**
       * Get frame symbol rate in bauds.
       *
       * \return Frame symbol rate in bauds.
       */
      double GetSymbolRateInBauds () const { return m_frameConf->GetBtuConf ()->GetSymbolRateInBauds ();}

    private:
      typedef struct
      {
        double              m_cno;
        SatFrameAllocInfo   m_request;
        SatFrameAllocInfo   m_allocation;
      } UtAllocItem_t;

      // first = UT address, second = RC index
      typedef std::pair<Address, uint8_t>                     RcAllocItem_t;

      typedef std::map<Address, UtAllocItem_t>                UtAllocContainer_t;
      typedef std::list<RcAllocItem_t>                        RcAllocContainer_t;

      class CcReqCompare
      {
      public:
        typedef enum
        {
          CC_TYPE_MIN_RBDC,
          CC_TYPE_RBDC,
          CC_TYPE_VBDC,
        } CcReqType_t;

        CcReqCompare (const UtAllocContainer_t& utAllocContainer, CcReqCompare::CcReqType_t ccReqType)
          : m_utAllocContainer (utAllocContainer), m_ccReqType (ccReqType) {}

        bool operator() (RcAllocItem_t rcAlloc1, RcAllocItem_t rcAlloc2)
        {
          bool result = rcAlloc1.second < rcAlloc1.second;

          if ( rcAlloc1.second == rcAlloc1.second )
            {
              switch (m_ccReqType)
              {
                case CC_TYPE_MIN_RBDC:
                  result = m_utAllocContainer.at(rcAlloc1.first).m_request.m_minRbdcSymbols < m_utAllocContainer.at(rcAlloc2.first).m_request.m_minRbdcSymbols;
                  break;

                case CC_TYPE_RBDC:
                  result = m_utAllocContainer.at(rcAlloc1.first).m_request.m_rbdcSymbols < m_utAllocContainer.at(rcAlloc2.first).m_request.m_rbdcSymbols;
                  break;

                case CC_TYPE_VBDC:
                  result = m_utAllocContainer.at(rcAlloc1.first).m_request.m_vbdcSymbols < m_utAllocContainer.at(rcAlloc2.first).m_request.m_vbdcSymbols;
                  break;

                default:
                  NS_FATAL_ERROR ("Invalid CC type!!!");
                  break;
              }
            }

          return result;
        }
      private:
        const UtAllocContainer_t& m_utAllocContainer;
        CcReqType_t               m_ccReqType;
      };


      double  m_totalSymbolsInFrame;
      double  m_availableSymbolsInFrame;

      double  m_preAllocatedCraSymbols;
      double  m_preAllocatedMinRdbcSymbols;
      double  m_preAllocatedRdbcSymbols;
      double  m_preAllocatedVdbcSymbols;

      double    m_maxSymbolsPerCarrier;
      uint32_t  m_configTypeIndex;
      uint8_t   m_frameId;
      uint32_t  m_timeSlotSymbols;  // for configuration 0
      bool      m_rcBasedAllocation;

      Ptr<SatFrameConf>   m_frameConf;
      UtAllocContainer_t  m_utAllocs;
      RcAllocContainer_t  m_rcAllocs;

      /**
       * Share symbols between all UTs and RCs allocated to the frame.
       * \param fcaEnabled FCA (free capacity allocation) enable status
       */
      void ShareSymbols (bool fcaEnabled);
  };

  typedef std::map<uint8_t, SatFrameInfo> FrameInfoContainer_t;
  typedef std::map <uint8_t, uint32_t>    SupportedFrameInfo_t;

  FrameInfoContainer_t  m_frameInfos;
  Ptr<SatWaveformConf>  m_waveformConf;
  double                m_targetLoad;
  bool                  m_fcaEnabled;
  uint32_t              m_maxRcCount;

  /**
   *
   * \param ccLevel CC level of the request
   * \param allocReq Requested bytes
   * \param allocResp Frame information when allocation is successful
   * \param frames Information of the possibles frames to allocate.
   * \return
   */
  bool AllocateBasedOnCc (SatFrameInfo::CcLevel_t ccLevel, double cno, SatFrameAllocReq& allocReq, SatFrameAllocResp& allocResp, const SupportedFrameInfo_t &frames);

};

} // namespace ns3

#endif /* SAT_FRAME_ALLOCATOR_H */
