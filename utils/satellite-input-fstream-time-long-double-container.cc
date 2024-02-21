/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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

#include "satellite-input-fstream-time-long-double-container.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("SatInputFileStreamTimeLongDoubleContainer");

namespace ns3
{

TypeId
SatInputFileStreamTimeLongDoubleContainer::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatInputFileStreamTimeLongDoubleContainer")
                            .SetParent<Object>()
                            .AddConstructor<SatInputFileStreamTimeLongDoubleContainer>();
    return tid;
}

SatInputFileStreamTimeLongDoubleContainer::SatInputFileStreamTimeLongDoubleContainer(
    std::string filename,
    std::ios::openmode filemode,
    uint32_t valuesInRow)
    : m_inputFileStreamWrapper(),
      m_inputFileStream(),
      m_container(),
      m_fileName(filename),
      m_fileMode(filemode),
      m_valuesInRow(valuesInRow),
      m_lastValidPosition(0),
      m_numOfPasses(0),
      m_timeShiftValue(0),
      m_timeColumn(0)
{
    NS_LOG_FUNCTION(this << m_fileName << m_fileMode);

    UpdateContainer(m_fileName, m_fileMode, m_valuesInRow);
}

SatInputFileStreamTimeLongDoubleContainer::SatInputFileStreamTimeLongDoubleContainer()
    : m_inputFileStreamWrapper(),
      m_inputFileStream(),
      m_container(),
      m_fileName(),
      m_fileMode(),
      m_valuesInRow(),
      m_lastValidPosition(),
      m_numOfPasses(),
      m_timeShiftValue(),
      m_timeColumn()
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("SatInputFileStreamTimeLongDoubleContainer::"
                   "SatInputFileStreamTimeLongDoubleContainer - Constructor not in use");
}

SatInputFileStreamTimeLongDoubleContainer::~SatInputFileStreamTimeLongDoubleContainer()
{
    NS_LOG_FUNCTION(this);

    Reset();
}

void
SatInputFileStreamTimeLongDoubleContainer::DoDispose()
{
    NS_LOG_FUNCTION(this);

    Reset();
    Object::DoDispose();
}

void
SatInputFileStreamTimeLongDoubleContainer::UpdateContainer(std::string filename,
                                                           std::ios::openmode filemode,
                                                           uint32_t valuesInRow)
{
    NS_LOG_FUNCTION(this);

    ClearContainer();

    m_fileName = filename;
    m_fileMode = filemode;
    m_valuesInRow = valuesInRow;

    m_inputFileStreamWrapper = new SatInputFileStreamWrapper(filename, filemode);
    m_inputFileStream = m_inputFileStreamWrapper->GetStream();

    if (m_inputFileStream->is_open())
    {
        std::vector<long double> tempVector = ReadRow();

        while (!m_inputFileStream->eof())
        {
            m_container.push_back(tempVector);
            tempVector = ReadRow();
        }
        m_inputFileStream->close();
    }
    else
    {
        NS_ABORT_MSG("Input stream is not valid for reading.");
    }

    CheckContainerSanity();

    ResetStream();
}

std::vector<long double>
SatInputFileStreamTimeLongDoubleContainer::ReadRow()
{
    NS_LOG_FUNCTION(this);

    long double tempValue;
    std::vector<long double> tempVector;

    for (uint32_t i = 0; i < m_valuesInRow; i++)
    {
        *m_inputFileStream >> tempValue;
        tempVector.push_back(tempValue);
    }
    return tempVector;
}

void
SatInputFileStreamTimeLongDoubleContainer::CheckContainerSanity()
{
    NS_LOG_FUNCTION(this);

    /// check time sample sanity
    if (m_container.size() < 1)
    {
        NS_FATAL_ERROR("SatInputFileStreamTimeLongDoubleContainer::UpdateContainer - Empty file");
    }
    else if (m_container.size() == 1)
    {
        if (m_container[m_container.size() - 1].at(m_timeColumn) == 0)
        {
            NS_FATAL_ERROR("SatInputFileStreamTimeLongDoubleContainer::UpdateContainer - Invalid "
                           "input file format (time sample error)");
        }
    }
    else
    {
        double tempValue1 = m_container[0].at(m_timeColumn);

        for (uint32_t i = 1; i < m_container.size(); i++)
        {
            if (tempValue1 > m_container[i].at(m_timeColumn))
            {
                NS_FATAL_ERROR("SatInputFileStreamTimeLongDoubleContainer::UpdateContainer - "
                               "Invalid input file format (time sample error)");
            }
            tempValue1 = m_container[i].at(m_timeColumn);
        }
    }
}

std::vector<long double>
SatInputFileStreamTimeLongDoubleContainer::ProceedToNextClosestTimeSample()
{
    NS_LOG_FUNCTION(this);

    while (!FindNextClosest(m_lastValidPosition, m_timeShiftValue, Now().GetSeconds()))
    {
        m_lastValidPosition = 0;
        m_numOfPasses++;
        m_timeShiftValue = m_numOfPasses * m_container[m_container.size() - 1].at(m_timeColumn);

        NS_LOG_INFO("Looping samples again with shift value: " << m_timeShiftValue);
    }

    if (m_numOfPasses > 0)
    {
        std::cout
            << "WARNING! - "
               "SatInputFileStreamTimeLongDoubleContainer::ProceedToNextClosestTimeSample for "
            << m_fileName << " is out of samples @ time sample " << Now().GetSeconds()
            << " (passes " << m_numOfPasses << ")" << std::endl;
        std::cout << "The container will loop samples from the beginning." << std::endl;
    }

    return m_container[m_lastValidPosition];
}

std::vector<long double>
SatInputFileStreamTimeLongDoubleContainer::InterpolateBetweenClosestTimeSamples()
{
    NS_LOG_FUNCTION(this);

    long double currentTime = Now().GetSeconds();
    FindNextClosest(m_lastValidPosition, m_timeShiftValue, currentTime);

    std::vector<long double> selectedPosition = m_container[m_lastValidPosition];
    long double selectedTime = selectedPosition.at(m_timeColumn);

    // Easy case: a time sample for the current time exist
    if (selectedTime == currentTime)
    {
        return selectedPosition;
    }

    // Fetch the second position to perform linear interpolation
    std::vector<long double> closestPosition;
    if (selectedTime > currentTime)
    {
        if (m_lastValidPosition == 0)
        {
            // No previous position available, abort
            return selectedPosition;
        }
        closestPosition = m_container[m_lastValidPosition - 1];
    }
    else
    {
        if (m_lastValidPosition == m_container.size() - 1)
        {
            // No next position available, abort
            return selectedPosition;
        }
        closestPosition = m_container[m_lastValidPosition + 1];
    }

    long double linearCoefficient =
        (currentTime - selectedTime) / (closestPosition.at(m_timeColumn) - selectedTime);
    std::size_t rowSize = selectedPosition.size();
    std::vector<long double> interpolatedPosition(rowSize);
    for (std::size_t i = 0; i < rowSize; ++i)
    {
        interpolatedPosition[i] =
            selectedPosition[i] + linearCoefficient * (closestPosition[i] - selectedPosition[i]);
    }

    return interpolatedPosition;
}

bool
SatInputFileStreamTimeLongDoubleContainer::FindNextClosest(uint32_t lastValidPosition,
                                                           long double timeShiftValue,
                                                           long double comparisonTimeValue)
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT(m_timeColumn < m_valuesInRow);
    NS_ASSERT(m_container.size() > 0);
    NS_ASSERT(lastValidPosition >= 0 && lastValidPosition < m_container.size());

    NS_LOG_INFO("LastValidPosition " << lastValidPosition << " column " << m_timeColumn
                                     << " timeShiftValue " << timeShiftValue
                                     << " comparisonTimeValue " << comparisonTimeValue);

    bool valueFound = false;

    for (uint32_t i = lastValidPosition; i < m_container.size(); i++)
    {
        if (m_container[i].at(m_timeColumn) + timeShiftValue >= comparisonTimeValue)
        {
            long double difference1 = std::abs(m_container[lastValidPosition].at(m_timeColumn) +
                                               timeShiftValue - comparisonTimeValue);
            long double difference2 =
                std::abs(m_container[i].at(m_timeColumn) + timeShiftValue - comparisonTimeValue);

            if (difference1 < difference2)
            {
                m_lastValidPosition = lastValidPosition;
            }
            else
            {
                m_lastValidPosition = i;
            }
            valueFound = true;
            break;
        }
        lastValidPosition = i;
    }

    if (valueFound && m_numOfPasses > 0 && m_lastValidPosition == 0)
    {
        long double difference1 = std::abs(m_container[m_lastValidPosition].at(m_timeColumn) +
                                           timeShiftValue - comparisonTimeValue);
        long double difference2 =
            std::abs(m_container[m_container.size() - 1].at(m_timeColumn) +
                     ((m_numOfPasses - 1) * m_container[m_container.size() - 1].at(m_timeColumn)) -
                     comparisonTimeValue);

        if (difference1 > difference2)
        {
            m_lastValidPosition = m_container.size() - 1;
            m_numOfPasses--;
            m_timeShiftValue = m_numOfPasses * m_container[m_container.size() - 1].at(m_timeColumn);
        }
    }

    NS_LOG_INFO("Done: " << valueFound
                         << " value: " << m_container[m_lastValidPosition].at(m_timeColumn)
                         << " @ line: " << m_lastValidPosition + 1 << " comparison time value: "
                         << comparisonTimeValue << " passes: " << m_numOfPasses);

    return valueFound;
}

void
SatInputFileStreamTimeLongDoubleContainer::Reset()
{
    NS_LOG_FUNCTION(this);

    ResetStream();
    ClearContainer();
}

void
SatInputFileStreamTimeLongDoubleContainer::ResetStream()
{
    NS_LOG_FUNCTION(this);

    if (m_inputFileStreamWrapper)
    {
        delete m_inputFileStreamWrapper;
        m_inputFileStreamWrapper = 0;
    }
    m_inputFileStream = 0;
}

void
SatInputFileStreamTimeLongDoubleContainer::ClearContainer()
{
    NS_LOG_FUNCTION(this);

    if (!m_container.empty())
    {
        for (uint32_t i = 0; i < m_container.size(); i++)
        {
            if (!m_container[i].empty())
            {
                m_container[i].clear();
            }
        }
        m_container.clear();
    }

    m_valuesInRow = 0;
    m_lastValidPosition = 0;
    m_numOfPasses = 0;
    m_timeShiftValue = 0;
}

} // namespace ns3
