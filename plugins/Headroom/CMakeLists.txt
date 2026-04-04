# Headroom: Hard clipper with threshold control

juce_add_plugin(Headroom
    COMPANY_NAME             "CorvidAudio"
    PLUGIN_MANUFACTURER_CODE CVDA
    PLUGIN_CODE              HDRM
    PLUGIN_VERSION           "1.0.0"
    FORMATS                  AU VST3 Standalone
    PRODUCT_NAME             "Headroom"
    DESCRIPTION              "Hard clipper with threshold control"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    AU_MAIN_TYPE             kAudioUnitType_Effect
    AU_EXPORT_PREFIX         HeadroomAU
    BUNDLE_IDENTIFIER        com.CorvidAudio.Headroom
    COPY_PLUGIN_AFTER_BUILD  FALSE
)

juce_generate_juce_header(Headroom)

target_sources(Headroom PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

target_compile_definitions(Headroom PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Headroom PRIVATE
    CorvidShared
)

target_link_libraries(Headroom PUBLIC
    juce::juce_audio_formats
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_plugin_client
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce_recommended_config_flags
    juce_recommended_lto_flags
    juce_recommended_warning_flags
)
