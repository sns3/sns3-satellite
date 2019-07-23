#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Author: Aurelien DELRIEU / <adelrieu@toulouse.viveris.com>

"""
generate_trajectory.py - Generate GPS points composing a trajectory
"""

import math
import argparse
from collections import namedtuple


EARTH_RADIUS = 6371009.0  # meters


Coordinates = namedtuple('Coordinates', 'latitude longitude altitude')


def generate_locations(departure, arrival, speed, time_step, time_offset):
    """
    Generate a list containing interpolated location

    Args:
        departure:    departure GPS location
        arrival:      arrival GPS location
        speed:        speed
        time_step:    time between to GPS location
        time_offset:  time offset

    Returns:
        list of GPS location of the trajectory
    """
    # Evaluate moving during time step
    Δloc = Coordinates(*(
        stop - start
        for start, stop in zip(departure, arrival)
    ))

    halfway_lat = (departure.latitude + arrival.latitude) / 2.0
    dist = math.cos(math.radians(halfway_lat)) * Δloc.longitude
    dist = math.radians(Δloc.latitude) ** 2 + math.radians(dist) ** 2
    dist = EARTH_RADIUS * math.sqrt(dist)
    speed = speed * 1000.0 / 3600.0
    n = math.ceil(dist / speed / time_step)

    # Compute all GPS location
    locs = [
        {
            'time': time_offset + i * time_step,
            'coords': Coordinates(*(
                start + i * step / n
                for start, step in zip(departure, Δloc)
            )),
        }
        for i in range(n)
    ]
    locs.append({
            'time': time_offset + n * time_step,
            'coords': arrival,
    })

    return locs


def gps_location(value):
    """
    Define a GPS location.
    Check a value is a string matching the format "lat long alt".

    Args:
        value:  the value to check

    Returns:
        dictionary with value 'alt', 'lat', and 'lon'

    Raises:
        ArgumentTypeError: if value is invalid
    """
    coordinates = value.split(' ')
    if len(coordinates) != 3:
        raise argparse.ArgumentTypeError('Invalid format')

    return Coordinates(*map(float, coordinates))


def strictly_positive_float(value):
    """
    Define a strictly positive float.

    Args:
        value:  the value to check

    Returns:
        strictly positive value

    Raises:
        ValueError: if value is not a float
        ArgumentTypeError: if value is invalid
    """
    value = float(value)
    if value <= 0:
        raise argparse.ArgumentTypeError('Negative or null value')

    return value


if __name__ == "__main__":
    # Define arguments
    parser = argparse.ArgumentParser(
        description='Generate GPS point composing a trajectory',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    parser.add_argument(
        'departure',
        type=gps_location,
        help='departure GPS location',
    )

    parser.add_argument(
        'arrival',
        type=gps_location,
        help='arrival GPS location',
    )

    parser.add_argument(
        'speed',
        type=strictly_positive_float,
        help='speed (km/h)',
    )

    parser.add_argument(
        '--time-step',
        type=strictly_positive_float,
        help='time step (s) between two GPS location (default: 60)',
        default=60,
    )

    parser.add_argument(
        '--time-offset',
        type=strictly_positive_float,
        help='time offset (s) (default: 0)',
        default=0,
    )

    # Parse arguments
    args = parser.parse_args()

    # Generate GPS locations of the trajectory
    locations = generate_locations(
        args.departure,
        args.arrival,
        args.speed,
        args.time_step,
        args.time_offset,
    )

    # Print all GPS locations
    print('# time lat lon alt')
    for loc in locations:
        print(loc['time'], ' '.join(map(str, loc['coords'])))
