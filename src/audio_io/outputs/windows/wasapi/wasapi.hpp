/**This header is out-of-line because it is needed by the exactly two files in this directory.*/
#pragma once
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/single_threaded_apartment.hpp>
#include <mutex>
#include <vector>
#include <map>
#include <string>
#include <windows.h>

namespace audio_io {

class OutputDevice;

namespace implementation {

class WasapiOutputDevice: public OutputDeviceImplementation {
};

class WasapiOutputDeviceFactory: public OutputDeviceFactoryImplementation {
	public:
	WasapiOutputDeviceFactory();
	~WasapiOutputDeviceFactory();
	std::vector<std::string> getOutputNames() override;
	std::vector<int> getOutputMaxChannels() override;
	std::shared_ptr<OutputDevice> createDevice(std::function<void(float*, int)> callback, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) override;
	unsigned int getOutputCount() override;
	std::string getName() override;
	private:
	std::vector<std::string> names;
	std::vector<int> max_channels;
	//Mmdevapi uses device identifier strings as opposed to integers, so we need to map.
	void rescan();
	std::map<int, std::string> ids_to_id_strings;
};

}
}