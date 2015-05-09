CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

project(audio_io)

OPTION(AUDIO_IO_BUILD_TESTS "Whether to build tests from audio_io" ON)

if(${WIN32})
add_definitions(-DAUDIO_IO_WINDOWS_BACKENDS)
set(AUDIO_IO_NEEDED_LIBRARIES winmm)
endif()

SET(AUDIO_IO_FILES
src/audio_io/resampler.cpp
src/audio_io/speex_resampler.c
src/audio_io/common_output.cpp
src/audio_io/winmm_output.cpp
src/audio_io/factory_creation.cpp
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
