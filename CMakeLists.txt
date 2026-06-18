cmake_minimum_required(VERSION 3.22)

project(StageMind VERSION 0.9.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.13
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(StageMindNode
    COMPANY_NAME "StageMind"
    PRODUCT_NAME "StageMind Node"
    BUNDLE_ID "com.stagemind.node"
    PLUGIN_MANUFACTURER_CODE StMn
    PLUGIN_CODE SmNd
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(StageMindNode)

target_sources(StageMindNode
    PRIVATE
        Source/Plugin/PluginProcessor.cpp
        Source/Plugin/PluginEditor.cpp
        Source/Link/LinkActivityEnvelope.cpp
        Source/Link/StageMindLinkRegistry.cpp
        Source/Link/LinkSuggestionEngine.cpp
        Source/Model/Parameters.cpp
        Source/Model/PluginState.cpp
        Source/Model/RolePresetEngine.cpp
        Source/DSP/CleanUpProcessor.cpp
        Source/DSP/CorrelationMeter.cpp
        Source/DSP/DepthProcessor.cpp
        Source/DSP/DynamicEQ.cpp
        Source/DSP/LinkSpectralAnalyzer.cpp
        Source/DSP/LowMonoProcessor.cpp
        Source/DSP/MidSideProcessor.cpp
        Source/DSP/MotionProcessor.cpp
        Source/DSP/PseudoDoubleProcessor.cpp
        Source/DSP/ResonanceDetector.cpp
        Source/DSP/ResonanceLearner.cpp
        Source/DSP/SidechainDetector.cpp
        Source/DSP/SidechainDynamicEQ.cpp
        Source/DSP/SideHighPassProcessor.cpp
        Source/DSP/SpatialProcessor.cpp
        Source/UI/DirectorSceneView.cpp
        Source/UI/HardwareLookAndFeel.cpp
        Source/UI/MeterView.cpp
        Source/UI/ResonanceListView.cpp
        Source/UI/StageView.cpp
)

target_compile_definitions(StageMindNode
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(StageMindNode
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

add_executable(StageMindDSPTests
    Tests/LinkActivityEnvelopeTests.cpp
    Tests/LinkSpectralAnalyzerTests.cpp
    Tests/LinkSuggestionEngineTests.cpp
    Tests/ResonanceDetectorTests.cpp
    Tests/RideMemoryTests.cpp
    Tests/RideTimelineMemoryTests.cpp
    Tests/StageMindLinkRegistryTests.cpp
    Tests/SpatialEnhancementTests.cpp
    Source/Link/LinkActivityEnvelope.cpp
    Source/Link/StageMindLinkRegistry.cpp
    Source/Link/LinkSuggestionEngine.cpp
    Source/DSP/CleanUpProcessor.cpp
    Source/DSP/DepthProcessor.cpp
    Source/DSP/DynamicEQ.cpp
    Source/DSP/LinkSpectralAnalyzer.cpp
    Source/DSP/MotionProcessor.cpp
    Source/DSP/PseudoDoubleProcessor.cpp
    Source/DSP/ResonanceDetector.cpp
    Source/DSP/ResonanceLearner.cpp
)

target_include_directories(StageMindDSPTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(StageMindDSPTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(StageMindDSPTests
    PRIVATE
        juce::juce_audio_basics
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
