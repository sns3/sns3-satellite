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
#ifndef SATELLITE_SINGLETON_H
#define SATELLITE_SINGLETON_H

#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Base class for singleton
 */
template <typename T>
class SatSingleton
{
public:
  /**
   * \brief Function for getting the instance of this singleton
   * \return Singleton
   */
    static T& Instance ()
    {
      if (SatSingleton::instance == NULL)
      {
          SatSingleton::instance = CreateInstance ();
      }
      return *(SatSingleton::instance);
    }

protected:
    /**
     * \brief Destructor
     */
    virtual ~SatSingleton ()
    {
      if(SatSingleton::instance != NULL)
      {
          delete SatSingleton::instance;
      }
      SatSingleton::instance = NULL;
    }

    /**
     * \brief Constructor
     */
    inline explicit SatSingleton ()
    {
      if (!SatSingleton::instance == NULL)
        {
          NS_FATAL_ERROR ("SatSingleton - Calling constructor twice");
        }
      SatSingleton::instance = static_cast<T*> (this);
    }

private:
    /**
     * \brief Function for creating new singleton instance
     */
    static T* CreateInstance ()
    {
      return new T ();
    }

    /**
     * \brief Singleton object
     */
    static T* instance;
};

template<typename T>
T* SatSingleton<T>::instance = NULL;

} // namespace ns3

#endif /* SATELLITE_SINGLETON_H */
