#! /usr/bin/env python
# -*- coding: utf-8 -*-
## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# A list of C++ examples to run in order to ensure that they remain
# buildable and runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run, do_valgrind_run).
#
# See test.py for more information.
cpp_examples = [
    ("sat-cbr-example", "True", "True"),
    ("simple-scenario-p2p", "True", "True"),
    ("larger-scenario-p2p", "True", "True"),
    ("sat-http-example", "True", "True"),
    ("sat-nrtv-example", "True", "True"),
    ("sat-onoff-example", "True", "True"),
    ("sat-link-budget-example", "True", "True")
]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []
