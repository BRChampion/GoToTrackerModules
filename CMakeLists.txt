cmake_minimum_required(VERSION 3.30)
project(GoToTrackerModules)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(GoToTracker
        src/Clock.h
        src/TrackerTypes.h
        src/Driver.h
        src/SkyMath.h
        src/SkyMath.cpp
        src/MountModel.h
        src/MountModel.cpp
        src/AltAzTracker.h
        src/AltAzTracker.cpp
        src/CommandInterface.h
        src/CommandInterface.cpp
        src/AxisController.h
        src/AxisController.cpp
        src/TargetCatalog.h
        src/TargetCatalog.cpp
        src/GoToTracker.h
        src/ArduinoAdapters.h
)

target_include_directories(GoToTracker PUBLIC src)

add_executable(GoToTrackerModules
        main.cpp
        core/SimClock.h
        core/FakeDriver.h
)

target_link_libraries(GoToTrackerModules PRIVATE GoToTracker)

# Compile the public Arduino surface against a small host-side API stub. This
# catches accidental desktop-only dependencies even when arduino-cli is absent.
add_library(ArduinoCompileCheck OBJECT
        tests/ArduinoCompileCheck.cpp
        src/AxisController.cpp
        src/MountModel.cpp
        src/SkyMath.cpp
        src/TargetCatalog.cpp
        src/AltAzTracker.cpp
        src/CommandInterface.cpp
)
target_compile_definitions(ArduinoCompileCheck PRIVATE ARDUINO=10800)
target_include_directories(ArduinoCompileCheck PRIVATE tests/arduino_stubs src)

add_executable(SilTests
        tests/SilTests.cpp
        core/SimClock.h
        core/FakeDriver.h
)
target_include_directories(SilTests PRIVATE core)
target_link_libraries(SilTests PRIVATE GoToTracker)

add_executable(ShortestPathDemo
        tests/ShortestPathDemo.cpp
        core/SimClock.h
        core/FakeDriver.h
)
target_include_directories(ShortestPathDemo PRIVATE core)
target_link_libraries(ShortestPathDemo PRIVATE GoToTracker)

enable_testing()
add_test(NAME desktop_sil
         COMMAND SilTests
                 ${CMAKE_SOURCE_DIR}/sil-data/desktop_sil_trace.csv
                 ${CMAKE_SOURCE_DIR}/sil-data/desktop_sil_summary.csv
                 ${CMAKE_SOURCE_DIR}/sil-data/desktop_sil_comparisons.csv)
