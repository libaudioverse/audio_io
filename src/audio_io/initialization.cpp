#include <audio_io/audio_io.hpp>
#include <audio_io/private/logging.hpp>

namespace audio_io {

void initialize() {
	implementation::initializeLogger();
}

void shutdown() {
	implementation::shutdownLogger();
}

}