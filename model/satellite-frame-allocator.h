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
  /**
   * Container to store generated TBTP messages.
   */
  typedef std::vector<Ptr<SatTbtpMessage> > TbtpMsgContainer_t;

  /**
   * Map container to store UT allocation information.
   *
   * key is UT's address and value holds information of the allocated bytes
   * for the UT.
   */
  typedef std::map<Address, std::vector<uint32_t> > UtAllocInfoContainer_t;

  /**
   * Allocation information item for the UT/RC requests [bytes].
   */
  class SatFrameAllocReqItem
  {
  public:
    uint32_t  m_craBytes;
    uint32_t  m_minRbdcBytes;
    uint32_t  m_rbdcBytes;
    uint32_t  m_vbdcBytes;
  };

  /**
   * Container to store SatFrameAllocReqItem items.
   */
  typedef std::vector<SatFrameAllocReqItem>   SatFrameAllocReqItemContainer_t;

  /**
   * Allocation information item for requests and allocations [symbols] used
   * internally by SatFrameAllocator.
   */
  class SatFrameAllocInfoItem
  {
  public:
    double  m_craSymbols;
    double  m_minRbdcSymbols;
    double  m_rbdcSymbols;
    double  m_vbdcSymbols;

    /**
     * Construct SatFrameAllocInfoItem.
     */
    SatFrameAllocInfoItem ()
     : m_craSymbols (0.0),
       m_minRbdcSymbols (0.0),
       m_rbdcSymbols (0.0),
       m_vbdcSymbols (0.0) {}

    /**
     * Get symbols allocated/requested by this item.
     *
     * \return Total symbols allocated/requested.
     */
    double GetTotalSymbols () { return (m_craSymbols + m_rbdcSymbols + m_vbdcSymbols);}
  };

  /**
   * Container to store SatFrameAllocInfoItem items.
   */
  typedef std::vector<SatFrameAllocInfoItem>  SatFrameAllocInfoItemContainer_t;

  /**
   * SatFrameAllocReq is used to define frame allocation parameters when
   * requesting allocation from SatFrameAllocator (calling method AllocateSymbols).
   */
  class SatFrameAllocReq
  {
  public:
    double                            cno;
    Address                           m_address;
    SatFrameAllocReqItemContainer_t   m_reqPerRc;

    /**
     * Construct SatFrameAllocReq
     *
     * \param req Allocation request per RC/CC
     */
    SatFrameAllocReq (SatFrameAllocReqItemContainer_t req) : cno (NAN), m_reqPerRc (req) { }
  };

  /**
   * Container to store SatFrameAllocReq item pointers.
   */
  typedef std::vector<SatFrameAllocReq *> SatFrameAllocContainer_t;

  /**
   * SatFrameAllocInfo is used to hold a frame's allocation info in symbols.
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

    /**
     * Information for the RCs.
     */
    SatFrameAllocInfoItemContainer_t  m_allocInfoPerRc;

    /**
     * Construct empty SatFrameAllocInfo.
     */
    SatFrameAllocInfo ()
     : m_craSymbols (0.0),
       m_minRbdcSymbols (0.0),
       m_rbdcSymbols (0.0),
       m_vbdcSymbols (0.0)
    {
    }

    /**
     * Construct empty SatFrameAllocInfo with given number of RCs.
     */
    SatFrameAllocInfo (uint8_t countOfRcs)
     : m_craSymbols (0.0),
       m_minRbdcSymbols (0.0),
       m_rbdcSymbols (0.0),
       m_vbdcSymbols (0.0)
    {
      m_allocInfoPerRc = SatFrameAllocInfoItemContainer_t (countOfRcs, SatFrameAllocInfoItem ());
    }

    /**
     * Construct SatFrameAllocInfo from SatFrameAllocReqItem items.
     *
     * \param req Reference to container having SatFrameAllocReqItem items.
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

      for (SatFrameAllocReqItemContainer_t::const_iterator it = req.begin (); it != req.end (); it++ )
        {
          SatFrameAllocInfoItem  reqInSymbols;

          reqInSymbols.m_craSymbols  = byteInSymbols * it->m_craBytes;
          reqInSymbols.m_minRbdcSymbols = byteInSymbols * it->m_minRbdcBytes;
          reqInSymbols.m_rbdcSymbols = byteInSymbols * it->m_rbdcBytes;
          reqInSymbols.m_vbdcSymbols = byteInSymbols * it->m_vbdcBytes;

          m_craSymbols += reqInSymbols.m_craSymbols;
          m_minRbdcSymbols += reqInSymbols.m_minRbdcSymbols;
          m_rbdcSymbols += reqInSymbols.m_rbdcSymbols;
          m_vbdcSymbols += reqInSymbols.m_vbdcSymbols;

          m_allocInfoPerRc.push_back (reqInSymbols);
        }
    }

    /**
     * Update total count of SatFrameAllocInfo from RCs.
     *
     * \return SatFrameAllocInfoItem holding information of the total request per category.
     */
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

    /**
     * Get total symbols of the item.
     * \return
     */
    double GetTotalSymbols ()
    {
      return (m_craSymbols + m_rbdcSymbols + m_vbdcSymbols);
    }
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
   * Get super frame duration.
   *
   *  \return Super frame duration.
   **/
  inline Time GetSuperframeDuration () const { return m_superframeConf->GetDuration(); }

  /**
   * Reserve minimum rate from the allocator. This method is called to perform CAC functionality.
   *
   * \param minimumRateBytes Minimum rate based bytes needed to reserve
   */
  void ReserveMinimumRate (uint32_t minimumRateBytes);

  /**
   * Allocate symbols to UTs in all frames.
   * Allocation is done in fairly manner between UTs and RCs.
   */
  void AllocateSymbols (SatFrameAllocContainer_t& allocReqs);

  /**
   * Generate time slots in TBTP(s) for the UT/RC.
   *
   * \param tbtpContainer TBTP message container to add/fill TBTPs.
   * \param maxFrameSizeInBytes Maximum size for a TBTP message.
   */
  void GenerateTimeSlots (TbtpMsgContainer_t& tbtpContainer, uint32_t maxSizeInBytes, UtAllocInfoContainer_t& utAllocContainer);

private:

  /**
   * SatFrameInfo is used by SatFrameAllocator to maintain information of a specific frame.
   */
  class SatFrameInfo
  {
    public:

    /**
     * Enum for CC levels
     */
    typedef enum
      {
        CC_LEVEL_CRA,          //!< CC level CRA
        CC_LEVEL_CRA_MIN_RBDC, //!< CC level CRA + Minimum RBDC
        CC_LEVEL_CRA_RBDC,     //!< CC level CRA + RBDC
        CC_LEVEL_CRA_RBDC_VBDC,//!< CC level CRA + RBDC + VBDC
      } CcLevel_t;

      /**
       * Construct frame info
       *
       * \param frameConf Frame configuration for the frame info
       * \param waveformConf Waveform configuration
       * \param frameId Id of the frame
       * \param m_configType Type of the configuration (0-2 supported)
       */
      SatFrameInfo (Ptr<SatFrameConf> frameConf, Ptr<SatWaveformConf> waveformConf, uint8_t frameId, SatSuperframeConf::ConfigType_t m_configType);

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
       * Generate time slots for UT/RCs .
       *
       * \param tbtpContainer TBTP message container to add/fill TBTPs.
       * \param maxFrameSizeInBytes Maximum size for a TBTP message.
       */
      void GenerateTimeSlots (std::vector<Ptr<SatTbtpMessage> >& tbtpContainer, uint32_t maxSizeInBytes, UtAllocInfoContainer_t& utAllocContainer);

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

      /**
       * Get configuration type.
       *
       *  \return Configuration type of the frame.
       **/
      inline uint32_t GetConfigType () const { return m_configType; }

    private:
      /**
       * Allocation information for a UT.
       */
      typedef struct
      {
        double              m_cno;
        SatFrameAllocInfo   m_request;
        SatFrameAllocInfo   m_allocation;
      } UtAllocItem_t;

      /**
       * Pair used as RC allocation item.
       *
       * first = UT address, second = RC index
       */
      typedef std::pair<Address, uint8_t> RcAllocItem_t;

      /**
       * Map container for UT allocation items.
       */
      typedef std::map<Address, UtAllocItem_t> UtAllocContainer_t;

      /**
       * Container for RC allocation items.
       */
      typedef std::list<RcAllocItem_t> RcAllocContainer_t;

      /**
       *  CcReqCompare class for CC type comparisons.
       */
      class CcReqCompare
      {
      public:
        /**
         * Definition for different comparison types.
         */
        typedef enum
        {
          CC_TYPE_MIN_RBDC,//!< CC_TYPE_MIN_RBDC
          CC_TYPE_RBDC,    //!< CC_TYPE_RBDC
          CC_TYPE_VBDC,    //!< CC_TYPE_VBDC
        } CcReqType_t;

        /**
         * Construct CcReqCompare.
         *
         * \param utAllocContainer Reference to UT allocation container.
         * \param ccReqType Type used for comparisons.
         */
        CcReqCompare (const UtAllocContainer_t& utAllocContainer, CcReqCompare::CcReqType_t ccReqType)
          : m_utAllocContainer (utAllocContainer), m_ccReqType (ccReqType) {}

        /**
         * Comparison operator to compare two RC allocations.
         *
         * \param rcAlloc1
         * \param rcAlloc2
         * \return false if first RC allocation is smaller than second.
         */
        bool operator() (RcAllocItem_t rcAlloc1, RcAllocItem_t rcAlloc2)
        {
          bool result = false;

          switch (m_ccReqType)
          {
            case CC_TYPE_MIN_RBDC:
              result = ( m_utAllocContainer.at(rcAlloc1.first).m_request.m_allocInfoPerRc[rcAlloc1.second].m_minRbdcSymbols <
                         m_utAllocContainer.at(rcAlloc2.first).m_request.m_allocInfoPerRc[rcAlloc2.second].m_minRbdcSymbols );
              break;

            case CC_TYPE_RBDC:
              result = ( m_utAllocContainer.at(rcAlloc1.first).m_request.m_allocInfoPerRc[rcAlloc1.second].m_rbdcSymbols <
                         m_utAllocContainer.at(rcAlloc2.first).m_request.m_allocInfoPerRc[rcAlloc2.second].m_rbdcSymbols );
              break;

            case CC_TYPE_VBDC:
              result = ( m_utAllocContainer.at(rcAlloc1.first).m_request.m_allocInfoPerRc[rcAlloc1.second].m_vbdcSymbols <
                         m_utAllocContainer.at(rcAlloc2.first).m_request.m_allocInfoPerRc[rcAlloc2.second].m_vbdcSymbols );
              break;

            default:
              NS_FATAL_ERROR ("Invalid CC type!!!");
              break;
          }

          return result;
        }

      private:
        /**
         * Reference to UT allocation container
         */
        const UtAllocContainer_t& m_utAllocContainer;

        /**
         * Type used for comparisons.
         */
        CcReqType_t m_ccReqType;
      };

      // total symbols in frame.
      double  m_totalSymbolsInFrame;

      // available (left) symbols in frame.
      double  m_availableSymbolsInFrame;

      // pre-allocated CRA symbols in frame
      double  m_preAllocatedCraSymbols;

      // pre-allocated minimum RBDC symbols in frame
      double  m_preAllocatedMinRdbcSymbols;

      // pre-allocated RBDC symbols in frame
      double  m_preAllocatedRdbcSymbols;

      // pre-allocated minimum VBDC symbols in frame
      double  m_preAllocatedVdbcSymbols;

      // maximum symbols available in frame
      double  m_maxSymbolsPerCarrier;

      // configuration type of the frame
      SatSuperframeConf::ConfigType_t  m_configType;

      // Id of the frame
      uint8_t  m_frameId;

      /**
       * flag indicating if RC based allocation is used. When flag is on then
       * RC is tried to allocate fully. UT total request is used allocation criteria when flag is off.
       */
      bool  m_rcBasedAllocation;

      // Burst lengths in use.
      SatWaveformConf::BurstLengthContainer_t m_burstLenghts;

      // Waveform configuration
      Ptr<SatWaveformConf> m_waveformConf;

      // Frame configuration
      Ptr<SatFrameConf>   m_frameConf;

      // UT allocation container
      UtAllocContainer_t  m_utAllocs;

      // RC allocation container
      RcAllocContainer_t  m_rcAllocs;

      /**
       * Share symbols between all UTs and RCs allocated to the frame.
       *
       * \param fcaEnabled FCA (free capacity allocation) enable status
       */
      void ShareSymbols (bool fcaEnabled);

      /**
       * Get optimal burst length in symbols.
       *
       * \param symbolsToUse Symbols can be used for time slot.
       * \param rcSymbolsLeft Symbols left for RC.
       * \return Optimal burst length for the symbols to allocate.
       */
      uint32_t GetOptimalBurtsLengthInSymbols (int64_t symbolsToUse, int64_t rcSymbolsLeft);

      /**
       * Create time slot according to configuration type.
       *
       * \param carrierId Id of the carrier into create time slot
       * \param utSymbolsToUse Symbols possible to allocated for the UT
       * \param carrierSymbolsToUse Symbols possible to allocate to carrier
       * \param utSymbolsLeft Symbols left for the UT
       * \param rcSymbolsLeft Symbols left for RC
       * \param cno Estimated C/N0 of the UT.
       * \return Create time slot configuration
       */
      Ptr<SatTimeSlotConf> CreateTimeSlot (uint16_t carrierId, int64_t& utSymbolsToUse, int64_t& carrierSymbolsToUse, int64_t& utSymbolsLeft, int64_t& rcSymbolsLeft, double cno);
  };

  /**
   * Container for SatFrameInfo items.
   */
  typedef std::map<uint8_t, SatFrameInfo> FrameInfoContainer_t;

  /**
   * Container for ids of the supported SatFrameInfo.
   */
  typedef std::map<uint8_t, uint32_t> SupportedFrameInfo_t;

  // Frame info container.
  FrameInfoContainer_t    m_frameInfos;

  // waveform configuration
  Ptr<SatWaveformConf>    m_waveformConf;

  // super frame  configuration
  Ptr<SatSuperframeConf>  m_superframeConf;

  // target load for the frame
  double  m_targetLoad;

  // flag telling if FCA (free capacity allocation) is on
  bool  m_fcaEnabled;

  // maximum count for RCs
  uint32_t  m_maxRcCount;

  // bytes in minimum carrier
  uint32_t  m_minCarrierBytes;

  // minimum rate based bytes left can been guaranteed by frame allocator
  uint32_t  m_minimumRateBasedBytesLeft;

  /**
   *  Allocate given request according to type.
   *
   * \param ccLevel CC level of the request
   * \param allocReq Requested bytes
   * \param frames Information of the possibles frames to allocate.
   * \return
   */
  bool AllocateBasedOnCc (SatFrameInfo::CcLevel_t ccLevel, SatFrameAllocReq * allocReq, const SupportedFrameInfo_t &frames);

  /**
   * Allocate a request to a frame.
   *
   * \param allocReq  Allocation request parameters for RC/CCs
   * \return true when allocation is successful, false otherwise
   */
  bool AllocateToFrame (SatFrameAllocReq * allocReq);

  /**
   * Remove allocations from all frames maintained by frame allocator.
   */
  void RemoveAllocations ();
};

} // namespace ns3

#endif /* SAT_FRAME_ALLOCATOR_H */
