#pragma once
#include <vector>
#include <memory>
#include <functional>

namespace audio_io {

/**A physical output.*/
class OutputDevice {
	public:
	virtual ~OutputDevice() {}
};

class OutputDeviceFactory {
	public:
	OutputDeviceFactory() = default;
	virtual ~OutputDeviceFactory() {}
	virtual std::vector<std::string> getOutputNames() = 0;
	virtual std::vector<int> getOutputMaxChannels() = 0;
	virtual std::shared_ptr<OutputDevice> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) = 0;
	virtual unsigned int getOutputCount() = 0;
	virtual std::string getName() = 0;
};

std::shared_ptr<OutputDeviceFactory> getOutputDeviceFactory();

}