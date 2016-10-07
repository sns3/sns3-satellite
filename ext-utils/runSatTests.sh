#!/bin/bash

#!/bin/bash

# Copyright (c) 2016 Magister Solutions Ltd
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

# A script to easily run only the satellite module tests
# 

cd ../../../;

./test.py -s sat-antenna-gain-pattern-test --fullness=TAKES_FOREVER
./test.py -s sat-arq-seqno-test --fullness=TAKES_FOREVER
./test.py -s sat-arq-test --fullness=TAKES_FOREVER
./test.py -s sat-channel-estimation-error-test --fullness=TAKES_FOREVER
./test.py -s sat-cno-estimator-unit-test --fullness=TAKES_FOREVER
./test.py -s sat-cra-test --fullness=TAKES_FOREVER
./test.py -s sat-ctrl-msg-container-unit-test --fullness=TAKES_FOREVER
./test.py -s sat-fading-external-input-trace-test --fullness=TAKES_FOREVER
./test.py -s sat-frame-allocator-test --fullness=TAKES_FOREVER
./test.py -s sat-fsl-test --fullness=TAKES_FOREVER
./test.py -s geo-coordinate-test --fullness=TAKES_FOREVER
./test.py -s sat-gse-test --fullness=TAKES_FOREVER
./test.py -s sat-if-unit-test --fullness=TAKES_FOREVER
./test.py -s sat-link-results-test --fullness=TAKES_FOREVER
./test.py -s sat-mobility-observer-test --fullness=TAKES_FOREVER
./test.py -s sat-perf-mem --fullness=TAKES_FOREVER
./test.py -s sat-periodic-control-message-test --fullness=TAKES_FOREVER
./test.py -s sat-per-packet-if-test --fullness=TAKES_FOREVER
./test.py -s sat-random-access-test --fullness=TAKES_FOREVER
./test.py -s sat-mobility-test --fullness=TAKES_FOREVER
./test.py -s sat-rle-test --fullness=TAKES_FOREVER
./test.py -s sat-rm-test --fullness=TAKES_FOREVER
./test.py -s sat-waveform-conf-test --fullness=TAKES_FOREVER
./test.py -s sat-scenario-creation --fullness=TAKES_FOREVER
./test.py -s sat-simple-unicast --fullness=TAKES_FOREVER

