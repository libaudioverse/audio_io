#include <audio_io/audio_io.hpp>
#include <stdio.h>

int main(int argc, char** args) {
	auto factory = audio_io::getOutputDeviceFactory();
	printf("Enumerating with %s factory.\n", factory->getName().c_str());
	printf("%i output devices detected:\n\n", (int)factory->getOutputCount());
	auto names = factory->getOutputNames();
	auto channels = factory->getOutputMaxChannels();
	auto i = names.begin();
	auto j = channels.begin();
	for(; i != names.end(); i++, j++) {
		printf("%s (channels = %i)\n", i->c_str(), (int)(*j));
	}
}
