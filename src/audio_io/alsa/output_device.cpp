#include "alsa.hpp"
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <vector>
#include <string>
#include <functional>
#include <alsa/asoundlib.h>

namespace audio_io {
namespace implementation {

AlsaOutputDevice::AlsaOutputDevice(std::function<void(float*, int)> callback, std::string name, int sr, int channels, int blockSize, float minLatency, float startLatency, float maxLatency) {
}

void AlsaOutputDevice::stop() {
}

void AlsaOutputDevice::workerThreadFunction() {
}

}
}