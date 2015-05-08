//This file defines the magic macros for speex and then includes the headers.
//The purpose of this is to prevent us from having to pollute CMakeLists.txt, as we want a CMakeLists.txt that can safely be vendored.
#define OUTSIDE_SPEEX
#define RANDOM_PREFIX audio_io
#define FLOATING_POINT
#include "speex_arch.h"
#include "speex_stack_alloc.h"
#include "speex_resampler.h"
