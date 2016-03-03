#pragma once
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <string>
#include <vector>

namespace audio_io {
namespace implementation {

class AlsaOutputDeviceFactory: public OutputDeviceFactoryImplementation {
	public:
	AlsaOutputDeviceFactory();
	std::string getName() override;
	std::vector<std::string> getOutputNames() override;
	std::vector<int> getOutputMaxChannels() override;
	std::unique_ptr<OutputDevice> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) override;
	private:
	void rescan();
	std::vector<std::string> device_names;
	std::vector<int> device_channels;
};

}
}