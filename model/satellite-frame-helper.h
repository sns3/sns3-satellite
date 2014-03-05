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

#ifndef SAT_FRAME_HELPER_H
#define SAT_FRAME_HELPER_H

#include "ns3/simple-ref-count.h"
#include "ns3/satellite-frame-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief helper class for Satellite Beam Scheduler.
 *
 * SatFrameHelper class is used by SatBeamScheduler to maintain information
 * of the pre-allocated symbols per Capacity Category (CC) in frame.
 * It also knows physical constrains of the frames.
 *
 * SatFrameHelper is created and used by SatBeamScheduler.
 *
 */
class SatFrameHelper : public SimpleRefCount<SatFrameHelper>
{
public:
  /**
   * SatFrameAllocReq is used to define frame allocation parameters when
   * calling AllocateSymbols.
   */
  class SatFrameAllocReq
  {
  public:
    uint32_t  m_craBytes;
    uint32_t  m_minRbdcBytes;
    uint32_t  m_rbdcBytes;
    uint32_t  m_vbdcBytes;

    /**
     * Construct SatFrameAllocReq
     *
     * \param cra CRA request in bytes
     * \param minRbdc Minimum RBDC request in bytes
     * \param rbdc RBDC request in bytes
     * \param vbdc VBDC request in bytes
     */
    SatFrameAllocReq (uint32_t cra, uint32_t minRbdc, uint32_t rbdc, uint32_t vbdc)
      : m_craBytes (cra),
        m_minRbdcBytes (minRbdc),
        m_rbdcBytes (rbdc),
        m_vbdcBytes (vbdc) {}
  };

  /**
   * SatFrameAllocReqInSymbols is used to define frame allocation request in symbols.
   */
  class SatFrameAllocReqInSymbols
  {
  public:
    double  m_craSymbols;
    double  m_minRbdcSymbols;
    double  m_rbdcSymbols;
    double  m_vbdcSymbols;

    /**
     * Construct SatFrameAllocReqInSymbols
     *
     * \param req Frame allocation request parameters
     * \param waveForm  Waveform to use in allocation.
     */
    SatFrameAllocReqInSymbols (SatFrameAllocReq &req, Ptr<SatWaveform> waveForm)
    {
      double symbolsPerByte = waveForm->GetBurstLengthInSymbols() / waveForm->GetPayloadInBytes();

      m_craSymbols = symbolsPerByte * req.m_craBytes;
      m_minRbdcSymbols = symbolsPerByte * req.m_minRbdcBytes;
      m_rbdcSymbols = symbolsPerByte * req.m_rbdcBytes;
      m_vbdcSymbols = symbolsPerByte * req.m_vbdcBytes;
    }

  };

  /**
   * SatFrameAllocInfo is used by SatFrameHelper, when information of a allocation is returned.
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
   * Construct SatFrameHelper
   * \param superFrameConf Super frame configuration
   * \param waveformConf Waveform configuration
   */
  SatFrameHelper (Ptr<SatSuperframeConf> superFrameConf, Ptr<SatWaveformConf> waveformConf);

  /**
   * Destruct SatFrameHelper
   */
  ~SatFrameHelper ();

  /**
   * Allocate to the frame.
   * @param cno C/N0 estimation to use in allocation
   * @param allocReq  Allocation request parameters for CCs
   * @param allocResp Frame information when allocation is successful
   * @return true when allocation is successful, false otherwise
   */
  bool AllocateToFrame (double cno, SatFrameAllocReq &allocReq, SatFrameAllocResp &allocResp);

private:

  /**
   * SatFrameInfo is used by SatFrameHelper to maintain information of a specific frame.
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

      double  m_totalAvailableSymbols;

      double  m_preAllocatedCraSymbols;
      double  m_preAllocatedMinRdbcSymbols;
      double  m_preAllocatedRdbcSymbols;
      double  m_preAllocatedVdbcSymbols;

      double  m_maxSymbolsPerCarrier;
      Ptr<SatFrameConf> m_frameConf;

      /**
       * Construct frame info
       *
       * \param frameConf Frame configuration for the frame info
       */
      SatFrameInfo (Ptr<SatFrameConf> frameConf);

      /**
       * Reset load counters in frame info.
       */
      void ResetCounters ();

      /**
       * Get frame load by requested CC
       * \param ccLevel CC of the request
       * \return Load of the requested CC.
       */
      double GetCcLoad (CcLevel_t ccLevel);

      /**
       * Allocate symbols to this frame, if criteria are fulfilled
       *
       * \param ccLevel CC level of the request
       * \param req Requested symbols
       * \return true allocation done, otherwise false
       */
      bool Allocate (CcLevel_t ccLevel, SatFrameAllocReqInSymbols &req);
  };

  typedef std::map<uint8_t, SatFrameInfo> FrameInfoContainer_t;
  typedef std::map <uint8_t, uint32_t>    SupportedFrameInfo_t;

  FrameInfoContainer_t  m_frameInfos;
  Ptr<SatWaveformConf>  m_waveformConf;

  /**
   * Reset load counters.
   */
  void ResetCounters ();

  /**
   *
   * \param ccLevel CC level of the request
   * \param allocReq Requested bytes
   * \param allocResp Frame information when allocation is successful
   * \param frames Information of the possibles frames to allocate.
   * \return
   */
  bool AllocateBasedOnCc (SatFrameInfo::CcLevel_t ccLevel, SatFrameAllocReq& allocReq, SatFrameAllocResp& allocResp, const SupportedFrameInfo_t &frames);

};

} // namespace ns3

#endif /* SAT_FRAME_HELPER_H */
