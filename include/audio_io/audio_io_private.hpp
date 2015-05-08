#pragma once
#include "audio_io.hpp"
#include <vector>
#include <set>
#include <memory>
#include <utility>
#include <atomic>
#include <mutex>
#include <thread>
#include <functional>

//The details in this file are to be considered private.
//Do not include or use this file in public code directly.

namespace audio_io {
namespace private {

class OutputDeviceImplementation: public OutputDevice {
	protected:
	Device() = default;
	virtual ~Device();
	//function parameters: output buffer, number of channels to write.
	virtual void init(std::function<void(float*, int)> getBuffer, unsigned int inputBufferFrames, unsigned int inputBufferSr, unsigned int channels, unsigned int outputSr, unsigned int mixAhead); //second step fn initialization. We can't just fall through to the constructor.
	virtual void start(); //final step in initialization via subclasses: starts the background thread.
	virtual void stop(); //stop the output.
	//these hooks are run in the background thread, and should be overridden in subclasses.
	virtual void startup_hook();
	virtual void shutdown_hook();
	virtual void zeroOrNextBuffer(float* where);
	virtual void mixingThreadFunction();
	unsigned int channels = 0;
	unsigned int mix_ahead = 0;
	unsigned int input_buffer_size, output_buffer_size, input_buffer_frames, output_buffer_frames;
	unsigned int input_sr = 0;
	unsigned int output_sr = 0;
	bool is_resampling = false;
	bool should_apply_mixing_matrix = false;
	unsigned int next_output_buffer = 0;
	unsigned int callback_buffer_index = 0;

	float** buffers = nullptr;
	std::atomic<int>* buffer_statuses = nullptr;
	std::atomic_flag mixing_thread_continue;
	std::thread mixing_thread;
	std::function<void(float*, int)> get_buffer;
	bool started = false;
	friend class DeviceFactory;
};

class OutputDeviceFactoryImplementation: public OutputDeviceFactory {
	protected:
	std::vector<std::weak_ptr<Device>> created_devices;
	int output_count = 0;
};

typedef DeviceFactory* (*DeviceFactoryCreationFunction)();
DeviceFactory* createWinmmDeviceFactory();
DeviceFactory* createOpenALDeviceFactory();

//finally, the function that initializes all of this.
void initializeDeviceFactory();
void shutdownDeviceFactory();

}
}