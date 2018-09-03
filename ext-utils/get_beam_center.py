#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Author: Aurelien DELRIEU / <adelrieu@toulouse.viveris.com>

"""
get_beam_center.py - Get the GPS location of beam center
"""

import argparse
import math
import os
import bisect


def get_max_antenna_gain_location(filepath):
    '''
    Get max antenna gains.

    Args:
        filepath:  the antenna gains file

    Returns:
        location of the max gain, and gain
    '''
    max_item = None
    with open(filepath, 'r') as fd:
        for line in fd:
            line = line.split()
            if len(line) == 0 or not line[0] or line[0][0] == '#':
                continue
            if math.isnan(float(line[2])):
                continue
            if max_item is None:
                max_item = {}
                max_item['lat'] = float(line[0])
                max_item['lon'] = float(line[1])
                max_item['gain'] = float(line[2])
            elif max_item['gain'] < float(line[2]):
                dummy = max_item['gain']
                max_item['lat'] = float(line[0])
                max_item['lon'] = float(line[1])
                max_item['gain'] = float(line[2])
    return max_item


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
        description='Get the center of beam (based on max antenna gain)',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    parser.add_argument(
        'beams_dir',
        type=str,
        help='the directory containing the antenna gain for each beam',
    )

    # Parse arguments
    args = parser.parse_args()

    # Prepare data to check beams
    beam_files = os.listdir(args.beams_dir)
    beam_files.sort(key=get_beam_id)

    # Parse each beam and check each GPS positions
    print('beam,lat,lon,gain')
    for beam_file in beam_files:
        # Load beam file
        beam_id = get_beam_id(beam_file)
        item = get_max_antenna_gain_location(
            os.path.join(args.beams_dir, beam_file)
        )
        print('{},{},{},{}'.format(
            beam_id,
            item['lat'],
            item['lon'],
            item['gain'],
        ))
