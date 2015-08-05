/**This header is out-of-line because it is needed by the exactly two files in this directory.*/
#pragma once
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/single_threaded_apartment.hpp>
#include <mutex>
#include <vector>
#include <map>
#include <string>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>


namespace audio_io {

class OutputDevice;

namespace implementation {

class WasapiOutputDevice: public OutputDeviceImplementation {
	public:
	WasapiOutputDevice(std::function<void(float*, int)> callback, IMMDevice* device, int inputFrames, int inputChannels, int inputSr, int mixAhead);
	~WasapiOutputDevice();
	void stop() override;
	private:
	void wasapiMixingThreadFunction();
	std::thread wasapi_mixing_thread;
	std::atomic_flag should_continue;
	SingleThreadedApartment sta; //Named so we can use APARTMENTCALL.
	IAudioClient* client = nullptr;
	IMMDevice* device = nullptr;
	//This can be Waveformatex, but we use the larger struct because sometimes it's not.
	WAVEFORMATEXTENSIBLE format;
	REFERENCE_TIME period;
	//The rest of the variables live in the mixing thread.
};

class WasapiOutputDeviceFactory: public OutputDeviceFactoryImplementation {
	public:
	WasapiOutputDeviceFactory();
	~WasapiOutputDeviceFactory();
	std::vector<std::wstring> getOutputNames() override;
	std::vector<int> getOutputMaxChannels() override;
	std::shared_ptr<OutputDevice> createDevice(std::function<void(float*, int)> callback, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) override;
	unsigned int getOutputCount() override;
	std::string getName();
	private:
	void rescan();
	std::vector<std::wstring> names;
	std::vector<int> max_channels;
	//Mmdevapi uses device identifier strings as opposed to integers, so we need to map.
	std::map<int, std::wstring> ids_to_id_strings;
	//The device enumerator, kept here so that we can avoid remaking it all the time.
	IMMDeviceEnumerator* enumerator = nullptr;
	SingleThreadedApartment sta;
};

//These are constants for the interfaces we care about.
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IMMDevice = __uuidof(IMMDevice);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioClock = __uuidof(IAudioClock);


//Call the specified callable with the specified args (must be at least one)in apartment ast.
#define APARTMENTCALL(func, ...) (sta.callInApartment([&] () {return func(__VA_ARGS__);}))

}
}