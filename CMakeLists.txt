cmake_minimum_required(VERSION 3.22)
project(WavCultStrip VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use JUCE as a git submodule (in JUCE/ folder), or fall back to ../JUCE for local dev
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt)
    add_subdirectory(JUCE)
else()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
endif()

juce_add_plugin(WavCultStrip
    COMPANY_NAME "WavCult"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Wvcl
    PLUGIN_CODE Wcst
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "WavCult Strip"
)

target_sources(WavCultStrip PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/NoiseGate.cpp
    Source/DSP/SSLChannelEQ.cpp
    Source/DSP/CLACompressor.cpp
    Source/DSP/Saturation.cpp
    Source/DSP/ReverbEngine.cpp
    Source/DSP/DelayEngine.cpp
    Source/DSP/BusCompressor.cpp
)

juce_add_binary_data(WavCultStripData SOURCES
    Source/Resources/wavcult_logo.png
)

juce_generate_juce_header(WavCultStrip)

target_include_directories(WavCultStrip PRIVATE Source)

target_compile_definitions(WavCultStrip PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(WavCultStrip
    PRIVATE
        WavCultStripData
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
