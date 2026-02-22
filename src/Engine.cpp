#include "Engine.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <functional>
#include <random>
#include "imgui_internal.h"

#include "Window.h"
#include "Renderer.h"
#include "UI.h"
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

		window = std::make_unique<Window>(1920, 1080);
		window->SetCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1)); // bind or lambda that is the question :b
		
		Input::SetWindow(window->GetWindow());
		
		Renderer::Init();

		scene = std::make_shared<Scene>();

		ui = std::make_unique<UI>(&*window, &*scene);

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
		const auto& io = ImGui::GetIO();

		// Mouse & Keyboard capturing should be handled separately but for now will do
		if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
		{
			EventDispatcher dispatcher(event);
			camera->OnEvent(event);

			dispatcher.Dispatch<MouseButtonPressedEvent>(std::bind(&Engine::OnMouseButtonPressed, this, std::placeholders::_1));
		}
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

	void Engine::OnMouseButtonPressed(const MouseButtonPressedEvent& e)
	{
		if (e.GetButton() == GLFW_MOUSE_BUTTON_LEFT)
		{
			Entity entity = scene->CreateEntity();

			glm::vec3 position = glm::vec3(randX(), 5.f, randZ());
			entity.AddComponent<TransformComponent>(position, 21.37f, glm::vec3(0.f, 0.f, 1.f));
			entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.5f));
			entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
			entity.AddComponent<ShapeComponent>(new NormalCube(), glm::vec4(0.5f, 0.0f, 0.5f, 1.0f));
		}
		else if(e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			float z = -4.5f;
			glm::mat4 transform;
			glm::vec4 cubeColor = glm::vec4(0.0f, 1.0f, 0.5f, 1.f);
			for (int i = 0; i < 3; i++)
			{
				float y = 6.f;
				for (int j = 0; j < 3; j++)
				{
					float x = -3.f;
					for (int k = 0; k < 4; k++)
					{
						Entity entity = scene->CreateEntity();
						glm::vec3 position = glm::vec3(x, y, z);
						entity.AddComponent<TransformComponent>(position);
						entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.5f));
						entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
						entity.AddComponent<ShapeComponent>(new NormalCube(glm::vec3(0.5f)), cubeColor);
						x += 1.1f;
					}
					y += 1.1f;
				}
				z -= 1.1f;
			}
		}
	}

	void Engine::Run()
	{
		const float DESIRED_DT = 1 / 60.f; // 60 FPS
		
		Shader shader("res/shaders/Basic.shader");
		shader.Bind();

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;

		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

		glm::vec3 floorHalfSize = glm::vec3(3.5f, 0.1f, 4.5f);
		glm::vec4 floorColor = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);
		//{
		//	Entity floor = scene->CreateEntity();
		//	glm::vec3 floorPos = glm::vec3(0.f, -1.5f, -5.f);
		//	floor.AddComponent<TransformComponent>(floorPos, 90.f, glm::vec3(0.f, 0.f, 1.f));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ColliderComponent>(floorHalfSize);
		//	auto& r = floor.AddComponent<RigidBodyComponent>(100.f, floorHalfSize);
		//	//r.gravity = glm::vec3(0.f);
		//	r.angularVelocity = glm::vec3(0.f, 0.f, 5.f);
		//	r.friction = 1.f;
		//}
		//{
		//	Entity floor = scene->CreateEntity();
		//	glm::vec3 floorPos = glm::vec3(0.f, -1.5f, 4.f);
		//	floor.AddComponent<TransformComponent>(floorPos, 90.f, glm::vec3(0.f, 0.f, 1.f));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ColliderComponent>(floorHalfSize);
		//	auto& r = floor.AddComponent<RigidBodyComponent>(100.f, floorHalfSize);
		//	r.gravity = glm::vec3(0.f);
		//}
		//{
		//	Entity floor = scene->CreateEntity();
		//	glm::vec3 floorPos = glm::vec3(0.f, -8.5f, 4.f);
		//	floor.AddComponent<TransformComponent>(floorPos, 90.f, glm::vec3(0.f, 0.f, 1.f));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ColliderComponent>(floorHalfSize);
		//	auto& r = floor.AddComponent<RigidBodyComponent>(100.f, floorHalfSize);
		//	r.gravity = glm::vec3(0.f);
		//}
		//{
		//	Entity floor = scene->CreateEntity();
		//	glm::vec3 floorPos = glm::vec3(0.f, -8.5f, -5.f);
		//	floor.AddComponent<TransformComponent>(floorPos, 90.f, glm::vec3(0.f, 0.f, 1.f));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ColliderComponent>(floorHalfSize);
		//	auto& r = floor.AddComponent<RigidBodyComponent>(100.f, floorHalfSize);
		//	r.gravity = glm::vec3(0.f);
		//}





		//{
		//	Entity floor = scene->CreateEntity();
		//	glm::vec3 floorPos = glm::vec3(-3.5f, -5.5f, -5.f);
		//	floor.AddComponent<TransformComponent>(floorPos, -40.f, glm::vec3(0.f, 0.f, 1.f));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ColliderComponent>(floorHalfSize);
		//	auto& r = floor.AddComponent<RigidBodyComponent>();
		//	r.gravity = glm::vec3(0.f);
		//	r.angularVelocity = glm::vec3(0.f, 0.f, 5.f);
		//	r.friction = 1.f;
		//}
		//{
		//	Entity floor = scene->CreateEntity();
		//	glm::vec3 floorPos = glm::vec3(2.f, -9.5f, -5.f);
		//	floor.AddComponent<TransformComponent>(floorPos, 40.f, glm::vec3(0.f, 0.f, 1.f));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ColliderComponent>(floorHalfSize);
		//	auto& r = floor.AddComponent<RigidBodyComponent>();
		//	r.gravity = glm::vec3(0.f);
		//	r.angularVelocity = glm::vec3(0.f, 0.f, 5.f);
		//	r.friction = 1.f;
		//}
		//{
		//	Entity floor = scene->CreateEntity();
		//	glm::vec3 floorPos = glm::vec3(-3.5f, -13.5f, -5.f);
		//	floor.AddComponent<TransformComponent>(floorPos, -40.f, glm::vec3(0.f, 0.f, 1.f));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ColliderComponent>(floorHalfSize);
		//	auto& r = floor.AddComponent<RigidBodyComponent>();
		//	r.gravity = glm::vec3(0.f);
		//	r.angularVelocity = glm::vec3(0.f, 0.f, -5.f);
		//	r.friction = 1.f;
		//}
		{
			Entity floor = scene->CreateEntity();
			glm::vec3 floorPos = glm::vec3(2.f, -17.5f, -5.f);
			floor.AddComponent<TransformComponent>(floorPos, 40.f, glm::vec3(0.f, 0.f, 1.f));
			floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
			floor.AddComponent<ColliderComponent>(floorHalfSize);
		}
		{
			Entity floor = scene->CreateEntity();
			glm::vec3 floorPos = glm::vec3(-3.5f, -21.5f, -5.f);
			floor.AddComponent<TransformComponent>(floorPos, -40.f, glm::vec3(0.f, 0.f, 1.f));
			floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
			floor.AddComponent<ColliderComponent>(floorHalfSize);
		}


		//glm::vec3 wallHalfSize = glm::vec3(20.f, 0.1f, 15.f);
		//glm::vec4 wallColor = glm::vec4(1.f, 0.5f, 1.f, 1.0f);
		//{
		//	Entity wall = scene->CreateEntity();
		//	glm::vec3 wallPos = glm::vec3(6.f, -15.f, 0.f);
		//	wall.AddComponent<TransformComponent>(wallPos, 90.f, glm::vec3(0.f, 0.f, 1.f));
		//	wall.AddComponent<ShapeComponent>(new NormalCube(wallHalfSize), wallColor);
		//	wall.AddComponent<ColliderComponent>(wallHalfSize);
		//}
		//{
		//	Entity wall = scene->CreateEntity();
		//	glm::vec3 wallPos = glm::vec3(-8.f, -15.f, 0.f);
		//	wall.AddComponent<TransformComponent>(wallPos, 90.f, glm::vec3(0.f, 0.f, 1.f));
		//	wall.AddComponent<ShapeComponent>(new NormalCube(wallHalfSize), wallColor);
		//	wall.AddComponent<ColliderComponent>(wallHalfSize);
		//}

		//{
		//	glm::vec3 halfSize = glm::vec3(7.f, 0.1f, 20.f);
		//	Entity wall = scene->CreateEntity();
		//	glm::vec3 wallPos = glm::vec3(-1.f, -15.f, -15.11f);
		//	wall.AddComponent<TransformComponent>(wallPos, 90.f, glm::vec3(1.f, 0.f, 0.f));
		//	wall.AddComponent<ShapeComponent>(new NormalCube(halfSize), wallColor);
		//	wall.AddComponent<ColliderComponent>(halfSize);
		//}


		//{ // first "static" sphere
		//	Entity entity = scene->CreateEntity();
		//	glm::vec3 position = glm::vec3(-1.8f, 0.f, -5.f);
		//	float radius = 1.f;
		//	entity.AddComponent<TransformComponent>(position);
		//	auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(1000.f, radius);
		//	//rigidBodyComp.velocity = glm::vec3(0.f, -5.f, 0.f);
		//	rigidBodyComp.gravity = glm::vec3(0.f);
		//	entity.AddComponent<ColliderComponent>(radius);
		//	entity.AddComponent<ShapeComponent>(new Sphere(radius), glm::vec4(1.f, 0.f, 1.f, 1.f));
		//}
		//{ // first falling sphere
		//	Entity entity = scene->CreateEntity();
		//	glm::vec3 position = glm::vec3(-2.0f, 5.f, -5.f);
		//	float radius = 1.f;
		//	entity.AddComponent<TransformComponent>(position);
		//	auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(1.f, radius);
		//	entity.AddComponent<ColliderComponent>(radius);
		//	entity.AddComponent<ShapeComponent>(new Sphere(radius), glm::vec4(1.f, 0.f, 1.f, 1.f));
		//}
		//{ // second falling sphere
		//	Entity entity = scene->CreateEntity();
		//	glm::vec3 position = glm::vec3(2.0f, 5.f, -6.f);
		//	float radius = 1.f;
		//	entity.AddComponent<TransformComponent>(position);
		//	auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(1.f, radius);
		//	entity.AddComponent<ColliderComponent>(radius);
		//	entity.AddComponent<ShapeComponent>(new Sphere(radius), glm::vec4(1.f, 0.f, 1.f, 1.f));
		//}


		{ // fist "static" cube
			Entity entity = scene->CreateEntity();
			glm::vec3 position = glm::vec3(0.f, 0.f, -5.f);
			//glm::vec3 halfSize = glm::vec3(3.5f, 0.1f, 4.5f);
			glm::vec3 halfSize = glm::vec3(0.5f);
			entity.AddComponent<TransformComponent>(position, 0.f, glm::vec3(0.f, 0.f, 1.f));
			entity.AddComponent<ColliderComponent>(halfSize);
			entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(0.0f, 0.5f, 0.5f, 1.0f));
		}

		//{ // second "static" cube
		//	Entity entity = scene->CreateEntity();
		//	glm::vec3 position = glm::vec3(0.f, 0.f, -5.f);
		//	entity.AddComponent<TransformComponent>(position, 0.f, glm::vec3(0.f, 0.f, 1.f));
		//	auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(100.f, glm::vec3(0.5f));
		//	rigidBodyComp.gravity = glm::vec3(0.f);
		//	entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
		//	entity.AddComponent<ShapeComponent>(new NormalCube(), glm::vec4(1.f, 0.5f, 1.f, 1.f));
		//}
		//{ // first falling cube
		//	glm::vec3 size = glm::vec3(0.5f);
		//	//glm::vec3 size = glm::vec3(1.f, 0.5f, 2.f);
		//	Entity entity = scene->CreateEntity();
		//	glm::vec3 position = glm::vec3(0.f, 5.f, -5.f);
		//	entity.AddComponent<TransformComponent>(position, 30.f, glm::vec3(0.f, 0.f, 1.f));
		//	auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.5f));
		//	entity.AddComponent<ColliderComponent>(size);
		//	entity.AddComponent<ShapeComponent>(new NormalCube(size), glm::vec4(1.f, 0.f, 1.f, 1.f));
		//}


		// Stack of spheres
		//float z = -4.f;
		//glm::vec4 sphereColor = glm::vec4(0.0f, 1.0f, 0.5f, 1.f);
		//float radius = 0.25f;
		//for (int i = 0; i < 5; i++)
		//{
		//	float y = 2.f;
		//	for (int j = 0; j < 5; j++)
		//	{
		//		float x = -1.f;
		//		for (int k = 0; k < 5; k++)
		//		{
		//			Entity entity = scene->CreateEntity();
		//			glm::vec3 position = glm::vec3(x, y, z);
		//			entity.AddComponent<TransformComponent>(position);
		//			entity.AddComponent<RigidBodyComponent>(1.f, radius);
		//			entity.AddComponent<ColliderComponent>(radius);
		//			entity.AddComponent<ShapeComponent>(new Sphere(radius), sphereColor);
		//			x += 0.5f;
		//		}
		//		y += 0.5f;
		//	}
		//	z -= 0.5f;
		//}

		// Stack of cubes
		//float z = -4.5f;
		//glm::mat4 transform;
		//glm::vec4 cubeColor = glm::vec4(0.0f, 1.0f, 0.5f, 1.f);
		//for (int i = 0; i < 3; i++)
		//{
		//	float y = 6.f;
		//	for (int j = 0; j < 3; j++)
		//	{
		//		float x = -3.f;
		//		for (int k = 0; k < 4; k++)
		//		{
		//			Entity entity = scene->CreateEntity();
		//			glm::vec3 position = glm::vec3(x, y, z);
		//			entity.AddComponent<TransformComponent>(position);
		//			entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.5f));
		//			entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
		//			entity.AddComponent<ShapeComponent>(new NormalCube(glm::vec3(0.5f)), cubeColor);
		//			x += 1.1f;
		//		}
		//		y += 1.1f;
		//	}
		//	z -= 1.1f;
		//}


		LightCube lightCube;
		lightCube.position = glm::vec3(-1.f, 4.5f, 2.f);
		shader.SetUniform3f("lightColor", 1.f, 1.f, 1.f);
		shader.SetUniform3fv("lightPos", glm::value_ptr(lightCube.position));

		float lastTime = 0.f;

		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			float currentTime = (float)glfwGetTime(); // time since glfw initialization in seconds
			float dt = glm::min(currentTime - lastTime, 0.1f);
			
			if (dt >= DESIRED_DT)
			{
				lastTime = currentTime;

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				camera->Update(dt);
				ui->StartFrame();
				// Maybe view & projection should be in camera class?
				// first vector moves the scene???
				// camera move is the inverse of what we want to do? inverse the direction of where we want to go???
				// the second vector is the point that we look at 
				view = glm::lookAt(camera->position, camera->position + camera->front, camera->up);
				auto [width, height] = window->GetWindowSize();

				// @TODO: width and height can be 0 and cause a crash
				projection = glm::perspective(glm::radians(camera->fov), (float)width / (float)height, 0.1f, 100.0f);

				Renderer::linesShader->Bind();
				Renderer::linesShader->SetUniform4fm("view", glm::value_ptr(view));
				Renderer::linesShader->SetUniform4fm("projection", glm::value_ptr(projection));
				
				//Renderer::DrawDebug();

				shader.Bind();
				shader.SetUniform4fm("view", glm::value_ptr(view));
				shader.SetUniform4fm("projection", glm::value_ptr(projection));

				shader.SetUniform3fv("viewPos", glm::value_ptr(camera->position));

				// Physics for entities
				ApplyForces(scene);
				Update(scene, dt); // Update physics
				CheckCollisions(scene);
				SolveCollisions();
				UpdateTransform(scene); // Update transformComp with position & rotation from rigidBodyComp after physics simulation


				// Render entities
				RenderShapes(scene, shader);
				RenderModels(scene, shader);
				RenderCollisionShapes(scene);


				// Render light cube
				model = glm::translate(glm::mat4(1.0f), lightCube.position);
				lightCube.shader->Bind();
				lightCube.shader->SetUniform4fm("MVP", glm::value_ptr(projection * view * model));
				Renderer::Draw(*lightCube.shape->vao, *lightCube.shape->ibo, *lightCube.shader);

				ui->EndFrame();
				window->Update();
			}
		}
	}
}
