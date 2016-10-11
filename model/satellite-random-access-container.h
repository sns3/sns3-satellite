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
#ifndef SATELLITE_RANDOM_ACCESS_H
#define SATELLITE_RANDOM_ACCESS_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-random-access-container-conf.h"
#include "ns3/random-variable-stream.h"
#include <set>
#include "satellite-enums.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access algorithms. The purpose of this class is to implement
 * Slotted ALOHA and CRDSA algorithms for randomizing the Tx opportunities. In Slotted
 * ALOHAs case this means randomizing the wait time after which the next slot is selected
 * for transmission. In CRDSAs case this means randomizing the slots within a frame for
 * each unique packet and it's replicas.
 */
class SatRandomAccess : public Object
{
public:
  /**
   * \brief Typedef of callback for known DAMA status
   * \return Is there known DAMA allocations
   */
  typedef Callback<bool> IsDamaAvailableCallback;

  /**
   * \brief Typedef of callback for buffer status
   * \return Is there data left in the buffers
   */
  typedef Callback<bool> AreBuffersEmptyCallback;

  /**
   * \struct RandomAccessTxOpportunities_s
   * \brief Random access Tx opportunities. This struct contains the algorithm results from this module
   */
  typedef struct
  {
    SatEnums::RandomAccessTxOpportunityType_t txOpportunityType;
    uint32_t slottedAlohaTxOpportunity;
    std::map <uint32_t,std::set<uint32_t> > crdsaTxOpportunities;
    uint32_t allocationChannel;
  } RandomAccessTxOpportunities_s;

  /**
   * \brief Constructor
   */
  SatRandomAccess ();

  /**
   * \brief Constructor
   */
  SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, SatEnums::RandomAccessModel_t randomAccessModel);

  /**
   * \brief Destructor
   */
  virtual ~SatRandomAccess ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for setting the used random access model
   * \param randomAccessModel random access model
   */
  void SetRandomAccessModel (SatEnums::RandomAccessModel_t randomAccessModel);

  /**
   * \brief Function for setting the backoff time in milliseconds
   * \param allocationChannel allocation channel
   * \param backoffTimeInMilliSeconds backoff time
   */
  void SetCrdsaBackoffTimeInMilliSeconds (uint32_t allocationChannel,
                                          uint32_t backoffTimeInMilliSeconds);

  /**
   * \brief Function for setting the backoff probability
   * \param allocationChannel allocation channel
   * \param backoffProbability backoff probability
   */
  void SetCrdsaBackoffProbability (uint32_t allocationChannel,
                                   uint16_t backoffProbability);

  /**
   * \brief Function for setting the parameters related to CRDSA randomization
   * \param allocationChannel allocation channel
   * \param minRandomizationValue minimum randomization value
   * \param maxRandomizationValue maximum randomization value
   * \param numOfInstances number of packet instances (packet replicas)
   */
  void SetCrdsaRandomizationParameters (uint32_t allocationChannel,
                                        uint32_t minRandomizationValue,
                                        uint32_t maxRandomizationValue,
                                        uint32_t numOfInstances);

  /**
   * \brief Function for setting the maximum rate limitation parameters
   * \param allocationChannel allocation channel
   * \param maxUniquePayloadPerBlock maximum number of unique payloads per block
   * \param maxConsecutiveBlocksAccessed maximum number of consecutive blocks accessed
   * \param minIdleBlocks minimum number of idle blocks
   */
  void SetCrdsaMaximumDataRateLimitationParameters (uint32_t allocationChannel,
                                                    uint32_t maxUniquePayloadPerBlock,
                                                    uint32_t maxConsecutiveBlocksAccessed,
                                                    uint32_t minIdleBlocks);

  /**
   * \brief Function for setting the Slotted ALOHA control randomization interval
   * \param controlRandomizationIntervalInMilliSeconds Control randomization interval in milliseconds
   */
  void SetSlottedAlohaControlRandomizationIntervalInMilliSeconds (uint32_t controlRandomizationIntervalInMilliSeconds);

  /**
   * \brief Main function of this module. This will be called from outside and it is responsible
   * for selecting the appropriate RA algorithm
   * \param allocationChannel allocation channel
   * \param triggerType RA trigger type
   * \return RA algorithm results (Tx opportunities)
   */
  SatRandomAccess::RandomAccessTxOpportunities_s DoRandomAccess (uint32_t allocationChannel, SatEnums::RandomAccessTriggerType_t triggerType);

  /**
   * \brief Function for setting the IsDamaAvailable callback
   * \param callback callback
   */
  void SetIsDamaAvailableCallback (SatRandomAccess::IsDamaAvailableCallback callback);

  /**
   * \brief Function for setting the AreBuffersEmpty callback
   * \param callback callback
   */
  void SetAreBuffersEmptyCallback (SatRandomAccess::AreBuffersEmptyCallback callback);

  /**
   * \brief Function for adding a Slotted ALOHA allocation channel
   * \param allocationChannel
   */
  void AddSlottedAlohaAllocationChannel (uint32_t allocationChannel);

  /**
   * \brief Function for adding a CRDSA allocation channel
   * \param allocationChannel
   */
  void AddCrdsaAllocationChannel (uint32_t allocationChannel);

  /**
   * \brief Function for getting the CRDSA signaling overhead in bytes
   * \return CRDSA signaling overhead in bytes
   */
  uint32_t GetCrdsaSignalingOverheadInBytes ();

  /**
   * \brief Function for getting the Slotted ALOHA signaling overhead in bytes
   * \return Slotted ALOHA signaling overhead in bytes
   */
  uint32_t GetSlottedAlohaSignalingOverheadInBytes ();

  /**
   * \brief Function for checking whether the backoff time has passed for this allocation channel
   * \param allocationChannel allocation channel
   * \return Has backoff time passed
   */
  bool CrdsaHasBackoffTimePassed (uint32_t allocationChannel) const;

protected:
  /**
   * \brief Function for disposing the module and its variables
   */
  void DoDispose ();

private:
  /**
   * \brief Function for printing out various module variables to console
   */
  void PrintVariables ();

  /**
   * \brief Main function for Slotted ALOHA
   * \return Slotted ALOHA algorithm results (Tx opportunity)
   */
  SatRandomAccess::RandomAccessTxOpportunities_s DoSlottedAloha ();

  /**
    * \brief Function for performing the Slotted ALOHA release time randomization, i.e., the time
    * after which the next available slot is selected for Tx opportunity. Control randomization
    * interval defines the maximum release time.
    * \return randomized time
    */
  uint32_t SlottedAlohaRandomizeReleaseTime ();

  /**
   * \brief Function for checking the sanity of Slotted ALOHA related variables
   */
  void SlottedAlohaDoVariableSanityCheck ();

  /**
   * \brief Main function for CRDSA algorithm
   * \param allocationChannel allocation channel
   * \return CRDSA algorithm results (Tx opportunities)
   */
  SatRandomAccess::RandomAccessTxOpportunities_s DoCrdsa (uint32_t allocationChannel);

  /**
   * \brief Function for evaluating the backoff for this allocation channel
   * \param allocationChannel allocation channel
   * \return Was backoff performed or not
   */
  bool CrdsaDoBackoff (uint32_t allocationChannel);

  /**
   * \brief Function for randomizing the CRDSA Tx opportunities (slots) for each unique packet
   * \param allocationChannel allocation channel
   * \param slots a set of reserved slots
   * \return a set of reserved slots updated with the results of the latest randomization
   */
  std::pair <std::set<uint32_t>, std::set<uint32_t> > CrdsaRandomizeTxOpportunities (uint32_t allocationChannel, std::pair <std::set<uint32_t>, std::set<uint32_t> > slots);

  /**
   * \brief Function for evaluating backoff for each unique CRDSA packet and calling the
   * randomization of Tx opportunities if backoff is not triggered
   * \param allocationChannel allocation channel
   * \return Tx opportunities for all unique packets
   */
  SatRandomAccess::RandomAccessTxOpportunities_s CrdsaPrepareToTransmit (uint32_t allocationChannel);

  /**
   * \brief Function for setting the allocation channel specific backoff timer
   * \param allocationChannel allocation channel
   */
  void CrdsaSetBackoffTimer (uint32_t allocationChannel);

  /**
   * \brief Function for increasing the allocation channel specific count of consecutive used blocks.
   * If the maximum number of consecutive blocks is reached, this function triggers the call of idle
   * period.
   * \param allocationChannel allocation channel
   */
  void CrdsaIncreaseConsecutiveBlocksUsed (uint32_t allocationChannel);

  /**
   * \brief Function for increasing the count of consecutive used blocks for all allocation channels
   */
  void CrdsaIncreaseConsecutiveBlocksUsedForAllAllocationChannels ();

  /**
   * \brief Function for reducing the allocation channel specific number of idle blocks in effect
   * \param allocationChannel allocation channel
   */
  void CrdsaReduceIdleBlocks (uint32_t allocationChannel);

  /**
   * \brief Function for reducing the idle blocks in effect for all allocation channels
   */
  void CrdsaReduceIdleBlocksForAllAllocationChannels ();

  /**
   * \brief Function for resetting the number of consecutive blocks to zero for all allocation channels
   */
  void CrdsaResetConsecutiveBlocksUsedForAllAllocationChannels ();

  /**
   * \brief Function for resetting the number of consecutive blocks to zero for a specific allocation channel
   * \param allocationChannel allocation channel
   */
  void CrdsaResetConsecutiveBlocksUsed (uint32_t allocationChannel);

  /**
   * \brief Function for checking if the allocation channel is free
   * \param allocationChannel allocation channel
   * \return Is the allocation channel free
   */
  bool CrdsaIsAllocationChannelFree (uint32_t allocationChannel);

  /**
   * \brief Function for checking if the allocation channel is CRDSA allocation channel
   * \param allocationChannel allocation channel
   * \return Is CRDSA allocation channel
   */
  bool IsCrdsaAllocationChannel (uint32_t allocationChannel);

  /**
   * \brief Function for checking if the allocation channel is Slotted ALOHA allocation channel
   * \param allocationChannel allocation channel
   * \return Is Slotted ALOHA allocation channel
   */
  bool IsSlottedAlohaAllocationChannel (uint32_t allocationChannel);

  /**
   * \brief Uniform random variable object
   */
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  /**
   * \brief The used random access model
   */
  SatEnums::RandomAccessModel_t m_randomAccessModel;

  /**
   * \brief A pointer to random access configuration
   */
  Ptr<SatRandomAccessConf> m_randomAccessConf;

  /**
   * \brief Number of allocation channels available
   */
  uint32_t m_numOfAllocationChannels;

  /**
   * \brief A flag defining whether the buffers were emptied the last time RA was
   * evaluated, i.e., is the data now new
   */
  bool m_crdsaNewData;

  /**
    * \brief Callback for known DAMA status
   */
  IsDamaAvailableCallback m_isDamaAvailableCb;

  /**
    * \brief Callback for buffer status
   */
  AreBuffersEmptyCallback m_areBuffersEmptyCb;

  /**
   * \brief Defines the allocation channels which are enabled for Slotted ALOHA
   */
  std::set<uint32_t> m_slottedAlohaAllocationChannels;

  /**
   * \brief Defines the allocation channels which are enabled for CRDSA
   */
  std::set<uint32_t> m_crdsaAllocationChannels;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_H */
