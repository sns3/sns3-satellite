#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Author: Aurelien DELRIEU / <adelrieu@toulouse.viveris.com>

"""
get_beam_center.py - Get the GPS location of beam center
"""

import os
import math
import operator
import argparse
from contextlib import suppress


def command_line_parser():
    """Define a parser for command line arguments"""

    parser = argparse.ArgumentParser(
        description='Get the center of beam (based on max antenna gain)',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    parser.add_argument(
        'beams_dir',
        type=str,
        help='the directory containing the antenna gain for each beam',
    )

    return parser


def parse_file(filepath):
    """
    Parse an antenna gains file.

    Args:
        filepath:  the antenna gains file

    Yields:
        latitude, longitude and gain of entries in the file
    """
    with open(filepath) as antenna_gain_pattern:
        for line in antenna_gain_pattern:
            with suppress(ValueError):
                lat, lon, gain = map(float, line.split())
                yield lat, lon, gain


def get_max_antenna_gain_location(filepath):
    """
    Get max antenna gains.

    Args:
        filepath:  the antenna gains file

    Returns:
        location of the max gain, and gain
    """
    return max(
            (item for item in parse_file(filepath) if not math.isnan(item[2])),
            key=operator.itemgetter(2)
    )


def get_beam_id(filepath):
    """
    Get the beam id from the antenna gain file name.

    Args:
        filepath: the antenna gain file path

    Returns:
        the beam id
    """
    return int(filepath[filepath.index('_') + 1:filepath.index('.txt')])


def main(beam_directory):
    """Print the position and gain of the center of each beam in beam_directory"""

    # List beam files indexed by their beam ID
    beam_files = {
            get_beam_id(filename): filename
            for filename in os.listdir(beam_directory)
    }

    # Parse each beam and check each GPS positions
    print('beam,lat,lon,gain')
    for beam_id in sorted(beam_files):
        beam_path = os.path.join(beam_directory, beam_files[beam_id])
        center = get_max_antenna_gain_location(beam_path)
        print(beam_id, *center, sep=',')


if __name__ == "__main__":
    args = command_line_parser().parse_args()
    main(args.beams_dir)
