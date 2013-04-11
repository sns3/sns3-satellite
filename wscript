# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('satellite', ['internet'])
    module.source = [
        'model/satellite.cc',
        'model/cbr-application.cc',
        'model/sat-channel.cc',
        'model/sat-net-device.cc',       
        'helper/satellite-helper.cc',
        'helper/sat-net-dev-helper.cc',
        'helper/cbr-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('satellite')
    module_test.source = [
        'test/satellite-test-suite.cc',
        'test/cbr-test.cc',
        ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'satellite'
    headers.source = [
        'model/satellite.h',
        'model/cbr-application.h',
        'model/sat-channel.h',
        'model/sat-net-device.h',
        'helper/satellite-helper.h',
        'helper/sat-net-dev-helper.h',
        'helper/cbr-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')

    # bld.ns3_python_bindings()

