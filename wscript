# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('satellite', ['internet', 'csma'])
    module.source = [
        'model/satellite.cc',
        'model/geo-coordinate.cc',        
        'model/cbr-application.cc',
        'model/satellite-mac.cc',       
        'model/satellite-channel.cc',
        'model/satellite-net-device.cc',
        'model/satellite-geo-net-device.cc',
        'model/satellite-phy.cc',
        'model/satellite-phy-tx.cc',               
        'model/satellite-phy-rx.cc',
        'model/satellite-signal-parameters.cc',
        'model/virtual-channel.cc',
        'helper/satellite-helper.cc',
        'helper/satellite-beam-helper.cc',
        'helper/satellite-user-helper.cc',
        'helper/satellite-ut-helper.cc',
        'helper/satellite-gw-helper.cc',
        'helper/satellite-geo-helper.cc',
        'helper/satellite-conf.cc',
        'helper/cbr-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('satellite')
    module_test.source = [
        'test/satellite-test-suite.cc',
        'test/geo-coordinate-test.cc',
        'test/simple-p2p.cc',
        'test/cbr-test.cc',
        ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'satellite'
    headers.source = [
        'model/satellite.h',
        'model/geo-coordinate.h',
        'model/cbr-application.h',
        'model/satellite-mac.h',
        'model/satellite-channel.h',
        'model/satellite-net-device.h',
        'model/satellite-geo-net-device.h',
        'model/satellite-phy.h',
        'model/satellite-phy-tx.h',               
        'model/satellite-phy-rx.h',
        'model/satellite-signal-parameters.h',
        'model/virtual-channel.h',
        'helper/satellite-helper.h',
        'helper/satellite-beam-helper.h',
		'helper/satellite-user-helper.h',
		'helper/satellite-ut-helper.h',
        'helper/satellite-gw-helper.h',
        'helper/satellite-geo-helper.h',
        'helper/satellite-conf.h',
        'helper/cbr-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')

    # bld.ns3_python_bindings()

