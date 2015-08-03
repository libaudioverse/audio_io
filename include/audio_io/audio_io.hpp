#pragma once
#include <vector>
#include <memory>
#include <functional>

namespace audio_io {

/**A physical output.*/
class OutputDevice {
	public:
	virtual ~OutputDevice() {}
};

class OutputDeviceFactory {
	public:
	OutputDeviceFactory() = default;
	virtual ~OutputDeviceFactory() {}
	virtual std::vector<std::string> getOutputNames() = 0;
	virtual std::vector<int> getOutputMaxChannels() = 0;
	virtual std::shared_ptr<OutputDevice> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, unsigned int mixAhead) = 0;
	virtual unsigned int getOutputCount() = 0;
	virtual std::string getName() = 0;
};

std::shared_ptr<OutputDeviceFactory> getOutputDeviceFactory();

/**The following functions work with mixing matrices.
This includes applying and obtaining them.*/

/**Remix a buffer:
- The standard channel counts 1, 2, 6 (5.1) and 8 (7.1) are mixed between each other appropriately.
- Otherwise, if input is mono and output is not mono, the input is copied to all channels.
- Otherwise, we keep min(inputChannels, outputChannels) channels of audio data, and fill any remaining output channels with zero.

These functions come in two variants.
- The uninterleaved version expects audio data as contiguous frames.
- The interleaved version expects audio data as an array of buffers.

In-place usage is not safe.

zeroFirst is intended for applications that need to accumulate data; if true (the default) buffers are zeroed before use.
*/

void remixAudioInterleaved(int frames, int inputChannels, float* input, int outputChannels, float* output, bool zeroFirst = true);
void remixAudioUninterleaved(int frames, int inputChannels, float** inputs, int outputChannels, float** outputs, bool zeroFirst = true);

}