#include <audio_io/audio_io.hpp>
#include <math.h>
#include <thread>
#include <chrono>

#define PI 3.14159
#define BLOCK_SIZE 1024
#define MIX_AHEAD 3
#define CHANNELS 2
#define SR 48000

class SineGen {
	public:
	SineGen(float freq);
	void operator()(float* block, int channels);
	float phase, delta, frequency;
};

SineGen::SineGen(float freq): frequency(freq), phase(0) {
	delta= freq/SR;
}

void SineGen::operator()(float* block, int channels) {
	for(int i = 0; i < BLOCK_SIZE; i++) {
		for(int j = 0; j < channels; j++) block[i*channels+j] = sinf(2*PI*phase);
		phase+=delta;
	}
	phase=phase-floorf(phase);
}

int main(int argc, char** args) {
	auto gen= SineGen(300.0);
	auto factory = audio_io::getOutputDeviceFactory();
	auto dev = factory->createDevice(gen, -1, CHANNELS, SR, BLOCK_SIZE, MIX_AHEAD);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
