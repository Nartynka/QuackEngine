#include "Engine.h"

// This file is the application that will run on the engine
// Maybe in the future this will become a separate exe

int main()
{
	auto* engine = new Quack::Engine();
	engine->Run();
	
	//delete engine;
}
