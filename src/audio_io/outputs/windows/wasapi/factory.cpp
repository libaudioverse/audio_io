#include "wasapi.hpp"
#include <audio_io/private/audio_outputs.hpp>
#include <windows.h>

namespace audio_io {
namespace implementation {

WasapiOutputDeviceFactory::WasapiOutputDeviceFactory() {
	rescan();
}

WasapiOutputDeviceFactory::~WasapiOutputDeviceFactory() {
}

std::vector<std::string> WasapiOutputDeviceFactory::getOutputNames() {
	return names;
}

std::vector<int> WasapiOutputDeviceFactory::getOutputMaxChannels() {
	return max_channels;
}

std::shared_ptr<OutputDevice> WasapiOutputDeviceFactory::createDevice(std::function<void(float*, int)> callback, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) {
	return nullptr;
}

unsigned int WasapiOutputDeviceFactory::getOutputCount() {
	return 0;
}

std::string WasapiOutputDeviceFactory::getName() {
	return "Wasapi";
}

void WasapiOutputDeviceFactory::rescan() {
}


OutputDeviceFactory* createWasapiOutputDeviceFactory() {
	return nullptr;
}

}
}