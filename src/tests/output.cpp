#include <audio_io/audio_io.hpp>
#include <math.h>
#include <thread>
#include <chrono>

#define PI 3.14159
int channels = 2;
int mix_ahead = 0;
int sr = 44100;
int block_size = 1024;

class SineGen {
	public:
	SineGen(float freq);
	void operator()(float* block, int channels);
	float phase, delta, frequency;
};

SineGen::SineGen(float freq): frequency(freq), phase(0) {
	delta= freq/sr;
}

void SineGen::operator()(float* block, int channels) {
	for(int i = 0; i < block_size; i++) {
		for(int j = 0; j < channels; j++) block[i*channels+j] = sinf(2*PI*phase);
		phase+=delta;
	}
	phase=phase-floorf(phase);
}

int main(int argc, char** args) {
	if(argc != 5) {
		printf("Usage: output <channels> <sr> <block_size> <mixahead>\n");
		return 0;
	}
	sscanf(args[1], "%i", &channels);
	sscanf(args[2], "%i", &sr);
	sscanf(args[3], "%i", &block_size);
	sscanf(args[4], "%i", &mix_ahead);
	printf("Playing with channels=%i, sr=%i, block_size=%i, mix_ahead=%i\n", channels, sr, block_size, mix_ahead);
	auto gen= SineGen(300.0);
	auto factory = audio_io::getOutputDeviceFactory();
	printf("Using factory: %s\n", factory->getName().c_str());
	auto dev = factory->createDevice(gen, -1, channels, sr, block_size, mix_ahead);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
