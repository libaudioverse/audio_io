#pragma once
#include <vector>
#include <set>
#include <memory>
#include <utility>
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>

namespace audio_io {

/**A physical output.*/
class OutputDevice {
};

class OutputDeviceFactory {
	public:
	DeviceFactory() = default;
	virtual ~DeviceFactory();
	virtual std::vector<std::string> getOutputNames() = 0;
	virtual std::vector<int> getOutputMaxChannels() = 0;
	virtual std::shared_ptr<Device> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) = 0;
	virtual unsigned int getOutputCount();
	virtual std::string getName();
};

}