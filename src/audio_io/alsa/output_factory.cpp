#include "alsa.hpp"
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <string>
#include <vector>

namespace audio_io {
namespace implementation {

std::string AlsaOutputDeviceFactory::getName() {
	return "alsa";
}

std::vector<std::string> AlsaOutputDeviceFactory::getOutputNames() {
	return device_names;
}

std::vector<int> AlsaOutputDeviceFactory::getOutputMaxChannels() {
	return device_channels;
}

std::unique_ptr<OutputDevice> AlsaOutputDeviceFactory::createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) {
	return nullptr;
}

OutputDeviceFactory* createAlsaOutputDeviceFactory() {
	return new AlsaOutputDeviceFactory();
}

}
}