#include <iostream>

#include "Engine.h"
#include "Window.h"

// This file is the application that will run on the engine
// Maybe in the future this will become a separate exe

int main()
{
	std::cout << "Hello Engine!" << std::endl;

	auto* engine = new Quack::Engine();
	engine->Run();
	
	//delete engine;
}
