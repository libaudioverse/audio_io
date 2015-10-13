#include <audio_io/audio_io.hpp>
#include <logger_singleton/logger_singleton.hpp>
#include <audio_io/private/logging.hpp>
#include <memory>

namespace audio_io {
namespace implementation {

std::shared_ptr<logger_singleton::Logger> *logger = nullptr;

void initializeLogger() {
	logger = new std::shared_ptr<logger_singleton::Logger>();
	*logger = logger_singleton::createLogger();
}

void shutdownLogger() {
	delete logger;
}

std::shared_ptr<logger_singleton::Logger> getLoggerImpl() {
	return *logger;
}

}

std::shared_ptr<logger_singleton::Logger> getLogger() {
	return implementation::getLoggerImpl();
}

}