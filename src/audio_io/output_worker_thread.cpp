#include <audio_io/private/output_worker_thread.hpp>
#include <powercores/utilities.hpp>
#include <inttypes.h>
#include <thread>
#include <algorithm>

namespace audio_io {
namespace implementation {

OutputWorkerThread::OutputWorkerThread(std::function<void(float*, int)> callback, int sourceFrames, int sourceChannels, int sourceSr, int destinationChannels, int destinationSr, int mixahead):
converter(callback, sourceFrames, sourceChannels, sourceSr, destinationChannels, destinationSr),
prepared_buffers(mixahead),
returned_buffers(mixahead) {
	this->destination_channels = destinationChannels;
	// be careful of this cast. This can overflow without it.
	this->destination_frames = ((int64_t)sourceFrames)*destinationSr/sourceSr;
	this->mixahead = mixahead;
	int bufferSize = this->destination_channels*this->destination_frames;
	for(int i = 0; i < mixahead; i++) {
		auto buff = new AudioBuffer();
		buff->data = new float[bufferSize];
		while(returned_buffers.push(buff) == false);
	}
	semaphore.signal(mixahead);
	running_flag.test_and_set();
	still_initial_mix_flag.test_and_set();
	this->worker_thread = powercores::safeStartThread([&] () {workerThread();});
	// Wait until we've pre-mixed enough data.
	while(still_initial_mix_flag.test_and_set()) std::this_thread::yield();
}

OutputWorkerThread::~OutputWorkerThread() {
	// We free all the buffers, then clear the flag and shut down the background thread.
	int freed = 0;
	while(freed < mixahead) {
		AudioBuffer* buff;
		while(prepared_buffers.pop(buff) == false) std::this_thread::yield();
		delete[] buff->data;
		delete buff;
	}
	running_flag.clear();
	semaphore.signal();
	worker_thread.join();
}

void OutputWorkerThread::workerThread() {
	int mixed = 0;
	bool finishedInitialMix = false;
	while(running_flag.test_and_set()) {
		AudioBuffer* buff;
		bool gotBuffer = false; // Used for below spin-waiting loops.
		// This is ugly, admittedly.
		while((gotBuffer = returned_buffers.pop(buff)) == false && running_flag.test_and_set()); // Get a buffer.
		// The above loop waits until we got a buffer xor we need to exit. So:
		if(gotBuffer == false) return;
		buff->used = 0;
		std::fill(buff->data, buff->data+destination_frames*destination_channels, 0.0f);
		converter.write(destination_frames, buff->data);
		// If we got a buffer, there must be room to push the buffer, so this loop can't wait forever.
		// If we didn't, we exited above.
		while(returned_buffers.push(buff) == false); // Push it.
		// We need to count until we've finished the initial mixahead, then signal the constructor to continue.
		if(finishedInitialMix == false) {
			mixed += 1;
			if(mixed == mixahead) {
				still_initial_mix_flag.clear(); // Allow the constructor to return.
				finishedInitialMix = true;
			}
		}
		// the semaphore is signaled by returning buffers or a request to exit, so we wait on it.
		// It'll pause us when there's no more buffers.
		semaphore.wait();
	}
}

AudioBuffer* OutputWorkerThread::acquireBuffer() {
	AudioBuffer* buff;
	return prepared_buffers.pop(buff) ? buff : nullptr;
}

void OutputWorkerThread::releaseBuffer(AudioBuffer* buff) {
	while(returned_buffers.push(buff) == false);
	semaphore.signal();
}

int OutputWorkerThread::write(int count, float* destination) {
	if(current_buffer == nullptr) {
		current_buffer = acquireBuffer();
	}
	if(current_buffer == nullptr) return 0; // because we didn't get one.
	int remaining = destination_frames-current_buffer->used;
	int willWrite = remaining < count ? remaining : count;
	float* start = current_buffer->data+destination_channels*current_buffer->used;
	float* end = start+destination_channels*willWrite;
	std::copy(start, end, destination);
	current_buffer->used -= willWrite;
	if(current_buffer->used <= 0) {
		releaseBuffer(current_buffer);
		current_buffer = nullptr;
	}
	return willWrite;
}

}
}