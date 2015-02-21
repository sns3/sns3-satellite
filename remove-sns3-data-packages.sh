#!/bin/bash

##
# Remove any SNS3 data files from the current SNS3 instance.
#
# The files to be removed include all data files created and/or modified by
# the users.
#
# This script must be executed from the root of the SNS3 satellite module
# (i.e., contrib/satellite).
#
# Author: Frans Laakso <frans.laakso@magister.fi>
#

removeSnsDataPackage () {
    rm --verbose --recursive --force data/antennapatterns
    rm --verbose --recursive --force data/linkresults
    rm --verbose --recursive --force data/utpositions
    rm --verbose --recursive --force data/ext-fadingtraces
    rm --verbose --recursive --force data/fadingtraces
    rm --verbose --recursive --force data/rxpowertraces
    rm --verbose --recursive --force data/interferencetraces
    rm --verbose --recursive --force data/sinrmeaserror
}

if [ -d data ]
then
  echo "Satellite data folder found."
else
  echo "This script uses relative paths. Please run it from 'contrib/satellite' folder"
  exit 1
fi

echo "This script will remove _ALL_ SNS3 data package folders" \
  "regardless of the contents."

while true; do
    read -p "Are you absolutely sure you wish to proceed (y/n)? " yn
    case $yn in
        [Yy]* ) removeSnsDataPackage; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

unset removeSnsDataPackage

echo "--- Finished ---"
