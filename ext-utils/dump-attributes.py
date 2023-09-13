import argparse
import subprocess
from pathlib import Path


CWD = Path(__file__).resolve().parent


def gather_class_names(folder):
    process = subprocess.run(
            """grep -rh '^[^ ]*::GetTypeId(' | awk -F"::" '{ print $1 }'""",
            shell=True, text=True, capture_output=True, cwd=folder)
    return process.stdout.splitlines()


def gather_all_class_names():
    for folder in ('model', 'stats', 'utils', 'helper'):
        yield from gather_class_names(CWD.parent / folder)


def dump_attributes(class_name, sns3_folder):
    with sns3_folder.joinpath('scratch', 'attributes-dump.cc').open('w', encoding='utf-8') as f:
        print("""\
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2023 CNES
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
 *
 */

#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/satellite-module.h"

#include <algorithm>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("attributes-dump.cc");

const std::string nameHeader = "Name of the attribute";
const std::string helpHeader = "Description";

void
fillWithBlanks(std::size_t length, char blank = ' ')
{
    while (length)
    {
        std::cout << blank;
        --length;
    }
}

void
dumpTypeId(TypeId &tid)
{
    const std::size_t tidNameLength = tid.GetName().size();
    std::size_t nameLength = nameHeader.size();
    std::size_t helpLength = helpHeader.size();
    for (std::size_t i = 0; i < tid.GetAttributeN(); ++i)
    {
        auto attribute = tid.GetAttribute(i);
        // tid name and attribute name are separated using 2 colons
        nameLength = std::max(nameLength, attribute.name.size() + tidNameLength + 2);
        helpLength = std::max(helpLength, attribute.help.size());
    }

    fillWithBlanks(nameLength, '=');
    std::cout << "   ";
    fillWithBlanks(helpLength, '=');
    std::cout << "   ===\\n" << nameHeader;
    fillWithBlanks(nameLength - nameHeader.size());
    std::cout << "   " << helpHeader;
    fillWithBlanks(helpLength - helpHeader.size());
    std::cout << "   Default value\\n";
    fillWithBlanks(nameLength, '=');
    std::cout << "   ";
    fillWithBlanks(helpLength, '=');
    std::cout << "   ===\\n";

    for (std::size_t i = 0; i < tid.GetAttributeN(); ++i)
    {
        auto attribute = tid.GetAttribute(i);
        auto value = attribute.initialValue;
        auto name = tid.GetAttributeFullName(i);
        std::cout << name;
        fillWithBlanks(nameLength - name.size());
        std::cout << "   " << attribute.help;
        fillWithBlanks(helpLength - attribute.help.size());
        std::cout << "   " << value->SerializeToString(attribute.checker) << std::endl;
    }

    fillWithBlanks(nameLength, '=');
    std::cout << "   ";
    fillWithBlanks(helpLength, '=');
    std::cout << "   ===\\n";
}

int
main(int argc, char* argv[])
{""", file=f)
        print("    TypeId tid = ", class_name, "::GetTypeId();", sep='', file=f)
        print("    dumpTypeId(tid);\n}", file=f, flush=True)
    subprocess.run(['./ns3', 'configure', '-d', 'release', '--enable-modules', 'satellite'], check=True, cwd=sns3_folder)
    result = subprocess.run(['./ns3', 'run', 'attributes-dump'], check=True, text=True, capture_output=True, cwd=sns3_folder)
    return result.stdout


def command_line_parser(classes):
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(title='action')
    parser_list = subparsers.add_parser('list')

    parser_dump = subparsers.add_parser('dump')
    parser_dump.add_argument('class_name', metavar='NAME', choices=classes)
    parser_dump.add_argument('--sns3-path', '--sns3', '--path', '-p', type=Path, default=CWD.parent.parent.parent)

    return parser


if __name__ == '__main__':
    classes = list(gather_all_class_names())
    arguments = command_line_parser(classes).parse_args()
    class_name = getattr(arguments, 'class_name', None)
    if class_name is None:
        print('\n'.join(classes))
    else:
        print(dump_attributes(class_name, arguments.sns3_path))
