set(base_examples
    sat-arq-fwd-example
    sat-arq-rtn-example
    sat-cbr-example
    sat-cbr-full-example
    sat-cbr-stats-example
    sat-cbr-user-defined-example
    sat-dama-http-sim-tn9
    sat-dama-onoff-sim-tn9
    sat-dama-sim-tn9
    sat-dama-verification-sim
    sat-dynamic-frequency-plan-example
    sat-environmental-variables-example
    sat-essa-example
    sat-fwd-link-beam-hopping-example
    sat-fwd-system-test-example
    sat-generic-launcher
    sat-group-example
    sat-handover-example
    sat-http-example
    sat-iot-example
    sat-link-budget-example
    sat-link-results-plot
    sat-list-position-ext-fading-example
    sat-log-example
    sat-logon-example
    sat-loo-example
    sat-lora-example
    sat-markov-fading-trace-example
    sat-markov-logic-example
    sat-mobility-beam-tracer
    sat-mobility-example
    sat-mobility-position-generator
    sat-multi-application-fwd-example
    sat-multi-application-rtn-example
    sat-multicast-example
    sat-ncr-example
    sat-nrtv-example
    sat-onoff-example
    sat-per-packet-if-sim-tn9
    sat-profiling-sim
    sat-profiling-sim-tn8
    sat-random-access-crdsa-collision-example
    sat-random-access-crdsa-example
    sat-random-access-dynamic-load-control-example
    sat-random-access-example
    sat-random-access-slotted-aloha-collision-example
    sat-random-access-slotted-aloha-example
    sat-ra-sim-tn9
    sat-ra-sim-tn9-comparison
    sat-rayleigh-example
    sat-regeneration-collisions-example
    sat-regeneration-example
    sat-constellation-example
    sat-rtn-link-da-example
    sat-rtn-link-ra-example
    sat-rtn-system-test-example
    sat-trace-input-external-fading-example
    sat-trace-input-fading-example
    sat-trace-input-interference-example
    sat-trace-input-rx-power-example
    sat-trace-output-example
    sat-training-example
    sat-tutorial-example
    sat-vhts-example
)

foreach(
  example
  ${base_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK
    ${libsatellite}
  )
endforeach()
