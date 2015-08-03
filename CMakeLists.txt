CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(audio_io)

OPTION(AUDIO_IO_BUILD_TESTS "Whether to build tests from audio_io" ON)
OPTION(AUDIO_IO_HANDLED_DEPENDENCIES "Whether or not a parent script handles our dependencies.  Don't touch this, it's for Libaudioverse." OFF)

if(${WIN32})
add_definitions(-DAUDIO_IO_WINDOWS_BACKENDS)
set(AUDIO_IO_NEEDED_LIBRARIES winmm)
endif()

if(NOT ${AUDIO_IO_HANDLED_DEPENDENCIES})
message("Building audio_io in standalone configuration.")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/speex_resampler_cpp/include")
add_subdirectory(speex_resampler_cpp)
endif()

SET(AUDIO_IO_NEEDED_LIBRARIES ${AUDIO_IO_NEEDED_LIBRARIES} speex_resampler_cpp)

SET(AUDIO_IO_FILES
src/audio_io/outputs/common_output.cpp
src/audio_io/outputs/winmm_output.cpp
src/audio_io/outputs/factory_creation.cpp
src/audio_io/format_conversion/mixing_matrices.cpp
src/audio_io/format_conversion/remix.cpp
src/audio_io/format_conversion/sample_format_converter.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(audio_io STATIC
${AUDIO_IO_FILES}
)
target_link_libraries(audio_io ${AUDIO_IO_NEEDED_LIBRARIES})

if(${AUDIO_IO_BUILD_TESTS})
add_executable(output
src/tests/output.cpp
)
target_link_libraries(output audio_io)

endif()
