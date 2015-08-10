#include "wasapi.hpp"
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <logger_singleton.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <string.h>
#include <windows.h>
#include <audioclient.h>

namespace audio_io {
namespace implementation {

WasapiOutputDevice::WasapiOutputDevice(std::function<void(float*, int)> callback, IMMDevice* device, int inputFrames, int inputChannels, int inputSr, int mixAhead)  {
	this->device = device;
	logger_singleton::getLogger()->logDebug("audio_io", "Attempting to initialize a Wasapi device.");
	//todo: this whole thing needs error handling.
	APARTMENTCALL(device->Activate, IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
	WAVEFORMATEX *format = nullptr;
	APARTMENTCALL(client->GetMixFormat, &format);
	if(format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) this->format = *(WAVEFORMATEXTENSIBLE*)format;
	else this->format.Format = *format;
	CoTaskMemFree(format);
	//Msdn strongly hints that we will always be able to convert from float to the mix format in shared mode.
	//Consequently, force our format to be float and see if it's supported.
	//First, the WAVEFORMATEX.
	WAVEFORMATEX &f = this->format.Format;
	bool wasExtensible = f.wFormatTag == WAVE_FORMAT_EXTENSIBLE;
	f.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	f.nAvgBytesPerSec = 4*f.nSamplesPerSec*f.nChannels;
	f.nBlockAlign = f.nChannels*4;
	f.wBitsPerSample = 32;
	f.cbSize = 22;
	//Now the extended WAVEFORMATEXTENSIBLE.
	WAVEFORMATEXTENSIBLE &f2 = this->format;
	f2.Samples.wValidBitsPerSample = 32;
	f2.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	if(wasExtensible == false) {
		//We need to do the channel mask, if we can.
		if(f.nChannels == 2) f2.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		else if(f.nChannels == 4) f2.dwChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
		else if(f.nChannels == 6) f2.dwChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
		else if(f.nChannels == 8) f2.dwChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT;
		else f2.dwChannelMask = (1<<f.nChannels)-1; //At least have enough channels, even if they aren't the right ones.
	}
	//Todo: if the format isn't supported, error.
	auto res = APARTMENTCALL(client->IsFormatSupported, AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX*)&(this->format), &format);
	//Todo: this needs to error.
	if(res != S_OK) {
		logger_singleton::getLogger()->logCritical("audio_io", "Wasapi device cannot initialize due to unsupported format.");
		return;
	}
	//We need a latency in nanoseconds as a REFERENCE_TIME.  Let's go to seconds, first.
	//The +1.5 here lets us handle the zero mixahead case.
	//+1 means at least 1 buffer, .5 gives us a little extra time so that we can fill it and makes sure that we don't round below the input frame count.
	float latencySeconds = (float)(mixAhead+1.5)*inputFrames/inputSr;
	//We do this separately because if we don't the right-hand side will become too large for a float.
	//It could be combined, but that would be uglier.
	REFERENCE_TIME latencyNanoseconds = (REFERENCE_TIME)(latencySeconds*1000);
	latencyNanoseconds *= 1000000; //1e6 nanoseconds per second.
	//Finally, we can make the initialize call and retrieve our actual period in nanoseconds.
	res = APARTMENTCALL(client->Initialize, AUDCLNT_SHAREMODE_SHARED, 0, latencyNanoseconds/100, 0, (WAVEFORMATEX*)&(this->format), NULL);
	if(res != S_OK) {
		logger_singleton::getLogger()->logCritical("audio_io", "Failed to initialize the Wasapi  device. COM error %i.", (int)res);
		return;
	}
	logger_singleton::getLogger()->logInfo("audio_io", "Initialized Wasapi device.  Introduced software latency: %f seconds\n", latencySeconds);
	init(callback, inputFrames, inputChannels, inputSr, this->format.Format.nChannels, this->format.Format.nSamplesPerSec);
	//At this point, we no longer need to go via the STA for the client interface.
	should_continue.test_and_set();
	wasapi_mixing_thread = std::thread(&WasapiOutputDevice::wasapiMixingThreadFunction, this);
}

WasapiOutputDevice::~WasapiOutputDevice() {
	stop();
}

void WasapiOutputDevice::stop() {
	logger_singleton::getLogger()->logInfo("audio_io", "Wasapi device shutting down.");
	logger_singleton::getLogger()->logDebug("audio_io", "Stopping a Wasapi device.");
	should_continue.clear();
	wasapi_mixing_thread.join();
	if(client) client->Release();
	if(device) device->Release();
}

void WasapiOutputDevice::wasapiMixingThreadFunction() {
	//Stuff here can run outside the apartment.
	auto res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(IS_ERROR(res)) {
		logger_singleton::getLogger()->logDebug("audio_io", "Wassapi device mixing thread: could not initialize COM. Error %i", (int)res);
		return; //We really can't recover from this.
	}
	IAudioRenderClient *renderClient = nullptr;
	UINT32 padding, bufferSize;
	client->GetBufferSize(&bufferSize);
	client->GetCurrentPadding(&padding);
	client->GetService(IID_IAudioRenderClient, (void**)&renderClient);
	//We use double buffering, as processing can take a long time.
	float* workspace = new float[output_channels*bufferSize]();
	BYTE* audioBuffer;
	sample_format_converter->write(bufferSize-padding, workspace);
	renderClient->GetBuffer(bufferSize-padding, &audioBuffer);
	memcpy(audioBuffer, workspace, sizeof(float)*output_channels*(bufferSize-padding));
	renderClient->ReleaseBuffer(bufferSize-padding, 0);
	//The buffer is filled, so we begin processing.
	client->Start();
	logger_singleton::getLogger()->logDebug("audio_io", "Wasapi mixing thread: audio client is started.  Mixing audio.");
	//From here, it's thankfully much simpler.  Every time we have at least output_frames worth of empty buffer, we fill it.
	while(should_continue.test_and_set()) {
		client->GetCurrentPadding(&padding);
		UINT32 available = bufferSize-padding;
		//Wait until we have enough data.
		if(available < output_frames) {
			std::this_thread::sleep_for(std::chrono::milliseconds((output_frames-available)*1000/output_sr));
			continue;
		}
		sample_format_converter->write(output_frames, workspace);
		if(renderClient->GetBuffer(available, &audioBuffer) != S_OK) {
			std::this_thread::yield();
			continue;
		}
		memcpy(audioBuffer, workspace, sizeof(float)*output_frames*output_channels);
		renderClient->ReleaseBuffer(output_frames, 0);
	}
	//Free stuff:
	renderClient->Release();
	client->Stop();
	client->Reset();
	delete[] workspace;
	CoUninitialize();
	logger_singleton::getLogger()->logDebug("audio_io", "Wasapi mixing thread: exiting.");
}

}
}