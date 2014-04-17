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
    ("simple-scenario-p2p", "True", "True"),
    ("larger-scenario-p2p", "True", "True"),
    ("sat-arq-fwd-example", "True", "True"),
    ("sat-arq-rtn-example", "True", "True"),
    ("sat-cbr-example", "True", "True"),
    ("sat-cbr-larger-example", "True", "True"),
    ("sat-cbr-user-defined-example", "True", "True"),
    ("sat-environmental-variables-example", "True", "True"),
    ("sat-http-example", "True", "True"),
    ("sat-link-budget-example", "True", "True"),
    ("sat-link-result-plot", "True", "True"),
    ("sat-loo-example", "True", "True"),
    ("sat-markov-fading-trace-example", "True", "True"),
    ("sat-markov-logic-example", "True", "True"),
    ("sat-multi-application-fwd-example", "True", "True"),
    ("sat-multi-application-rtn-example", "True", "True"),
    ("sat-multicast-example", "True", "True"),
    ("sat-nrtv-example", "True", "True"),
    ("sat-onoff-example", "True", "True"),
    ("sat-random-access-example", "True", "True"),
    ("sat-random-access-crdsa-collision-example", "True", "True"),
    ("sat-random-access-crdsa-example", "True", "True"),
    ("sat-random-access-dynamic-load-control-example", "True", "True"),
    ("sat-random-access-slotted-aloha-collision-example", "True", "True"),
    ("sat-random-access-slotted-aloha-example", "True", "True"),
    ("sat-rayleigh-example", "True", "True"),
]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []
