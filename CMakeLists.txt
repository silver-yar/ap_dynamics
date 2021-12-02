cmake_minimum_required(VERSION 3.15)

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the `project()` command.
# `project()` sets up some helpful variables that describe source/binary directories, and the current project version.
# This is a standard CMake command.

#SET(MACOSX_DEPLOYMENT_TARGET ${DARWIN_MAJOR_SDK_VERSION}.${DARWIN_MINOR_SDK_VERSION})
#SET(CMAKE_CXX_FLAGS "--sysroot ${CMAKE_OSX_SYSROOT} ${CMAKE_CXX_FLAGS}")
#MESSAGE("Setting MACOSX_DEPLOYMENT_TARGET to '${MACOSX_DEPLOYMENT_TARGET}'.")

project(ap_dynamics VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(dependencies)

# juce_set_vst2_sdk_path(...) juce_set_aax_sdk_path(...)

# `juce_add_plugin` adds a static library target with the name passed as the first argument (ap_dynamics here). This
# target is a normal CMake target, but has a lot of extra properties set up by default. As well as this shared code
# static library, this function adds targets for each of the formats specified by the FORMATS arguments. This function
# accepts many optional arguments. Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(
        ap_dynamics
        # VERSION ...                               # Set this if the plugin version is different to the project version
        ICON_BIG "Resources/apdicon.png"
        MICROPHONE_PERMISSION_ENABLED FALSE
        "Audio Pirate" # Specify the name of the plugin's author
        IS_SYNTH
        FALSE # Is this a synth or an effect?
        NEEDS_MIDI_INPUT
        FALSE # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT
        FALSE # Does the plugin need midi output?
        IS_MIDI_EFFECT
        FALSE # Is this plugin a MIDI effect?
        EDITOR_WANTS_KEYBOARD_FOCUS
        FALSE # Does the editor need keyboard focus?
        COPY_PLUGIN_AFTER_BUILD
        TRUE # Should the plugin be installed to a default location after building?
        PLUGIN_MANUFACTURER_CODE
        Aupi # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE
        Apdy # A unique four-character plugin id with exactly one upper-case character
        # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
        FORMATS
        AU
        AUv3
        VST3
        Standalone # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME
        "APDynamics") # The name of the final executable, which can differ from the target name

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated into your build
# tree. This should be included with `#include <JuceHeader.h>`. The include path for this header will be automatically
# added to the target. The main function of the JuceHeader is to include all your JUCE module headers; if you're happy
# to include module headers directly, you probably don't need to call this.

juce_generate_juce_header(ap_dynamics)

# `target_sources` adds source files to a target. We pass the target that needs the sources as the first argument, then
# a visibility parameter for the sources which should normally be PRIVATE. Finally, we supply a list of source files
# that will be built into the target. This is a standard CMake command.

target_sources(
        ap_dynamics
        PRIVATE DSP/APCompressor.cpp
        DSP/APOverdrive.cpp
        DSP/APTubeDistortion.cpp
        Helpers/APDefines.h
        Source/APSlider.cpp
        Source/MixerButton.cpp
        Source/OpenGL/SliderBarGL.cpp
        Source/PluginEditor.cpp
        Source/PluginProcessor.cpp)
# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer project, these might be
# passed in the 'Preprocessor Definitions' field. JUCE modules also make use of compile definitions to switch certain
# features on/off, so if there's a particular feature you need that's not on by default, check the module header for the
# correct flag to set here. These definitions will be visible both to your code, and also the JUCE module code, so for
# new definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(
        ap_dynamics
        PUBLIC # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0 # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0 # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:ap_dynamics,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:ap_dynamics,JUCE_VERSION>"
)

# If your target needs extra binary assets, you can add them here. The first argument is the name of a new static
# library target that will include all the binary resources. There is an optional `NAMESPACE` argument that can specify
# the namespace of the generated binary data class. Finally, the SOURCES argument should be followed by a list of source
# files that should be built into the static library. These source files can be of any kind (wav data, images, fonts,
# icons etc.). Conversion to binary-data will happen when your target is built.

juce_add_binary_data(
        AudioPluginData
        SOURCES
        Resources/VarelaRound.ttf
        Resources/blue_noise.png
        Resources/logo_clean.png
        Resources/shadow.png
)

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here, we're linking our
# executable target to the `juce::juce_audio_utils` module. Inter-module dependencies are resolved automatically, so
# `juce_core`, `juce_events` and so on will also be linked automatically. If we'd generated a binary data target above,
# we would need to link to it here too. This is a standard CMake command.

target_link_libraries(
        ap_dynamics
        PRIVATE AudioPluginData # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils juce::juce_opengl
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

add_library(project_warnings INTERFACE)
include(CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Testing
enable_testing()
list(
        APPEND
        FILES_tests
        Tests/tester.cpp
        DSP/APCompressor.cpp)
add_executable(catch-test ${FILES_tests})
add_test(Catch-Test catch-test)
target_link_libraries(catch-test
        PRIVATE
        Catch2::Catch2
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_graphics
        )

list(
        APPEND
        FILES_plot
        Tests/plot_test.cpp
        DSP/APCompressor.cpp
        DSP/APTubeDistortion.cpp
        DSP/APOverdrive.cpp)
add_executable(plot-test ${FILES_plot})
target_link_libraries(plot-test
        PRIVATE
        juce::juce_core
        juce::juce_graphics
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_dsp
        )


target_include_directories(catch-test PRIVATE DSP)
