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

#ifndef SAT_OUTPUT_FSTREAM_DOUBLE_CONTAINER_H
#define SAT_OUTPUT_FSTREAM_DOUBLE_CONTAINER_H

#include <fstream>
#include "ns3/object.h"
#include "satellite-output-fstream-wrapper.h"
#include <ns3/gnuplot.h>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for output file stream container for double values.
 * The class implements storing the values and writing the stored
 * values into a file. A figure output in two dimensions is also supported.
 */
class SatOutputFileStreamDoubleContainer : public Object
{
public:
  typedef enum
  {
    RAW,
    DECIBEL,
    DECIBEL_AMPLITUDE
  } FigureUnitConversion_t;

  /**
   * \brief NS-3 function for type id
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   * \param filename file name
   * \param filemode file mode
   * \param valuesInRow number of values in a row
   */
  SatOutputFileStreamDoubleContainer (std::string filename, std::ios::openmode filemode, uint32_t valuesInRow);

  /**
   * \brief Constructor
   */
  SatOutputFileStreamDoubleContainer ();

  /**
   * \brief Destructor
   */
  ~SatOutputFileStreamDoubleContainer ();

  /**
   * \brief Function for writing the container contents to file
   */
  void WriteContainerToFile ();

  /**
   * \brief Function for adding the values to container
   */
  void AddToContainer (std::vector<double> newItem);

  /**
   * \brief Do needed dispose actions
   */
  void DoDispose ();

  /**
   * \brief Function for enabling the figure output
   * \param title title text
   * \param legendY Y-axis title
   * \param legendX X-axis title
   * \param keyPosition Legend position
   * \param figureUnitConversionType Conversion type for units in the figure
   * \param style 2D dataset figure style
   */
  void EnableFigureOutput (std::string title,
                           std::string legendY,
                           std::string legendX,
                           std::string keyPosition,
                           FigureUnitConversion_t figureUnitConversionType,
                           Gnuplot2dDataset::Style style);

private:
  /**
   * \brief Function for resetting the variables
   */
  void Reset ();

  /**
   * \brief Function for resetting the stream
   */
  void ResetStream ();

  /**
   * \brief Function for clearing the container
   */
  void ClearContainer ();

  /**
   * \brief Function for opening the output file stream
   */
  void OpenStream ();

  /**
   * \brief Function for printing the container contents into a figure
   */
  void PrintFigure ();

  /**
   * \brief Function for converting the container data samples
   * \param value original data sample value
   * \return converted data sample value
   */
  double ConvertValue (double value);

  /**
   * \brief Function for creating Gnuplot datasets
   * \return dataset
   */
  Gnuplot2dDataset GetGnuplotDataset ();

  /**
   * \brief Function for creating Gnuplots
   * \return Gnuplot
   */
  Gnuplot GetGnuplot ();

  /**
   * \brief Pointer to output file stream wrapper
   */
  SatOutputFileStreamWrapper* m_outputFileStreamWrapper;

  /**
   * \brief Pointer to output file stream
   */
  std::ofstream* m_outputFileStream;

  /**
   * \brief Container for value rows
   */
  std::vector<std::vector<double> > m_container;

  /**
   * \brief File name
   */
  std::string m_fileName;

  /**
   * \brief File mode
   */
  std::ios::openmode m_fileMode;

  /**
   * \brief Number of values in a row
   */
  uint32_t m_valuesInRow;

  /**
   * \brief Enable / disable printing of container contents into a figure
   */
  bool m_printFigure;

  /**
   * \brief Figure title
   */
  std::string m_title;

  /**
   * \brief  Figure Y-axis title
   */
  std::string m_legendY;

  /**
   * \brief Figure X-axis title
   */
  std::string m_legendX;

  /**
   * \brief Figure legend position
   */
  std::string m_keyPosition;

  /**
   * \brief Describes which unit conversion should be used with the figure
   */
  FigureUnitConversion_t m_figureUnitConversionType;

  /**
   * \brief 2D dataset figure style
   */
  Gnuplot2dDataset::Style m_style;
};

} // namespace ns3

#endif /* SAT_OUTPUT_FSTREAM_DOUBLE_CONTAINER_H */
