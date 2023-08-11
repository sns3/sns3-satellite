/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Joaquin Muguerza <joaquin.muguerza@viveris.fr>
 *
 */

#ifndef SATELLITE_MUTUAL_INFORMATION_TABLE_H
#define SATELLITE_MUTUAL_INFORMATION_TABLE_H

#include <ns3/object.h>

#include <fstream>
#include <vector>

namespace ns3
{

/**
 * \ingroup satellite
 *
 * \brief Loads a mutual information file and provide query service.
 */
class SatMutualInformationTable : public Object
{
  public:
    /**
     * Constructor with initialization parameters.
     * \param mutualInformationPath
     */
    SatMutualInformationTable(std::string mutualInformationPath);

    /**
     * Destructor for SatMutualInformationTable
     */
    virtual ~SatMutualInformationTable();

    /**
     * \brief Get the type ID
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * \brief Get the Normalized Symbol Information corresponding to a given SNIR
     * \param snirDb SNIR in logarithmic scale
     * \return NormaizedSymbolInformation
     */
    double GetNormalizedSymbolInformation(double snirDb) const;

    /**
     * \brief Get the SNIR in dB for a given Normalized Symbol Information target
     * \param Normalized Symbol Information target (0-1)
     * \return Es/No target in dB
     */
    double GetSnirDb(double symbolInformationTarget) const;

    /**
     * \brief Get the value of beta
     */
    inline double GetBeta() const
    {
        return m_beta;
    }

  private:
    virtual void DoDispose();

    /**
     * \brief Load the mutual information
     * \param mutualInformationPath Path to a mutual information file.
     */
    void Load(std::string mutualInformationPath);

    std::vector<double> m_snirDb;
    std::vector<double> m_symbolInformation;
    std::ifstream* m_ifs;

    /**
     * \brief The adjusting factor beta.
     */
    double m_beta;
};

} // end of namespace ns3

#endif /* SATELLITE_MUTUAL_INFORMATION_TABLE_H */
