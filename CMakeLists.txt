cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)
project(MiniMetersServer VERSION 1.0.8)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(JUCE)
add_subdirectory(clap-juce-extensions EXCLUDE_FROM_ALL)

juce_add_plugin(MiniMetersServer
        COMPANY_NAME Direct
        COMPANY_WEBSITE https://minimeters.app/
        COMPANY_EMAIL contact@directmusic.me
        BUNDLE_ID com.josephlyncheski.MiniMetersServer
        VST3_CATEGORIES Fx Tools
        AU_MAIN_TYPE kAudioUnitType_Effect
        IS_MIDI_EFFECT FALSE
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Jldr
        PLUGIN_CODE mmsv
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "MiniMetersServer")

clap_juce_extensions_plugin(TARGET MiniMetersServer
        CLAP_ID "com.josephlyncheski.MiniMetersServer"
        CLAP_FEATURES effect utility)

juce_generate_juce_header(MiniMetersServer)

# `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.
if(APPLE)
        target_sources(MiniMetersServer
                PRIVATE
                ma_implementation_file.cpp
                MiniMetersOpener.cpp
                PluginEditor.cpp
                MacOsHelpers.mm
                PluginProcessor.cpp)
else()
        target_sources(MiniMetersServer
                PRIVATE
                ma_implementation_file.cpp
                MiniMetersOpener.cpp
                PluginEditor.cpp
                PluginProcessor.cpp)
endif()

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.
target_compile_definitions(MiniMetersServer
        PUBLIC

        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0 # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0 # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_DISPLAY_SPLASH_SCREEN=0
        REPORT_JUCE_APP_USAGE=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.
juce_add_binary_data(AudioPluginData SOURCES "Assets/SpaceMono-Regular.ttf" "Assets/bg.png")

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_audio_utils` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.
target_link_libraries(MiniMetersServer
        PRIVATE

        AudioPluginData # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
