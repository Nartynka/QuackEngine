#include "ModelLibrary.h"

#include "Model.h"

namespace Quack
{
	std::unique_ptr<Model> ModelLibrary::duck = nullptr;

	void ModelLibrary::Init()
	{
		duck = std::make_unique<Model>("res/models/duck.fbx");
	}
}