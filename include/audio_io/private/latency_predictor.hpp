#include <chrono>
namespace audio_io {
namespace implementation {

/**Predict latency requirements.*/
class LatencyPredictor {
	public:
	LatencyPredictor(int historyLength, double startLatency, double minAllowedLatency, double maxAllowedLatency);
	LatencyPredictor(const LatencyPredictor& other) = delete;
	LatencyPredictor& operator=(const LatencyPredictor& other) = delete;
	LatencyPredictor(int historyLength, float startLatency, float minAllowedLatency, float maxAllowedLatency);
	~LatencyPredictor();
	//Call before processing a chunk of audio.
	void beginPass();
	//Call after processing a chunk of audio.
	void endPass();
	double predictLatency();
	int predictLatencyInBlocks(int blockFrames, int sr);
	private:
	double* history;
	int history_length, write_pointer = 0;
	double min_allowed_latency, max_allowed_latency;
	std::chrono::high_resolution_clock::time_point pass_start_time;
};

}
}