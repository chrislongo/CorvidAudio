# Dist308: ProCo Rat-inspired distortion

juce_add_plugin(Dist308
    COMPANY_NAME             "CorvidAudio"
    PLUGIN_MANUFACTURER_CODE CVDA
    PLUGIN_CODE              D308
    PLUGIN_VERSION           "1.3.0"
    FORMATS                  AU VST3 Standalone
    PRODUCT_NAME             "Dist308"
    DESCRIPTION              "ProCo Rat-inspired distortion"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    AU_MAIN_TYPE             kAudioUnitType_Effect
    AU_EXPORT_PREFIX         Dist308AU
    BUNDLE_IDENTIFIER        com.CorvidAudio.Dist308
    COPY_PLUGIN_AFTER_BUILD  FALSE
)

juce_generate_juce_header(Dist308)

target_sources(Dist308 PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

target_compile_definitions(Dist308 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Dist308 PUBLIC
    CorvidShared
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
