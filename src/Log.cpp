#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Quack
{
	std::shared_ptr<spdlog::logger> Log::logger;

	void Log::Init()
	{
		logger = spdlog::stdout_color_mt("Quack");
		spdlog::set_pattern("%^[%T] %n: %v%$");
		spdlog::set_level(spdlog::level::trace);
	}
}
