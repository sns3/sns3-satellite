#!/bin/bash

##
# Download and extract an archive of SNS3 data files from Magister server.
#
# The SNS3 data files are not included when users download the SNS3 source code
# from the repository. They are available as a separate download, which is
# handled by this script. The data files are typically approximately 1 GB in
# size.
#
# This script must be executed from the root of the SNS3 satellite module
# (i.e., contrib/satellite).
#
# Author: Frans Laakso <frans.laakso@magister.fi>
#

archive_name="sns3-data-default-latest.tar.xz"
direct_link="https://docs.google.com/a/magister.fi/uc?id=0Bx03Cn0AzYGGaDlFVnJHWDdwX3M"

if [ -d data ]
then
  echo "Satellite data folder found, checking for existing data packages..."
else
  echo "This script uses relative paths. Please run it from 'contrib/satellite' folder"
  exit 1
fi

if [ -d data/antennapatterns \
  -o -d data/linkresults \
  -o -d data/utpositions \
  -o -d data/ext-fadingtraces \
  -o -d data/fadingtraces \
  -o -d data/rxpowertraces \
  -o -d data/interferencetraces \
  -o -d data/sinrmeaserror ]
then
  echo "This SNS3 instance has an existing set of SNS3 data files."
  echo "All existing data packages can be removed with remove-sns3-data-packages.sh script."
  
  while true; do
    read -p "Do you wish to proceed with the installation of '$archive_name' data package (y/n)? " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
  done
fi

echo "Attempting to download the data file"
perl utils/gdown.pl $direct_link $archive_name || exit $?

echo "Download complete, extracting the data file"
tar --verbose --extract --xz --file=$archive_name || exit $?

echo "Removing the downloaded archive file"
rm --verbose --force $archive_name

echo "Removing the cookies"
rm --verbose --force cookie.txt

echo "--- Finished ---"
