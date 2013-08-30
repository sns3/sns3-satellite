# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('satellite', ['internet', 'csma', 'propagation'])
    module.source = [
        'model/geo-coordinate.cc',
        'model/cbr-application.cc',
        'model/satellite-arp-cache.cc',
        'model/satellite-mac.cc',
        'model/satellite-channel.cc',
        'model/satellite-net-device.cc',
        'model/ideal-net-device.cc',
        'model/satellite-geo-net-device.cc',
        'model/satellite-phy.cc',
        'model/satellite-phy-tx.cc',               
        'model/satellite-phy-rx.cc',
        'model/satellite-phy-rx-carrier.cc',
        'model/satellite-phy-rx-carrier-conf.cc',
        'model/satellite-link-results.cc',
        'model/satellite-look-up-table.cc',
        'model/satellite-signal-parameters.cc',
        'model/virtual-channel.cc',
        'model/satellite-beam-scheduler.cc',
        'model/satellite-control-header.cc',        
        'model/satellite-ncc.cc',
        'model/satellite-ut-mac.cc',
        'model/satellite-mobility-model.cc',
        'model/satellite-constant-position-mobility-model.cc',
        'model/satellite-position-allocator.cc',
        'model/satellite-propagation-delay-model.cc',
        'model/satellite-interference.cc',
        'model/satellite-per-packet-interference.cc',
        'model/satellite-constant-interference.cc',
        'model/satellite-traced-interference.cc',
        'model/satellite-free-space-loss.cc',
        'helper/satellite-helper.cc',
        'helper/satellite-beam-helper.cc',
        'helper/satellite-user-helper.cc',
        'helper/satellite-ut-helper.cc',
        'helper/satellite-gw-helper.cc',
        'helper/satellite-geo-helper.cc',
        'helper/satellite-conf.cc',
        'helper/satellite-beam-user-info.cc',
        'helper/cbr-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('satellite')
    module_test.source = [
        'test/geo-coordinate-test.cc',
        'test/performance-memory-test.cc',
        'test/cbr-test.cc',
        'test/link-results-test.cc',
        'test/satellite-simple-p2p.cc',
        'test/satellite-scenario-creation.cc',
        'test/satellite-mobility-test.cc',
        'test/satellite-interference-test.cc',
        'test/satellite-fsl-test.cc',
        ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'satellite'
    headers.source = [
        'model/geo-coordinate.h',
        'model/cbr-application.h',
        'model/satellite-arp-cache.h',
        'model/satellite-mac.h',
        'model/satellite-channel.h',
        'model/satellite-net-device.h',
        'model/ideal-net-device.h',
        'model/satellite-geo-net-device.h',
        'model/satellite-phy.h',
        'model/satellite-phy-tx.h',               
        'model/satellite-phy-rx.h',
        'model/satellite-phy-rx-carrier.h',
		'model/satellite-phy-rx-carrier-conf.h',
        'model/satellite-link-results.h',
        'model/satellite-look-up-table.h',
        'model/satellite-signal-parameters.h',
        'model/virtual-channel.h',
        'model/satellite-beam-scheduler.h',
        'model/satellite-control-header.h',        
        'model/satellite-ncc.h',
        'model/satellite-ut-mac.h',
        'model/satellite-mobility-model.h',
        'model/satellite-constant-position-mobility-model.h',
        'model/satellite-position-allocator.h',
        'model/satellite-propagation-delay-model.h',
        'model/satellite-interference.h',
        'model/satellite-per-packet-interference.h',
        'model/satellite-constant-interference.h',
        'model/satellite-traced-interference.h',
        'model/satellite-free-space-loss.h',       
        'helper/satellite-helper.h',
        'helper/satellite-beam-helper.h',
		'helper/satellite-user-helper.h',
		'helper/satellite-ut-helper.h',
        'helper/satellite-gw-helper.h',
        'helper/satellite-geo-helper.h',
        'helper/satellite-conf.h',
        'helper/satellite-beam-user-info.h',
        'helper/cbr-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')

    # bld.ns3_python_bindings()

