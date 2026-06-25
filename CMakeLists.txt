cmake_minimum_required(VERSION 3.15)
project(Detector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SQL Server / ADO persistence is Windows-only; enable by default on MSVC.
if(MSVC)
    option(DETECTOR_WITH_SQLSERVER "Build the SQL Server (ADO) sink" ON)
else()
    option(DETECTOR_WITH_SQLSERVER "Build the SQL Server (ADO) sink" OFF)
endif()

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# Portable core + the file sink.
set(SOURCES
    src/core/Config.cpp
    src/core/Log.cpp
    src/core/Time.cpp
    src/capture/FrameSource.cpp
    src/inference/DetectorNet.cpp
    src/persistence/FileSink.cpp
    src/pipeline/Pipeline.cpp
    src/pipeline/main.cpp
)

# Windows-only SQL Server backend.
if(DETECTOR_WITH_SQLSERVER)
    list(APPEND SOURCES
        src/persistence/AdoConnection.cpp
        src/persistence/SqlServerSink.cpp
    )
endif()

add_executable(Detector ${SOURCES})

target_include_directories(Detector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(Detector PRIVATE ${OpenCV_LIBS} Threads::Threads)

if(DETECTOR_WITH_SQLSERVER)
    target_compile_definitions(Detector PRIVATE DETECTOR_WITH_SQLSERVER)
endif()

# Copy the runtime configuration next to the binary.
add_custom_command(TARGET Detector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/Configuration.cfg
        $<TARGET_FILE_DIR:Detector>/Configuration.cfg
)
