#pragma once

#include <memory>

namespace Quack
{
	class Model;

	// @TODO: Add shapes here to not create new cube for every entity
	struct ModelLibrary
	{
		static std::unique_ptr<Model> duck;
		static std::unique_ptr<Model> sphere;

		static void Init();
	};
}