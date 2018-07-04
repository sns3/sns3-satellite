#!/bin/bash

# Copyright (c) 2013 Magister Solutions Ltd
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
# Author: Frans Laakso <frans.laakso@magister.fi>
#

##
# Create a new compressed archive from the currently active SNS3 data files.
# The archive will be saved in the current working directory using the name
# "sns3-data-package.tar.xz".
#
# The resulting archive can then be distributed, for example to be hosted in
# the Internet.
#
# This script must be executed from the root of the SNS3 satellite module
# (i.e., contrib/satellite).
#

archive_name="sns3-data-package.tar.xz"

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
