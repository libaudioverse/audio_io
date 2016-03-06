#include "alsa.hpp"
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <alsa/asoundlib.h>

namespace audio_io {
namespace implementation {

AlsaOutputDevice::AlsaOutputDevice(std::function<void(float*, int)> callback, std::string name, int sr, int channels, int blockSize, float minLatency, float startLatency, float maxLatency) {
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca(&params);
	int res = snd_pcm_open(&device_handle, name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if(res) {
		throw AudioIOError("An Alsa Error occurred.");
	}
	res = snd_pcm_hw_params_any(device_handle, params);
	if(res < 0) {
		throw AudioIOError("Alsa: could not configuer PCM device.");
	}
	//TODO: this doesn't deal with endianness properly, and uses our knowledge that x86 is little endian.
	res = snd_pcm_hw_params_set_format(device_handle, params, SND_PCM_FORMAT_FLOAT_LE);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't get little endian float.");
	}
	unsigned int alsaChannels = channels;
	res = snd_pcm_hw_params_set_channels_near (device_handle, params, &alsaChannels);
	if(res < 0) {
		throw AudioIOError("Couldn't constrain channel count.");
	}
	unsigned int alsaSr = sr;
	res = snd_pcm_hw_params_set_rate_near(device_handle, params, &alsaSr, 0);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't constrain sample rate.");
	}
	res = snd_pcm_hw_params_set_access(device_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set access.");
	}
	//compute needed buffer size.
	snd_pcm_uframes_t alsaBufferSize = alsaSr*maxLatency;
	res = snd_pcm_hw_params_set_buffer_size_near(device_handle, params, &alsaBufferSize);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set buffer size.");
	}
	res = snd_pcm_hw_params(device_handle, params);
	if(res < 0) {
		throw AudioIOError("ALSA: Could not prepare device.");
	}
	init(callback, blockSize, channels, sr, (int)alsaChannels, (int)alsaSr);
	worker_running.test_and_set();
	worker_thread = std::thread([&] () {workerThreadFunction();});
}

AlsaOutputDevice::~AlsaOutputDevice() {
	stop();
}

void AlsaOutputDevice::stop() {
	if(stopped == false) {
		if(worker_thread.joinable()) {
			worker_running.clear();
			worker_thread.join();
		}
	}
	stopped = true;
}

void AlsaOutputDevice::workerThreadFunction() {
	float* buffer = new float[output_channels*output_frames];
	while(worker_running.test_and_set()) {
		sample_format_converter->write(output_frames, buffer);
		snd_pcm_sframes_t res = snd_pcm_writei(device_handle, buffer, (snd_pcm_uframes_t)output_frames);
		if(res < 0) {
			snd_pcm_prepare(device_handle);
		}
	}
}

}
}