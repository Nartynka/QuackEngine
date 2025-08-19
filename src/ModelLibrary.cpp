#include "ModelLibrary.h"

#include "Model.h"

namespace Quack
{
	std::unique_ptr<Model> ModelLibrary::duck = nullptr;
	std::unique_ptr<Model> ModelLibrary::sphere = nullptr;

	void ModelLibrary::Init()
	{
		duck = std::make_unique<Model>("res/models/duck.fbx");
		sphere = std::make_unique<Model>("res/models/sphere.fbx");
	}
}