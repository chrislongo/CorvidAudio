cmake_minimum_required(VERSION 3.22)
project(CorvidAudio VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)

# -- Dependencies --------------------------------------------------------------
set(JUCE_DIR "$ENV{HOME}/src/github/JUCE" CACHE PATH "Path to JUCE")
set(EURORACK_DIR "$ENV{HOME}/src/github/eurorack" CACHE PATH "Path to eurorack")

add_subdirectory(${JUCE_DIR} JUCE)

# -- Shared UI library ---------------------------------------------------------
add_subdirectory(shared)

# -- Plugins -------------------------------------------------------------------
add_subdirectory(plugins/2-OP)
add_subdirectory(plugins/Dist308)
add_subdirectory(plugins/Headroom)
add_subdirectory(plugins/Life)
add_subdirectory(plugins/Loc-Box)
