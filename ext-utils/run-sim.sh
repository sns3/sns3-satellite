#!/bin/bash

# Copyright (c) 2014 Magister Solutions Ltd
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Jani Puttonen <jani.puttonen@magister.fi>
#

####################################################
# Example simulation script for pushing simulations 
# to grid engine. Run the script in the NS-3 root.
####################################################

# $1: Number of UTs
# $2: Simulation length in seconds
run_sim()
{
    name="uts$1-simTime$2"
    echo "./waf --run \"sat-training-example \
                          --utsPerBeam=$1 \
                          --simDurationInSeconds=$2 \
                          --ns3::SatStatsHelperContainer::Name=$name\"" \
           > "run-$name.sh"
    chmod +x "run-$name.sh"

    qsub -m eas -M $USER@magister.fi -j yes -b yes -cwd /bin/bash "run-$name.sh"
}

main()
{
    uts="10 20"
    simTime="10 20"
    for i in $uts
    do
        for j in $simTime
        do
	   # uts simTime
           run_sim $i $j
           run_sim $i $j
        done
    done
}

main $*


