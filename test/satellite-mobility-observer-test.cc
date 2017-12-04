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

/**
 * \file satellite-mobility-observer-test.cc
 * \ingroup satellite
 * \brief Test cases to unit test Satellite Mobility Observer model.
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-propagation-delay-model.h"
#include "../model/satellite-constant-position-mobility-model.h"
#include "../model/satellite-mobility-observer.h"
#include "../helper/satellite-helper.h"
#include "ns3/singleton.h"
#include "../utils/satellite-env-variables.h"

using namespace ns3;

// definitions for reference elevation angle table, g_refElAngles
static const uint32_t g_latitudeCount = 19;
static const uint32_t g_longitudeCount = 37;

static const double g_firstLatValue = -90.00;
static const double g_firstLonValue = -180.00;

static const double g_latStep = 10.00;
static const double g_lonStep = 10.00;

// reference test values, when sphere (radius 6378137 [m]) is used as reference ellipsoid
// satellite is at location latitude 0.00, longitude 33.00, altitude 35786000 [m]
// values in table are with altitude 0 [m]
//
// row = latitude from  -90 ... 90 with 10 degrees step
// col = longitude from -180 ... 180 with 10 degrees step
//
static const double g_refElAngles[g_latitudeCount][g_longitudeCount] =
{
  { -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, },

  { -1.6704625e+01, -1.7493055e+01, -1.8012186e+01, -1.8246050e+01, -1.8187401e+01,
    -1.7838061e+01, -1.7208826e+01, -1.6318974e+01, -1.5195434e+01, -1.3871747e+01,
    -1.2386929e+01, -1.0784340e+01, -9.1106057e+00, -7.4145745e+00, -5.7462845e+00,
    -4.1558514e+00, -2.6922153e+00, -1.4016983e+00, -3.2637365e-01,  4.9768367e-01,
    1.0421305e+00,  1.2878816e+00,  1.2262239e+00,  8.5934923e-01,  2.0021921e-01,
    -7.2821655e-01, -1.8943228e+00, -3.2594126e+00, -4.7795075e+00, -6.4070521e+00,
    -8.0924743e+00, -9.7855486e+00, -1.1436585e+01, -1.2997501e+01, -1.4422852e+01,
    -1.5670881e+01, -1.6704625e+01, },

  { -2.4576013e+01, -2.6154391e+01, -2.7198607e+01, -2.7670434e+01, -2.7552023e+01,
    -2.6847888e+01, -2.5584395e+01, -2.3806912e+01, -2.1575549e+01, -1.8960673e+01,
    -1.6039194e+01, -1.2892100e+01, -9.6032842e+00, -6.2593405e+00, -2.9498717e+00,
    2.3226561e-01,  3.1913676e+00,  5.8305290e+00,  8.0550058e+00, 9.7774715e+00,
    1.0924793e+01,  1.1445248e+01,  1.1314514e+01,  1.0538752e+01, 9.1538106e+00,
    7.2208060e+00,  4.8194220e+00,  2.0406964e+00, -1.0192362e+00, -4.2636521e+00,
    -7.5978706e+00, -1.0930338e+01, -1.4172702e+01, -1.7239467e+01, -2.0047834e+01,
    -2.2518238e+01, -2.4576013e+01, },

  { -3.2150830e+01, -3.4559650e+01, -3.6169920e+01, -3.6902409e+01, -3.6718280e+01,
    -3.5627470e+01, -3.3686477e+01, -3.0986701e+01, -2.7639000e+01, -2.3760356e+01,
    -1.9465727e+01, -1.4865194e+01, -1.0065058e+01, -5.1713370e+00, -2.9441703e-01,
    4.4461307e+00,  8.9176003e+00,  1.2972465e+01,  1.6450908e+01,  1.9189792e+01,
    2.1039320e+01,  2.1885576e+01,  2.1672558e+01,  2.0414606e+01,  1.8193117e+01,
    1.5139226e+01,  1.1410534e+01,  7.1704160e+00,  2.5744409e+00, -2.2357749e+00,
    -7.1331465e+00, -1.2002325e+01, -1.6735261e+01, -2.1226341e+01, -2.5368068e+01,
    -2.9048360e+01, -3.2150830e+01, },

  { -3.9340005e+01, -4.2663495e+01, -4.4926798e+01, -4.5969303e+01, -4.5706421e+01,
    -4.4160181e+01, -4.1450862e+01, -3.7756269e+01, -3.3266805e+01, -2.8156901e+01,
    -2.2575234e+01, -1.6647050e+01, -1.0482042e+01, -4.1841877e+00,  2.1377900e+00,
    8.3595752e+00,  1.4330196e+01,  1.9861842e+01,  2.4723918e+01,  2.8648396e+01,
    3.1356671e+01,  3.2613755e+01,  3.2296196e+01,  3.0436121e+01,  2.7209219e+01,
    2.2875939e+01,  1.7715647e+01,  1.1983180e+01,  5.8918217e+00, -3.8641798e-01,
    -6.7125435e+00, -1.2969801e+01, -1.9052647e+01, -2.4856049e+01, -3.0265212e+01,
    -3.5146265e+01, -3.9340005e+01, },

  { -4.6006510e+01, -5.0377333e+01, -5.3449005e+01, -5.4896833e+01, -5.4529539e+01,
    -5.2398361e+01, -4.8765619e+01, -4.3969265e+01, -3.8312737e+01, -3.2027268e+01,
    -2.5278415e+01, -1.8185662e+01, -1.0841716e+01, -3.3289119e+00,  4.2661058e+00,
    1.1837327e+01,  1.9241765e+01,  2.6277592e+01,  3.2658732e+01,  3.7994962e+01,
    4.1806163e+01,  4.3619473e+01,  4.3158455e+01,  4.0497024e+01,  3.6015197e+01,
    3.0207381e+01,  2.3523617e+01,  1.6311315e+01,  8.8197845e+00,  1.2243958e+00,
    -6.3490162e+00, -1.3804110e+01, -2.1057702e+01, -2.8025122e+01, -3.4605077e+01,
    -4.0662117e+01, -4.6006510e+01, },

  { -5.1930043e+01, -5.7520985e+01, -6.1664013e+01, -6.3706420e+01, -6.3181670e+01,
    -6.0221250e+01, -5.5425332e+01, -4.9408454e+01, -4.2595155e+01, -3.5239767e+01,
    -2.7490147e+01, -1.9435029e+01, -1.1133295e+01, -2.6328414e+00,  6.0148092e+00,
    1.4739663e+01,  2.3433938e+01,  3.1922592e+01,  3.9915329e+01,  4.6932754e+01,
    5.2230631e+01,  5.4870709e+01,  5.4190691e+01,  5.0376730e+01,  4.4284022e+01,
    3.6803157e+01,  2.8567067e+01,  1.9969144e+01,  1.1246230e+01,  2.5419583e+00,
    -6.0538155e+00, -1.4480449e+01, -2.2689549e+01, -3.0630706e+01, -3.8235778e+01,
    -4.5396738e+01, -5.1930043e+01, },

  { -5.6763473e+01, -6.3721803e+01, -6.9350213e+01, -7.2406072e+01, -7.1595300e+01,
    -6.7322356e+01, -6.1051957e+01, -5.3761638e+01, -4.5899432e+01, -3.7661064e+01,
    -2.9133792e+01, -2.0356792e+01, -1.1348040e+01, -2.1185707e+00,  7.3174061e+00,
    1.6932120e+01,  2.6669719e+01,  3.6421102e+01,  4.5972840e+01,  5.4890661e+01,
    6.2253932e+01,  6.6299419e+01,  6.5223137e+01,  5.9587071e+01,  5.1443019e+01,
    4.2196812e+01,  3.2529315e+01,  2.2765306e+01,  1.3067511e+01,  3.5195690e+00,
    -5.8361176e+00, -1.4978614e+01, -2.3896132e+01, -3.2576186e+01, -4.0994468e+01,
    -4.9096993e+01, -5.6763473e+01, },

  { -6.0018125e+01, -6.8243134e+01, -7.5780862e+01, -8.0930434e+01, -7.9407340e+01,
    -7.2901185e+01, -6.5003558e+01, -5.6632347e+01, -4.8002623e+01, -3.9171269e+01,
    -3.0147310e+01, -2.0921952e+01, -1.1479516e+01, -1.8030130e+00,  8.1215023e+00,
    1.8300042e+01,  2.8723815e+01,  3.9357750e+01,  5.0116479e+01,  6.0796148e+01,
    7.0799966e+01,  7.7720429e+01,  7.5667940e+01,  6.6957660e+01,  5.6556579e+01,
    4.5806960e+01,  3.5082849e+01,  2.4526346e+01,  1.4198272e+01, 4.1213054e+00,
    -5.7027141e+00, -1.5283649e+01, -2.4637188e+01, -3.3780337e+01, -4.2726850e+01,
    -5.1480271e+01, -6.0018125e+01, },

  { -6.1181558e+01, -6.9969924e+01, -7.8698721e+01, -8.7394065e+01, -8.3918289e+01,
    -7.5212668e+01, -6.6463206e+01, -5.7643574e+01, -4.8726780e+01, -3.9685056e+01,
    -3.0489905e+01, -2.1112394e+01, -1.1523783e+01, -1.6966442e+00,  8.3934130e+00,
    1.8765296e+01,  2.9429276e+01,  4.0383351e+01,  5.1609653e+01,  6.3071832e+01,
    7.4714498e+01,  8.6465700e+01,  8.1757338e+01,  7.0040060e+01,  5.8461787e+01,
    4.7088358e+01,  3.5967660e+01,  2.5128442e+01,  1.4581832e+01,  4.3244734e+00,
    -5.6577772e+00, -1.5386361e+01, -2.4887136e+01, -3.4188224e+01, -4.3318546e+01,
    -5.2306916e+01, -6.1181558e+01, },

  { -6.0018125e+01, -6.8243134e+01, -7.5780862e+01, -8.0930434e+01, -7.9407340e+01,
    -7.2901185e+01, -6.5003558e+01, -5.6632347e+01, -4.8002623e+01, -3.9171269e+01,
    -3.0147310e+01, -2.0921952e+01, -1.1479516e+01, -1.8030130e+00,  8.1215023e+00,
    1.8300042e+01,  2.8723815e+01,  3.9357750e+01,  5.0116479e+01,  6.0796148e+01,
    7.0799966e+01,  7.7720429e+01,  7.5667940e+01,  6.6957660e+01,  5.6556579e+01,
    4.5806960e+01,  3.5082849e+01,  2.4526346e+01,  1.4198272e+01,  4.1213054e+00,
    -5.7027141e+00, -1.5283649e+01, -2.4637188e+01, -3.3780337e+01, -4.2726850e+01,
    -5.1480271e+01, -6.0018125e+01, },

  { -5.6763473e+01, -6.3721803e+01, -6.9350213e+01, -7.2406072e+01, -7.1595300e+01,
    -6.7322356e+01, -6.1051957e+01, -5.3761638e+01, -4.5899432e+01, -3.7661064e+01,
    -2.9133792e+01, -2.0356792e+01, -1.1348040e+01, -2.1185707e+00,  7.3174061e+00,
    1.6932120e+01,  2.6669719e+01,  3.6421102e+01,  4.5972840e+01,  5.4890661e+01,
    6.2253932e+01,  6.6299419e+01,  6.5223137e+01,  5.9587071e+01,  5.1443019e+01,
    4.2196812e+01,  3.2529315e+01,  2.2765306e+01,  1.3067511e+01,  3.5195690e+00,
    -5.8361176e+00, -1.4978614e+01, -2.3896132e+01, -3.2576186e+01, -4.0994468e+01,
    -4.9096993e+01, -5.6763473e+01, },

  { -5.1930043e+01, -5.7520985e+01, -6.1664013e+01, -6.3706420e+01, -6.3181670e+01,
    -6.0221250e+01, -5.5425332e+01, -4.9408454e+01, -4.2595155e+01, -3.5239767e+01,
    -2.7490147e+01, -1.9435029e+01, -1.1133295e+01, -2.6328414e+00,  6.0148092e+00,
    1.4739663e+01,  2.3433938e+01,  3.1922592e+01,  3.9915329e+01,  4.6932754e+01,
    5.2230631e+01,  5.4870709e+01,  5.4190691e+01,  5.0376730e+01,  4.4284022e+01,
    3.6803157e+01,  2.8567067e+01,  1.9969144e+01,  1.1246230e+01,  2.5419583e+00,
    -6.0538155e+00, -1.4480449e+01, -2.2689549e+01, -3.0630706e+01, -3.8235778e+01,
    -4.5396738e+01, -5.1930043e+01, },

  { -4.6006510e+01, -5.0377333e+01, -5.3449005e+01, -5.4896833e+01, -5.4529539e+01,
    -5.2398361e+01, -4.8765619e+01, -4.3969265e+01, -3.8312737e+01, -3.2027268e+01,
    -2.5278415e+01, -1.8185662e+01, -1.0841716e+01, -3.3289119e+00,  4.2661058e+00,
    1.1837327e+01,  1.9241765e+01,  2.6277592e+01,  3.2658732e+01,  3.7994962e+01,
    4.1806163e+01,  4.3619473e+01,  4.3158455e+01,  4.0497024e+01,  3.6015197e+01,
    3.0207381e+01,  2.3523617e+01,  1.6311315e+01,  8.8197845e+00,  1.2243958e+00,
    -6.3490162e+00, -1.3804110e+01, -2.1057702e+01, -2.8025122e+01, -3.4605077e+01,
    -4.0662117e+01, -4.6006510e+01, },

  { -3.9340005e+01, -4.2663495e+01, -4.4926798e+01, -4.5969303e+01, -4.5706421e+01,
    -4.4160181e+01, -4.1450862e+01, -3.7756269e+01, -3.3266805e+01, -2.8156901e+01,
    -2.2575234e+01, -1.6647050e+01, -1.0482042e+01, -4.1841877e+00,  2.1377900e+00,
    8.3595752e+00,  1.4330196e+01,  1.9861842e+01,  2.4723918e+01,  2.8648396e+01,
    3.1356671e+01,  3.2613755e+01,  3.2296196e+01,  3.0436121e+01,  2.7209219e+01,
    2.2875939e+01,  1.7715647e+01,  1.1983180e+01,  5.8918217e+00, -3.8641798e-01,
    -6.7125435e+00, -1.2969801e+01, -1.9052647e+01, -2.4856049e+01, -3.0265212e+01,
    -3.5146265e+01, -3.9340005e+01, },

  { -3.2150830e+01, -3.4559650e+01, -3.6169920e+01, -3.6902409e+01, -3.6718280e+01,
    -3.5627470e+01, -3.3686477e+01, -3.0986701e+01, -2.7639000e+01, -2.3760356e+01,
    -1.9465727e+01, -1.4865194e+01, -1.0065058e+01, -5.1713370e+00, -2.9441703e-01,
    4.4461307e+00,  8.9176003e+00,  1.2972465e+01,  1.6450908e+01,  1.9189792e+01,
    2.1039320e+01,  2.1885576e+01,  2.1672558e+01,  2.0414606e+01,  1.8193117e+01,
    1.5139226e+01,  1.1410534e+01,  7.1704160e+00,  2.5744409e+00, -2.2357749e+00,
    -7.1331465e+00, -1.2002325e+01, -1.6735261e+01, -2.1226341e+01, -2.5368068e+01,
    -2.9048360e+01, -3.2150830e+01, },

  { -2.4576013e+01, -2.6154391e+01, -2.7198607e+01, -2.7670434e+01, -2.7552023e+01,
    -2.6847888e+01, -2.5584395e+01, -2.3806912e+01, -2.1575549e+01, -1.8960673e+01,
    -1.6039194e+01, -1.2892100e+01, -9.6032842e+00, -6.2593405e+00, -2.9498717e+00,
    2.3226561e-01,  3.1913676e+00,  5.8305290e+00,  8.0550058e+00,  9.7774715e+00,
    1.0924793e+01,  1.1445248e+01,  1.1314514e+01,  1.0538752e+01,  9.1538106e+00,
    7.2208060e+00,  4.8194220e+00,  2.0406964e+00, -1.0192362e+00, -4.2636521e+00,
    -7.5978706e+00, -1.0930338e+01, -1.4172702e+01, -1.7239467e+01, -2.0047834e+01,
    -2.2518238e+01, -2.4576013e+01, },

  { -1.6704625e+01, -1.7493055e+01, -1.8012186e+01, -1.8246050e+01, -1.8187401e+01,
    -1.7838061e+01, -1.7208826e+01, -1.6318974e+01, -1.5195434e+01, -1.3871747e+01,
    -1.2386929e+01, -1.0784340e+01, -9.1106057e+00, -7.4145745e+00, -5.7462845e+00,
    -4.1558514e+00, -2.6922153e+00, -1.4016983e+00, -3.2637365e-01,  4.9768367e-01,
    1.0421305e+00,  1.2878816e+00,  1.2262239e+00,  8.5934923e-01,  2.0021921e-01,
    -7.2821655e-01, -1.8943228e+00, -3.2594126e+00, -4.7795075e+00, -6.4070521e+00,
    -8.0924743e+00, -9.7855486e+00, -1.1436585e+01, -1.2997501e+01, -1.4422852e+01,
    -1.5670881e+01, -1.6704625e+01, },

  { -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00, -8.6018742e+00,
    -8.6018742e+00, -8.6018742e+00, }
};

/**
 * \ingroup satellite
 * \brief Test case to unit test Satellite Mobility Observer.
 *
 * This case tests that SatMobilityObserver object can be created successfully and it calculates correctly.
 *  1.  Create SatMobilityObserver object with geo mobility and own mobility.
 *  2.  Set position of Geo satellite mobility.
 *  3.  Set position of own mobility.
 *  4.  Set propagation delay and another mobility to observer.
 *  5.  Get timing advance from observer.
 *  6.  Set position of own mobility.
 *  7.  Get elevation angle from observer.
 *  8.  Repeat steps 6 and 7 several times.
 *  9.  Set Geo satellite using reference satellite and repeat steps 6 and 7 several times.
 *
 *  Expected result:
 *      Timing advance and elevation angle values got are correct.
 *
 *  Notes:
 *    At least corner points, valid values and out of range (NAN) elevation angle values is needed to test.
 *    When reference satellite is used for example MatLab is useful to get reference values for elevation angles.
 *
 *
 */
class SatMobilityObserverTestCase : public TestCase
{
public:
  SatMobilityObserverTestCase ();
  virtual ~SatMobilityObserverTestCase ();

private:
  virtual void DoRun (void);
};

SatMobilityObserverTestCase::SatMobilityObserverTestCase ()
  : TestCase ("Test satellite mobility observer model.")
{
}

SatMobilityObserverTestCase::~SatMobilityObserverTestCase ()
{
}

void
SatMobilityObserverTestCase::DoRun (void)
{
  // Set simulation output details
  Singleton<SatEnvVariables>::Get ()->DoInitialize ();
  Singleton<SatEnvVariables>::Get ()->SetOutputVariables ("test-sat-mobility-observer", "", true);

  // create mobilities
  Ptr<SatConstantPositionMobilityModel> gwMob = CreateObject<SatConstantPositionMobilityModel> ();
  Ptr<SatConstantPositionMobilityModel> utMob = CreateObject<SatConstantPositionMobilityModel> ();
  Ptr<SatConstantPositionMobilityModel> geoMob = CreateObject<SatConstantPositionMobilityModel> ();

  // create propagation delays
  Ptr<SatConstantPropagationDelayModel> gwProgDelay = CreateObject<SatConstantPropagationDelayModel> ();
  Ptr<SatConstantPropagationDelayModel> utProgDelay = CreateObject<SatConstantPropagationDelayModel> ();

  // set propagation delays
  gwProgDelay->SetDelay (Seconds (200));
  utProgDelay->SetDelay (Seconds (300));

  // set satellite position, altitude is the Earth radius
  double earthRadius = CalculateDistance ( GeoCoordinate (0.00, 0.00, 0.00).ToVector (), Vector (0,0,0) );
  GeoCoordinate satellitePosition = GeoCoordinate (0.00, 0.00, earthRadius);
  geoMob->SetGeoPosition (satellitePosition);

  // set some positions to UT and GW
  gwMob->SetGeoPosition (GeoCoordinate (0.00, 0.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (0.00, 0.00, 0.00));

  // create mobility observers
  Ptr<SatMobilityObserver> utObserver = CreateObject<SatMobilityObserver> (utMob, geoMob);
  Ptr<SatMobilityObserver> gwObserver = CreateObject<SatMobilityObserver> (gwMob, geoMob);

  utObserver->ObserveTimingAdvance (utProgDelay, gwProgDelay, gwMob);
  double timingAdvance = utObserver->GetTimingAdvance ().GetSeconds ();

  // check that we use correct earth radius
  NS_TEST_ASSERT_MSG_EQ ( earthRadius, 6378137, "Earth radius is not what expected");

  // check that timing advance is correct
  NS_TEST_ASSERT_MSG_EQ ( timingAdvance, 500, "Timing Advance incorrect");


  // Test that we get 0 degrees elevation angle at range points, where we stop seeing satellite
  gwMob->SetGeoPosition (GeoCoordinate (60.00, 0.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (00.0, 60.00, 0.00));

  double utEl = utObserver->GetElevationAngle ();
  double gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( utEl, 0, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( gwEl, 0, "GW elevation angle incorrect");

  gwMob->SetGeoPosition (GeoCoordinate (-60.0, 0.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (0.00, -60.00, 0.00));

  utEl = utObserver->GetElevationAngle ();
  gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( utEl, 0, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( gwEl, 0, "GW elevation angle incorrect");

  gwMob->SetGeoPosition (GeoCoordinate (45.00, 45.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (-45.00, -45.00, 0.00));

  utEl = utObserver->GetElevationAngle ();
  gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( utEl, 0, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( gwEl, 0, "GW elevation angle incorrect");

  gwMob->SetGeoPosition (GeoCoordinate (-45.00, 45.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (45.00, -45.00, 0.00));

  utEl = utObserver->GetElevationAngle ();
  gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( utEl, 0, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( gwEl, 0, "GW elevation angle incorrect");


  // Test that we get valid elevation angle at points where we should see satellite

  gwMob->SetGeoPosition (GeoCoordinate (44.01, 45.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (45.00, -42.00, 0.00));

  utEl = utObserver->GetElevationAngle ();
  gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( std::isnan (utEl), false, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (gwEl), false, "GW elevation angle incorrect");

  gwMob->SetGeoPosition (GeoCoordinate (10.0, 10.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (0.00, 0.00, 0.00));

  utEl = utObserver->GetElevationAngle ();
  gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( std::isnan (utEl), false, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (gwEl), false, "GW elevation angle incorrect");


  // Test that we get invalid elevation angle at points where we shouldn't see satellite

  gwMob->SetGeoPosition (GeoCoordinate (45.01, 45.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (45.00, -45.01, 0.00));

  utEl = utObserver->GetElevationAngle ();
  gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( std::isnan (utEl), true, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (gwEl), true, "GW elevation angle incorrect");

  gwMob->SetGeoPosition (GeoCoordinate (90.0, 0.00, 0.00));
  utMob->SetGeoPosition (GeoCoordinate (0.00, 180.00, 0.00));

  utEl = utObserver->GetElevationAngle ();
  gwEl = gwObserver->GetElevationAngle ();

  NS_TEST_ASSERT_MSG_EQ ( std::isnan (utEl), true, "UT elevation angle incorrect");
  NS_TEST_ASSERT_MSG_EQ ( std::isnan (gwEl), true, "GW elevation angle incorrect");

  // Test that we get correct elevation angle with our reference satellite

  satellitePosition.SetLatitude (0.00);
  satellitePosition.SetLongitude (33.00);
  satellitePosition.SetAltitude (35786000.00);
  geoMob->SetGeoPosition (satellitePosition);

  Ptr<UniformRandomVariable> randAlt = CreateObject<UniformRandomVariable> ();
  randAlt->SetAttribute ("Min", DoubleValue (-200.00));
  randAlt->SetAttribute ("Max", DoubleValue (600.00));

  gwObserver->SetAttribute ("MinAltitude", DoubleValue (-200.00));
  gwObserver->SetAttribute ("MaxAltitude", DoubleValue (600.00));

  for ( uint32_t i = 0; i < g_latitudeCount; i++ )
    {
      for ( uint32_t j = 0; j < g_longitudeCount; j++ )
        {
          double lat = g_firstLatValue + (double)i * g_latStep;
          double lon = g_firstLonValue + (double)j * g_lonStep;
          double alt = randAlt->GetValue ();

          gwMob->SetGeoPosition (GeoCoordinate ( lat, lon, alt));
          gwEl = gwObserver->GetElevationAngle ();
          double refValue = g_refElAngles[i][j];

          if ( refValue < 0 )
            {
              // NAN should be returned, because only points where we can see satellite is accepted
              NS_TEST_ASSERT_MSG_EQ ( std::isnan (gwEl), true, "UT elevation angle incorrect: lat-> " << lat << " lon-> " << " alt->" << alt );
            }
          else
            {
              // valid values should be returned and it should be inside tolerance
              NS_TEST_ASSERT_MSG_EQ ( std::isnan (gwEl), false, "UT elevation angle incorrect (NAN): lat-> " << lat << " lon-> " << " alt" << alt );
              NS_TEST_ASSERT_MSG_EQ_TOL ( gwEl, refValue, 0.00001, "UT elevation angle incorrect: lat-> " << lat << " lon-> " << " alt" << alt );
            }
        }
    }

  Simulator::Destroy ();

  Singleton<SatEnvVariables>::Get ()->DoDispose ();
}

/**
 * \brief Test suite for Satellite mobility observer unit test cases.
 */
class SatMobilityObserverTestSuite : public TestSuite
{
public:
  SatMobilityObserverTestSuite ();
};

SatMobilityObserverTestSuite::SatMobilityObserverTestSuite ()
  : TestSuite ("sat-mobility-observer-test", UNIT)
{
  AddTestCase (new SatMobilityObserverTestCase, TestCase::QUICK);
}

// Do allocate an instance of this TestSuite
static SatMobilityObserverTestSuite satSatMobilityObserverTestSuite;

