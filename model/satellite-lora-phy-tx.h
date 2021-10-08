/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef SAT_LORA_PHY_TX_H
#define SAT_LORA_PHY_TX_H

#include "ns3/satellite-phy-tx.h"

namespace ns3 {

/**
 * Structure to collect all parameters that are used to compute the duration of
 * a packet (excluding payload length).
 */
struct LoraTxParameters
{
  uint8_t sf = 7;     //!< Spreading Factor
  bool headerDisabled = 0;     //!< Whether to use implicit header mode
  uint8_t codingRate = 1;     //!< Code rate (obtained as 4/(codingRate+4))
  double bandwidthHz = 125000;     //!< Bandwidth in Hz
  uint32_t nPreamble = 8;     //!< Number of preamble symbols
  bool crcEnabled = 1;     //!< Whether Cyclic Redundancy Check is enabled
  bool lowDataRateOptimizationEnabled = 0;     //!< Whether Low Data Rate Optimization is enabled
};

/**
 * \ingroup lorawan
 *
 * Class adding methods linked to Lora, needed to be used in a satellite context
 */
class SatLoraPhyTx : public SatPhyTx
{
public:
  // TypeId
  static TypeId GetTypeId (void);

  /**
   * Constructor and destructor
   */
  SatLoraPhyTx ();
  virtual ~SatLoraPhyTx ();

  /**
   * Type definition for a callback to call when a packet has finished sending.
   *
   * This callback is used by the MAC layer, to determine when to open a receive
   * window.
   */
  typedef Callback<void> TxFinishedCallback;

  /**
   * Set the callback to call after transmission of a packet.
   *
   * This method is typically called by an upper MAC layer that wants to be
   * notified after the transmission of a packet.
   */
  void SetTxFinishedCallback (TxFinishedCallback callback);

  /**
  * Start packet transmission to the channel.
  * \param txParams Transmission parameters for a packet
  */
  void StartTx (Ptr<SatSignalParameters> txParams);

  void EndTx ();

protected:

private:
  /**
   * The callback to perform upon the end of a transmission.
   */
  TxFinishedCallback m_txFinishedCallback;

};

} /* namespace ns3 */

#endif /* SAT_LORA_PHY_TX_H */
