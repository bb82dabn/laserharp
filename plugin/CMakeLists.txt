cmake_minimum_required(VERSION 3.22)

project(LaserHarpPlugin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

set(JUCE_GIT_TAG "8.0.0" CACHE STRING "JUCE git tag or commit to fetch")
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG ${JUCE_GIT_TAG}
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(LaserHarpVST3
  COMPANY_NAME "Brian Bouchard"
  COMPANY_CODE BrBo
  PLUGIN_CODE LaHp
  FORMATS VST3
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD TRUE
  PRODUCT_NAME "LaserHarp"
  DESCRIPTION "Laser harp virtual instrument with mouse and MIDI beam control")

target_sources(LaserHarpVST3 PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)

target_compile_definitions(LaserHarpVST3 PRIVATE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(LaserHarpVST3 PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp)
