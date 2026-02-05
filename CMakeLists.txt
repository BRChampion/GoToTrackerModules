cmake_minimum_required(VERSION 3.30)
project(GoToTrackerModules)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(GoToTrackerModules main.cpp
        core/Clock.h
        core/SimClock.h
        core/Driver.h
        core/FakeDriver.h
        core/SkyMath.h
        core/SkyMath.cpp
        core/MountModel.h
        core/MountModel.cpp
        core/AxisController.h
        core/AxisController.cpp
)
