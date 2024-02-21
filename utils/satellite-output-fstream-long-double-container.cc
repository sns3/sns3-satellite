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

#include "satellite-output-fstream-long-double-container.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("SatOutputFileStreamLongDoubleContainer");

namespace ns3
{

TypeId
SatOutputFileStreamLongDoubleContainer::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SatOutputFileStreamLongDoubleContainer")
                            .SetParent<Object>()
                            .AddConstructor<SatOutputFileStreamLongDoubleContainer>();
    return tid;
}

SatOutputFileStreamLongDoubleContainer::SatOutputFileStreamLongDoubleContainer(
    std::string filename,
    std::ios::openmode filemode,
    uint32_t valuesInRow)
    : m_outputFileStreamWrapper(),
      m_outputFileStream(),
      m_container(),
      m_fileName(filename),
      m_fileMode(filemode),
      m_valuesInRow(valuesInRow),
      m_printFigure(false),
      m_figureUnitConversionType(RAW)
{
    NS_LOG_FUNCTION(this << m_fileName << m_fileMode);

    if (!(m_valuesInRow > 0))
    {
        NS_FATAL_ERROR("SatOutputFileStreamLongDoubleContainer::"
                       "SatOutputFileStreamLongDoubleContainer - No values in the row");
    }
}

SatOutputFileStreamLongDoubleContainer::SatOutputFileStreamLongDoubleContainer()
    : m_outputFileStreamWrapper(),
      m_outputFileStream(),
      m_container(),
      m_fileName(),
      m_fileMode(),
      m_valuesInRow(),
      m_printFigure(),
      m_figureUnitConversionType()
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("SatOutputFileStreamLongDoubleContainer::SatOutputFileStreamLongDoubleContainer "
                   "- Constructor not in use");
}

SatOutputFileStreamLongDoubleContainer::~SatOutputFileStreamLongDoubleContainer()
{
    NS_LOG_FUNCTION(this);

    Reset();
}

void
SatOutputFileStreamLongDoubleContainer::DoDispose()
{
    NS_LOG_FUNCTION(this);

    Reset();
    Object::DoDispose();
}

void
SatOutputFileStreamLongDoubleContainer::WriteContainerToFile()
{
    NS_LOG_FUNCTION(this);

    OpenStream();

    if (m_outputFileStream->is_open())
    {
        for (uint32_t i = 0; i < m_container.size(); i++)
        {
            for (uint32_t j = 0; j < m_valuesInRow; j++)
            {
                if (j + 1 == m_valuesInRow)
                {
                    *m_outputFileStream << m_container[i].at(j);
                }
                else
                {
                    *m_outputFileStream << m_container[i].at(j) << "\t";
                }
            }
            *m_outputFileStream << std::endl;
        }
        m_outputFileStream->close();
    }
    else
    {
        NS_ABORT_MSG("Output stream is not valid for writing.");
    }

    if (m_printFigure)
    {
        PrintFigure();
    }

    Reset();
}

void
SatOutputFileStreamLongDoubleContainer::PrintFigure()
{
    NS_LOG_FUNCTION(this);

    Gnuplot2dDataset dataset = GetGnuplotDataset();
    Gnuplot plot = GetGnuplot();
    plot.AddDataset(dataset);

    std::string plotFileName = m_fileName + ".plt";
    std::ofstream plotFile(plotFileName.c_str());
    plot.GenerateOutput(plotFile);
    plotFile.close();

    std::string conversionCommand = "gnuplot " + m_fileName + ".plt";

    int result = system(conversionCommand.c_str());

    if (result < 0)
    {
        std::cout << "Unable to open shell process for Gnuplot file conversion for " << m_fileName
                  << ", conversion not done!" << std::endl;
    }
}

void
SatOutputFileStreamLongDoubleContainer::AddToContainer(std::vector<long double> newItem)
{
    NS_LOG_FUNCTION(this);

    if (newItem.size() != m_valuesInRow)
    {
        NS_FATAL_ERROR(
            "SatOutputFileStreamLongDoubleContainer::AddToContainer - Invalid vector size");
    }

    m_container.push_back(newItem);
}

void
SatOutputFileStreamLongDoubleContainer::OpenStream()
{
    NS_LOG_FUNCTION(this);

    m_outputFileStreamWrapper = new SatOutputFileStreamWrapper(m_fileName, m_fileMode);
    m_outputFileStream = m_outputFileStreamWrapper->GetStream();
}

void
SatOutputFileStreamLongDoubleContainer::Reset()
{
    NS_LOG_FUNCTION(this);

    ResetStream();
    ClearContainer();
}

void
SatOutputFileStreamLongDoubleContainer::ResetStream()
{
    NS_LOG_FUNCTION(this);

    if (m_outputFileStreamWrapper)
    {
        delete m_outputFileStreamWrapper;
        m_outputFileStreamWrapper = 0;
    }
    m_outputFileStream = 0;

    m_fileName = "";
    m_fileMode = std::ofstream::out;
}

void
SatOutputFileStreamLongDoubleContainer::ClearContainer()
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
}

Gnuplot2dDataset
SatOutputFileStreamLongDoubleContainer::GetGnuplotDataset()
{
    NS_LOG_FUNCTION(this);

    Gnuplot2dDataset ret;
    ret.SetTitle(m_title);
    ret.SetStyle(Gnuplot2dDataset::LINES);

    if (!m_container.empty())
    {
        switch (m_valuesInRow)
        {
        case 2: {
            for (uint32_t i = 0; i < m_container.size(); i++)
            {
                ret.Add(m_container[i].at(0), ConvertValue(m_container[i].at(1)));
            }
            break;
        }
        default: {
            NS_ABORT_MSG("SatOutputFileStreamLongDoubleContainer::GetGnuplotDataset - Figure "
                         "output not implemented for "
                         << m_valuesInRow << " columns.");
        }
        }
    }
    return ret;
}

long double
SatOutputFileStreamLongDoubleContainer::ConvertValue(long double value)
{
    NS_LOG_FUNCTION(this << value);

    switch (m_figureUnitConversionType)
    {
    case RAW: {
        return value;
    }
    case DECIBEL: {
        if (value > 0)
        {
            return 10.0 * std::log10(value);
        }
        return 10.0 * std::log10(std::numeric_limits<long double>::min());
    }
    case DECIBEL_AMPLITUDE: {
        if (value > 0)
        {
            return 20.0 * std::log10(value);
        }
        return 20.0 * std::log10(std::numeric_limits<long double>::min());
    }
    default: {
        NS_ABORT_MSG(
            "SatOutputFileStreamLongDoubleContainer::ConvertValue - Invalid conversion type.");
        break;
    }
    }
    return -1;
}

Gnuplot
SatOutputFileStreamLongDoubleContainer::GetGnuplot()
{
    NS_LOG_FUNCTION(this);

    Gnuplot ret(m_fileName + ".png");
    ret.SetTitle(m_title);
    ret.SetTerminal("png");
    ret.SetLegend(m_legendY, m_legendX);
    ret.AppendExtra(m_keyPosition);
    ret.AppendExtra("set grid xtics mxtics ytics");
    return ret;
}

void
SatOutputFileStreamLongDoubleContainer::EnableFigureOutput(
    std::string title,
    std::string legendY,
    std::string legendX,
    std::string keyPosition,
    FigureUnitConversion_t figureUnitConversionType)
{
    NS_LOG_FUNCTION(this);

    m_printFigure = true;
    m_title = title;
    m_legendY = legendY;
    m_legendX = legendX;
    m_keyPosition = keyPosition;
    m_figureUnitConversionType = figureUnitConversionType;
}

} // namespace ns3
