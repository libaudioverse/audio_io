#include "wasapi.hpp"
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/single_threaded_apartment.hpp>
#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

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


//There's a deficiency somewhere.  This makes safe COM calls.
#define SAFECALL(func, ...) (sta.callInApartment([&] () {return func(__VA_ARGS__);}))

OutputDeviceFactory* createWasapiOutputDeviceFactory() {
	//In order to determine if we have Wasapi, we attempt to open and close the default output device without error.
	try {
		SingleThreadedApartment sta;
		IMMDeviceEnumerator* enumerator = nullptr;
		IMMDevice *default_device = nullptr;
		IAudioClient* client = nullptr;
		auto res = sta.callInApartment(CoCreateInstance, CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator);
		if(IS_ERROR(res)) return nullptr;
		//Attempt to get the default device.
		res = SAFECALL(enumerator->GetDefaultAudioEndpoint, eRender, eMultimedia, &default_device);
		if(IS_ERROR(res)) {
			enumerator->Release();
			return nullptr;
		}
		res = SAFECALL(default_device->Activate, IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
		if(IS_ERROR(res)) {
			default_device->Release();
			enumerator->Release();
			return nullptr;
		}
		//We now have an IAudioClient.  We can attempt to initialize it with the default mixer format in shared mode, which is supposed to always be accepted.
		WAVEFORMATEX *format = nullptr;
		res = SAFECALL(client->GetMixFormat, &format);
		if(IS_ERROR(res)) {
			client->Release();
			default_device->Release();
			enumerator->Release();
			return nullptr;
		}
		//1e6 nanoseconds in a millisecond.
		//We don't request a specific buffer length, we just want to know if we can open and otherwise don't care.
		res = SAFECALL(client->Initialize, AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, format, NULL);
		if(IS_ERROR(res)) {
			client->Release();
			default_device->Release();
			enumerator->Release();
			return nullptr;
		}
		client->Release();
		default_device->Release();
		enumerator->Release();
		return new WasapiOutputDeviceFactory();
	}
	catch(...) {
		return nullptr;
	}
	return nullptr;
}

}
}