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

#ifndef SAT_BEAM_USER_INFO_H
#define SAT_BEAM_USER_INFO_H

#include <vector>

namespace ns3 {

/**
 * @brief Information of beam users liken UTs and their users.
 * Used e.g. when creating beams.
 *
 */
class SatBeamUserInfo
{
public:
  /**
   * Default constructor for SatBeamUserInfo. Should not be used.
   */
  SatBeamUserInfo ();
  /**
   * Constructor for SatBeamUserInfo.
   *
   * @param utCount UT count of the beam (can be added later by method AddUt)
   * @param userCountPerUt User count of the UTs (can be changed for the specific UT by method SetUtUserCount)
   */
  SatBeamUserInfo (uint32_t utCount, uint32_t userCountPerUt);

  /**
   * Destructor for SatBeamUserInfo.
   */
  virtual ~SatBeamUserInfo () {}

  /**
   * @return number of UTs in beam.
   */
  uint32_t GetUtCount ();

  /**
   * @param utIndex index of the UT. Possible indexes are 0  to value -1 returned by GetUtCount
   *
   * @return number of users in a UT.
   */
  uint32_t GetUtUserCount (uint32_t utIndex);

  /**
   * Sets user count for the UT with given uIndex.
   *
   * @param utIndex index of the UT. Possible indexes are 0 to value -1 returned by GetUtCount.
   * @param userCount Number of users under the UT of with given utIndex. Minimum value is 1.
   *                  If this is not called for the UT then default value (1) is used for the UT.
   *
   */
  void SetUtUserCount (uint32_t utIndex, uint32_t userCount);

  /**
   * Adds new UT to end of the list with given user count for the added UT.
   *
   * @param userCountPerUt Number of users under the added UT. Minimum value is 1.
   */
  void AddUt (uint32_t userCountPerUt);

private:
  // vector to store users per UT
  std::vector<uint32_t> m_userCount;
 };

} // namespace ns3

#endif /* SAT_BEAM_USER_INFO_H */
