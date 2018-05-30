#!/bin/sh

# Copyright (c) 2018
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
# Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
#


check_uncrustify_exists () {
  which uncrustify > /dev/null 2>&1
  if [ $? -ne 0 ]
  then
    cat <<\EOF
Error: You need to install uncrustify before being
able to commit your changes to this repository.

Check http://uncrustify.sourceforge.net/ to get you started.
EOF
    return 1
  fi
  return 0
}


check_uncrustify_has_check_flag () {
  uncrustify --help | grep -e "--check" > /dev/null
  if [ $? -ne 0 ]
  then
    cat <<\EOF
Error: Your version of uncrustify does not support
the `--check` flag. Please update it to be able to
use the pre-commit hook.
EOF
    return 1
  fi
  return 0
}


create_files_list () {
  file_list=$1
  if [ -f ext-utils/.uncrustifyignore ]
  then
    find . -type f -name "*.h" -o -name "*.cc" | grep -v -f ext-utils/.uncrustifyignore > $file_list
  else
    find . -type f -name "*.h" -o -name "*.cc" > $file_list
  fi
}


check_source_code () {
  file_list=$(mktemp)
  create_files_list $file_list
  uncrustify -c ext-utils/uncrustify.cfg -L 1 --check -F $file_list
  if [ $? -ne 0 ]
  then
    echo -e "\nSome files does not conform to the coding style."
  else
    echo -e "\nAll files comply to the coding style."
  fi
  rm $file_list
  return 0
}


check_modified_source_code () {
  if git rev-parse --verify HEAD >/dev/null 2>&1
  then
    against=HEAD
  else
    # Initial commit: diff against an empty tree object
    against=4b825dc642cb6eb9a060e54bf8d69288fbee4904
  fi

  file_list=$(mktemp)
  git diff --cached --name-only --diff-filter=dt $against -- "examples/" "helper/" "model/" "stats/" "utils/" "test/*.cc" > $file_list
  uncrustify -c ext-utils/uncrustify.cfg -L 1 --check -F $file_list
  result=$?
  rm $file_list
  return $result
}


diff_source_code () {
  file_list=$(mktemp)
  diff_file=$(mktemp)
  create_files_list $file_list
  while read source_file
  do
    uncrustify -c ext-utils/uncrustify.cfg -q -o "$diff_file" -f "$source_file"
    case "$1" in
      git) git diff -- "$source_file" "$diff_file"
        ;;
      *) diff -U 3 "$source_file" "$diff_file"
        ;;
    esac
  done < $file_list
  rm $diff_file
  rm $file_list
  return 0
}


apply_source_code () {
  file_list=$(mktemp)
  create_files_list $file_list
  uncrustify -c ext-utils/uncrustify.cfg --replace --no-backup -F $file_list
  rm $file_list
  return 0
}


check_uncrustify_exists || exit 1


script_dir=$(dirname -- "$0")
cd "$script_dir/../"
action=$1

case $action in
  check) check_uncrustify_has_check_flag && check_source_code && exit 0
    exit 1
    ;;
  git-check) check_uncrustify_has_check_flag && check_modified_source_code && exit 0
    exit 1
    ;;
  diff) diff_source_code "$2" || exit 1
    ;;
  apply) apply_source_code || exit 1
    ;;
  *) cat <<\EOF
Usage: ./coding_style.sh action

Where action is either check, git-check, diff or apply.
EOF
    exit 1
    ;;
esac
