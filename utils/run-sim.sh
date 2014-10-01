#!/bin/bash

####################################################
# Example simulation script for pushing simulations 
# to grid engine. Run the script in the NS-3 root.
# Author: Jani Puttonen
# Date: 1.10.2014
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


