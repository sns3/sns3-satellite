# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('satellite', ['internet', 'csma'])
    module.source = [
        'model/satellite.cc',
        'model/cbr-application.cc',
        'model/satellite-mac.cc',       
        'model/satellite-channel.cc',
        'model/satellite-net-device.cc',
        'model/satellite-phy.cc',
        'model/satellite-phy-tx.cc',               
        'model/satellite-phy-rx.cc',
        'model/satellite-signal-parameters.cc',
        'model/virtual-channel.cc',
        'helper/satellite-helper.cc',
        'helper/sat-net-dev-helper.cc',
        'helper/cbr-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('satellite')
    module_test.source = [
        'test/satellite-test-suite.cc',
        'test/simple-p2p.cc',
        'test/cbr-test.cc',
        ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'satellite'
    headers.source = [
        'model/satellite.h',
        'model/cbr-application.h',
        'model/satellite-mac.h',
        'model/satellite-channel.h',
        'model/satellite-net-device.h',
        'model/satellite-phy.h',
        'model/satellite-phy-tx.h',               
        'model/satellite-phy-rx.h',
        'model/satellite-signal-parameters.h',
        'model/virtual-channel.h',
        'helper/satellite-helper.h',
        'helper/sat-net-dev-helper.h',
        'helper/cbr-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')

    # bld.ns3_python_bindings()

