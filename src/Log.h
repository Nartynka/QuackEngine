#pragma once

#include <memory>
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

#define QUACK_LOG(...)   ::Quack::Log::GetLogger()->trace(__VA_ARGS__);
#define QUACK_GOOD(...)  ::Quack::Log::GetLogger()->info(__VA_ARGS__);
#define QUACK_WARN(...)  ::Quack::Log::GetLogger()->warn(__VA_ARGS__);
#define QUACK_ERROR(...) ::Quack::Log::GetLogger()->error(__VA_ARGS__);