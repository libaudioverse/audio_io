#include <thread>
#include <type_traits>
#include <atomic>
#include <functional>
#include <windows.h>

namespace audio_io {
namespace implementation {

/**A single-threaded apartment (as pertaining to Microsoft COM) consists of a message loop and COM initialization.

This class encapsulates a single-threaded apartment and the thread to manage it.*/

class SingleThreadedApartment {
	public:
	SingleThreadedApartment();
	~SingleThreadedApartment();
	
	/*Run a call in the apartment.
	This algorithm is threadsafe, but performance may suffer under contension.*/
	template<typename FuncT, typename... ArgsT>
	typename std::result_of<FuncT(ArgsT...)>::type runInApartment(FuncT callable, ArgsT... args) {
		std::promise<typename std::result_of<FuncT(ArgsT...)>::type> promise;
		auto future = promise.get_future();
		std::function<void(void)> task ([&] () {
			promise.set_value(callable(args));
		});
		submitTask(task);
		future.wait();
		return future.get();
	}
	
	private:
	//Send a task to the thread.
	//Caller needs to keep task alive until after it executes.
	void submitTask(std::function<void(void)> &task);
	//Sends a message to the apartment's thread.
	void sendMessage(UINT msg, WPARAM wparam, LPARAM lparam);
	//The thread.
	void apartmentThreadFunction();
	//Goes null when no task is being worked on.
	std::atomic<std::function<void(void)>*> current_task;
	//Goes to 1 once the apartment thread has made the message queue.
	std::atomic<int> ready;
	std::thread apartment_thread;
};

}
}