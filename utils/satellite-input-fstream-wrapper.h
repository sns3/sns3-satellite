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

#ifndef SAT_INPUT_FSTREAM_WRAPPER_H
#define SAT_INPUT_FSTREAM_WRAPPER_H

#include <fstream>
#include "ns3/object.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

/**
 * \brief A class encapsulating an STL input stream.
 *
 * This class wraps a pointer to a C++ std::ifstream and provides
 * reference counting of the object.
 *
 * This class uses a basic ns-3 reference counting base class but is not
 * an ns3::Object with attributes, TypeId, or aggregation.
 */
class SatInputFileStreamWrapper : public SimpleRefCount<SatInputFileStreamWrapper>
{
public:
  /**
   * \brief Constructor
   * \param filename file name
   * \param filemode file mode
   */
  SatInputFileStreamWrapper (std::string filename, std::ios::openmode filemode);

  /**
   * \brief Destructor
   */
  ~SatInputFileStreamWrapper ();

  /**
   * \brief Return a pointer to an ifstream previously set in the wrapper.
   * \return a pointer to the encapsulated std::ifstream
   */
  std::ifstream * GetStream (void);

private:
  /**
   * \brief Input file stream
   */
  std::ifstream *m_ifstream;

  /**
   * \brief Is the stream destroyable
   */
  bool m_destroyable;
};

} // namespace ns3

#endif /* SAT_INPUT_FSTREAM_WRAPPER_H */
