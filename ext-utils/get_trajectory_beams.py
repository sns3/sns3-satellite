#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Author: Aurelien DELRIEU / <adelrieu@toulouse.viveris.com>

"""
get_trajectory_beams.py - Get beams for a trajectory
"""

import argparse
import math
import os
import bisect


def load_trajectory(filepath):
    '''
    Load a trajectory file.

    Args:
        filepath:  the trajectory file to load

    Returns:
        list of dictionaries containing time and GPS locations
    '''
    locations = []
    with open(filepath) as fd:
        for line in fd:
            line = line.split()
            if len(line) == 0 or not line[0] or line[0][0] == '#':
                continue
            loc = {}
            loc['time'] = float(line[0])
            loc['lat'] = float(line[1])
            loc['lon'] = float(line[2])
            loc['alt'] = float(line[3])
            locations.append(loc)
    return locations


def load_antenna_gain(filepath):
    '''
    Load a antenna gains file.

    Args:
        filepath:  the antenna gains file

    Returns:
        list of GPS position and associated antenna gain
    '''
    antenna_gain = {}
    with open(filepath, 'r') as fd:
        for line in fd:
            line = line.split()
            if len(line) == 0 or not line[0] or line[0][0] == '#':
                continue
            lat = float(line[0])
            lon = float(line[1])
            gain = float(line[2])

            if lat not in antenna_gain:
                antenna_gain[lat] = {}
            antenna_gain[lat][lon] = gain
    return antenna_gain


def interpolate_antenna_gain(data, lat, lon):
    '''
    Interpolate location in antenna gain.
    Data is sorted by lat, then by lon.

    Args:
        data:  the tabulated values
        lat:   the first operand of the point to interpolate
        lon:   the second  operand of the point to interpolate

    Returns:
        the interpolated value or NaN
    '''
    #print('(lat, lon) = ({}, {})'.format(lat, lon))
    # Get latitude index on data
    lat_values = sorted(list(data))
    ilat = bisect.bisect_left(lat_values, lat)

    if ilat == 0 and lat < lat_values[0]:
        return float('NaN')
    elif ilat == len(lat_values) and lat_values[len(lat_values) - 1] < lat:
        return float('NaN')

    if lat_values[ilat - 1] == lat:
        return interpolate(data[lat], lon)
    elif lat_values[ilat] == lat:
        return interpolate(data[lat], lon)

    # Get longitude index on data
    lat0 = lat_values[ilat - 1]
    gain0 = interpolate(data[lat0], lon)
    if math.isnan(gain0):
        return float('NaN')
    
    lat1 = lat_values[ilat]
    gain1 = interpolate(data[lat1], lon)
    if math.isnan(gain1):
        return float('NaN')

    return interpolate({ lat0: gain0, lat1: gain1 }, lat)


def interpolate(values, x):
    '''
    Interpolate in a table.

    Args:
        values:  the values at format {x: f}
        x:       the value to interpolate

    Returns:
        the interpolated value of x
    '''
    xvalues = sorted(list(values))
    i0 = bisect.bisect_left(xvalues, x)
    if i0 == 0 and x < xvalues[0]:
        return float('NaN')
    elif i0 == len(xvalues) and xvalues[len(xvalues) - 1] < x:
        return float('NaN')

    if xvalues[i0 - 1] == x:
        return values[xvalues[i0 - 1]]
    if xvalues[i0] == x:
        return values[xvalues[i0]]
    
    coeff = (values[xvalues[i0]] - values[xvalues[i0 - 1]]) \
            / (xvalues[i0] - xvalues[i0 - 1])
    return values[xvalues[i0 - 1]] + coeff * (x - xvalues[i0 - 1])


def get_beam_id(filepath):
    '''
    Get the beam id from the antenna gain file name.

    Args:
        filepath: the antenna gain file path

    Returns:
        the beam id
    '''
    return int(filepath[filepath.index('_') + 1:filepath.index('.txt')])


if __name__ == "__main__":
    
    # Define arguments
    parser = argparse.ArgumentParser(
        description='Get the beam id for each GPS location composing a trajectory',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    parser.add_argument(
        'beams_dir',
        type=str,
        help='the directory containing the antenna gain for each beam',
    )

    parser.add_argument(
        'trajectory',
        type=str,
        metavar='PATH',
        help='the trajectory file path containing GPS locations',
    )

    parser.add_argument(
        '-v',
        '--verbose',
        help='enable verbosity',
        action='store_true',
        default=False,
    )

    # Parse arguments
    args = parser.parse_args()

    # Load the GPS locations of the trajectory
    locations = load_trajectory(args.trajectory)

    # Prepare data to check beams
    beam_files = os.listdir(args.beams_dir)
    beam_files.sort(key=get_beam_id)

    # Parse each beam and check each GPS positions
    for beam_file in beam_files:
        # Load beam file
        beam_id = get_beam_id(beam_file)
        antenna_gain = load_antenna_gain(
            os.path.join(args.beams_dir, beam_file)
        )

        for loc in locations:
            # Check antenna gain for the current beam
            value = interpolate_antenna_gain(
                antenna_gain,
                loc['lat'],
                loc['lon'],
            )
            if math.isnan(value):
                continue

            # Store beam and antenna gain
            if 'beam' not in loc:
                loc['beam'] = {}
            loc['beam'][beam_id] = value
            if 'best_beam' not in loc or loc['best_gain'] < value:
                loc['best_beam'] = beam_id
                loc['best_gain'] = value

    beams = set()
    for loc in locations:
        best_beam = None
        best_gain = None
        if 'best_beam' in loc:
            beams.add(loc['best_beam'])

    # Print all beams
    if not args.verbose:
        print(beams)
    else:
        print('# time lat lon alt beam_id antenna_gain')
        for loc in locations:
            msg = '{} {} {} {}'.format(loc['time'], loc['lat'], loc['lon'], loc['alt'])
            if 'best_beam' in loc:
                msg += ' {} {}'.format(loc['best_beam'], loc['best_gain'])
            print(msg)
