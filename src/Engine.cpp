#include "Engine.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <functional>
#include <random>

#include "Window.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Log.h"
#include "Model.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "Input.h"
#include "Camera.h"

#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "Shapes.h"
#include "ModelLibrary.h"
#include "LightCube.h"
#include "Systems.h"

namespace Quack
{
	Engine::Engine()
	{
		Log::Init();

		window = std::make_unique<Window>(1080, 720);
		window->SetCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1)); // bind or lambda that is the question :b
		
		Input::SetWindow(window->GetWindow());
		
		Renderer::Init();

		scene = std::make_shared<Scene>();

		camera = std::make_unique<Camera>();

		ModelLibrary::Init();

		QUACK_LOG("Hello Engine!");
	}

	Engine::~Engine()
	{
		window->Shutdown();
	}

	void Engine::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		//dispatcher.Dispatch<KeyPressedEvent>([](const KeyPressedEvent& e) { QUACK_GOOD("Key Pressed!!! key code: {}", e.GetKeyCode()); });
		dispatcher.Dispatch<MouseLeftButtonPressedEvent>(std::bind(&Engine::OnLeftMouseButton, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseRightButtonPressedEvent>(std::bind(&Engine::OnRightMouseButton, this, std::placeholders::_1));
		//dispatcher.Dispatch<MouseMovedEvent>([](const MouseMovedEvent& e) {QUACK_GOOD("Mouse Moved!!!"); });
		dispatcher.Dispatch<MouseScrolledEvent>([&](const MouseScrolledEvent& e) {QUACK_GOOD("Mouse Scrolled!!!"); camera->speed += e.offsetY; camera->speed = camera->speed < 1.f ? 1.f : camera->speed; });
	}


	float randRange(float min, float max)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}

	inline float randX()
	{
		return randRange(-3.5f, 3.5f);
	}

	inline float randZ()
	{
		return randRange(-9.5f, -0.5f);
	}

	void Engine::OnLeftMouseButton(const MouseLeftButtonPressedEvent& e)
	{
		QUACK_LOG("Left mouse button pressed!!");

		Entity entity = scene->CreateEntity();

		//entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(randX(), 5.f, randZ())));
		//entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
		//entity.AddComponent<CollisionComponent>(glm::vec3(0.3f, 0.41f, 0.5f));
		//entity.AddComponent<ModelComponent>(ModelLibrary::duck.get());

		glm::vec3 position = glm::vec3(randX(), 5.f, randZ());
		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), position));
		entity.AddComponent<PhysicsComponent>(position);
		entity.AddComponent<CollisionComponent>(glm::vec3(0.5f));
		entity.AddComponent<ModelComponent>(ModelLibrary::sphere.get());
	}

	void Engine::OnRightMouseButton(const MouseRightButtonPressedEvent& e)
	{
		QUACK_LOG("Right mouse button pressed!!");

		Entity entity = scene->CreateEntity();

		//entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(randX(), 5.f, randZ())));
		//entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
		//entity.AddComponent<CollisionComponent>(glm::vec3(0.25f));
		//entity.AddComponent<ShapeComponent>(new Cube(glm::vec3(0.25f)));

		glm::vec3 position = glm::vec3(randX(), 5.f, randZ());
		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), position));
		entity.AddComponent<PhysicsComponent>(position, 100.f);
		entity.AddComponent<CollisionComponent>(glm::vec3(0.5f));
		entity.AddComponent<ModelComponent>(ModelLibrary::sphere.get());
	}

	void Engine::Run()
	{
		const float DESIRED_DT = 1 / 60.f; // 60 FPS
		
		Shader shader("res/shaders/Basic.shader");
		shader.Bind();

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		auto& registry = scene->GetRegistry(); // @TODO: remove this and make systems for ECS
		float lastTime = 0.f;

		glm::vec3 floorPos = glm::vec3(0.f, -0.5f, -5.f);
		glm::vec3 floorHalfSize = glm::vec3(3.5f, 0.1f, 4.5f);
		NormalCube floor(floorHalfSize);


		LightCube lightCube;
		lightCube.position = glm::vec3(0.f, 3.5f,  0.f);
		//lightCube.position = glm::vec3(0.f, sin(glfwGetTime()) * 3.5f, (cos(glfwGetTime()) * 5.5f) - 5.5f);
		shader.SetUniform3f("lightColor", 1.f, 1.f, 1.f);
		shader.SetUniform3fv("lightPos", glm::value_ptr(lightCube.position));


		NormalCube normalCube;
		glm::vec3 normalCubePos = glm::vec3(0.f, 0.0f, 2.f);
		
		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			float currentTime = (float)glfwGetTime(); // time since glfw initialization in seconds
			float dt = currentTime - lastTime;
			
			if (dt >= DESIRED_DT)
			{
				lastTime = currentTime;

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				camera->Update(dt);

				// Maybe view & projection should be in camera class?
				// first vector moves the scene???
				// camera move is the inverse of what we want to do? inverse the direction of where we want to go???
				// the second vector is the point that we look at 
				view = glm::lookAt(camera->position, camera->position + camera->front, camera->up);
				shader.Bind();
				shader.SetUniform4fv("view", glm::value_ptr(view));
				shader.SetUniform4fv("projection", glm::value_ptr(projection));

				shader.SetUniform3fv("viewPos", glm::value_ptr(camera->position));

				// Physics for entities
				//Move(scene, dt);
				//CheckCollision(scene, dt, floorPos, floorHalfSize);

				ApplyForces(scene);
				Update(scene, dt);
				SolveConstraint(scene, floorPos, floorHalfSize);
				Move(scene); // Update transform component with position from physics component


				// Render entities
				RenderShapes(scene, shader);
				RenderModels(scene, shader);
				RenderCollisionShapes(scene, shader);


				// Render floor
				model = glm::translate(glm::mat4(1.0f), floorPos);
				shader.SetUniform4fv("model", glm::value_ptr(model));
				shader.SetUniform4f("inColor", 0.0f, 0.5f, 0.5f);
				Renderer::DrawNotIndexed(*floor.vao, floor.GetVerticesCount(), shader);

				// Render normal cube
				model = glm::translate(glm::mat4(1.0f), normalCubePos);
				model = glm::scale(model, glm::vec3(4.5f, 0.1f, 3.5f));
				shader.SetUniform4fv("model", glm::value_ptr(model));
				shader.SetUniform4f("inColor", 0.5f, 0.5f, 0.5f);
				Renderer::DrawNotIndexed(*normalCube.vao, normalCube.GetVerticesCount(), shader);

				// Render light cube
				model = glm::translate(glm::mat4(1.0f), lightCube.position);
				lightCube.shader->Bind();
				lightCube.shader->SetUniform4fv("MVP", glm::value_ptr(projection * view * model));
				Renderer::Draw(*lightCube.shape->vao, *lightCube.shape->ibo, *lightCube.shader);

				window->Update();
			}
		}
	}
}
