#!/bin/bash

##
# Create a new compressed archive from the currently active SNS3 data files.
# The archive will be saved in the current working directory using the name
# "sns3-data-default-latest.tar.xz".
#
# The resulting archive can then be distributed, for example to be hosted in
# the Internet.
#
# This script must be executed from the root of the SNS3 satellite module
# (i.e., contrib/satellite).
#
# Author: Frans Laakso <frans.laakso@magister.fi>
#

archive_name="sns3-data-default-latest.tar.xz"

if [ -d data ]
then
  echo "Satellite data folder found."
else
  echo "This script uses relative paths. Please run it from 'contrib/satellite' folder"
  exit 1
fi

echo "Removing any old versions of the file"
if [ -f $archive_name ]
then
  rm --verbose $archive_name
fi

echo "Compressing the data folders"
tar --verbose --create --xz --file=$archive_name \
  data/antennapatterns/    \
  data/linkresults/        \
  data/utpositions/        \
  data/ext-fadingtraces/   \
  data/fadingtraces/       \
  data/rxpowertraces/      \
  data/interferencetraces/ \
  data/sinrmeaserror/

echo "--- Finished ---"
