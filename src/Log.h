#pragma once

#include <memory>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

namespace Quack
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return logger; }
	private:
		// in future maybe add more logger e.g. for rendering, physics etc.
		static std::shared_ptr<spdlog::logger> logger;
	};
}

// spdlog macros because we want the source file and the line in the log message
#define QUACK_LOG(...)   SPDLOG_LOGGER_TRACE(::Quack::Log::GetLogger(), __VA_ARGS__);
#define QUACK_GOOD(...)  SPDLOG_LOGGER_INFO(::Quack::Log::GetLogger(), __VA_ARGS__);
#define QUACK_WARN(...)  SPDLOG_LOGGER_WARN(::Quack::Log::GetLogger(), __VA_ARGS__);
#define QUACK_ERROR(...) SPDLOG_LOGGER_ERROR(::Quack::Log::GetLogger(), __VA_ARGS__);

//#define QUACK_LOG(...)   ::Quack::Log::GetLogger()->trace(__VA_ARGS__);
//#define QUACK_GOOD(...)  ::Quack::Log::GetLogger()->info(__VA_ARGS__);
//#define QUACK_WARN(...)  ::Quack::Log::GetLogger()->warn(__VA_ARGS__);
//#define QUACK_ERROR(...) ::Quack::Log::GetLogger()->error(__VA_ARGS__);