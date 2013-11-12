/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Washington
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
 * Modified by: Frans Laakso <frans.laakso@magister.fi>
 */

#ifndef SAT_INPUT_STREAM_WRAPPER_H
#define SAT_INPUT_STREAM_WRAPPER_H

#include <fstream>
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

/*
 * @brief A class encapsulating an STL input stream.
 *
 * This class wraps a pointer to a C++ std::istream and provides
 * reference counting of the object.
 *
 * This class uses a basic ns-3 reference counting base class but is not 
 * an ns3::Object with attributes, TypeId, or aggregation.
 */
class SatInputStreamWrapper : public SimpleRefCount<SatInputStreamWrapper>
{
public:
  /**
   *
   * @param filename
   * @param filemode
   */
  SatInputStreamWrapper (std::string filename, std::ios::openmode filemode);

  /**
   *
   * @param is
   */
  SatInputStreamWrapper (std::istream* is);

  /**
   *
   */
  ~SatInputStreamWrapper ();

  /**
   * Return a pointer to an istream previously set in the wrapper.
   *
   * \see SetStream
   *
   * \returns a pointer to the encapsulated std::istream
   */
  std::istream *GetStream (void);

private:
  /**
   *
   */
  std::istream *m_istream;

  /**
   *
   */
  bool m_destroyable;
};

} // namespace ns3

#endif /* SAT_INPUT_STREAM_WRAPPER_H */
