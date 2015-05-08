SET(AUDIO_IO_FILES
src/audio_io/resampler.cpp
src/audio_io/speex_resampler.c
src/audio_io/common_output.cpp
src/audio_io/winmm_output.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(audio_io STATIC
${AUDIO_IO_FILES}
)
