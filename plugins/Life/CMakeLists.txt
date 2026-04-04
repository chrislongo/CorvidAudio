# Life: Analog character and warmth effect

juce_add_plugin(Life
    COMPANY_NAME             "CorvidAudio"
    PLUGIN_MANUFACTURER_CODE CVDA
    PLUGIN_CODE              LIFE
    PLUGIN_VERSION           "0.5.0"
    FORMATS                  AU VST3 Standalone
    PRODUCT_NAME             "Life"
    DESCRIPTION              "Analog life and character for digital tracks"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    AU_MAIN_TYPE             kAudioUnitType_Effect
    AU_EXPORT_PREFIX         LifeAU
    BUNDLE_IDENTIFIER        com.CorvidAudio.Life
    COPY_PLUGIN_AFTER_BUILD  FALSE
)

juce_generate_juce_header(Life)

target_sources(Life PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

juce_add_binary_data(LifeBinaryData SOURCES
    resource/Noise.aif
)

target_compile_definitions(Life PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Life PRIVATE
    LifeBinaryData
    CorvidShared
)

target_link_libraries(Life PUBLIC
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
