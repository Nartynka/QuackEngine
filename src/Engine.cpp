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
			//QUACK_LOG("Left mouse button pressed!!");

			//Entity entity = scene->CreateEntity();

			//entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(randX(), 5.f, randZ())));
			//entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
			//entity.AddComponent<CollisionComponent>(glm::vec3(0.3f, 0.41f, 0.5f));
			//entity.AddComponent<ModelComponent>(ModelLibrary::duck.get());

			//glm::vec3 position = glm::vec3(randX(), 5.f, randZ());
			//entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), position));
			//entity.AddComponent<PhysicsComponent>(position);
			//entity.AddComponent<CollisionComponent>(0.5f);
			//entity.AddComponent<ModelComponent>(ModelLibrary::sphere.get(), glm::vec4(0.0f, 1.0f, 0.5f, 1.f));
			Entity entity = scene->CreateEntity();

			glm::vec3 position = glm::vec3(randX(), 5.f, randZ());
			entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), position));
			entity.AddComponent<PhysicsComponent>(position, 1.f, glm::vec3(0.5f), 21.37f, glm::vec3(0.f, 0.f, 1.f));
			entity.AddComponent<CollisionComponent>(glm::vec3(0.5f));
			entity.AddComponent<ShapeComponent>(new NormalCube(), glm::vec4(0.5f, 0.0f, 0.5f, 1.0f));
		}
		else if(e.GetButton() == GLFW_MOUSE_BUTTON_RIGHT)
		{
			//QUACK_LOG("Right mouse button pressed!!");

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
		//	glm::vec3 floorPos = glm::vec3(0.f, -0.5f, -5.f);
		//	//glm::vec3 floorPos = glm::vec3(2.f, -0.5f, -5.f);
		//	floor.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), floorPos));
		//	floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		//	floor.AddComponent<ConstraintComponent>(floorPos, floorHalfSize, 0.f, glm::vec3(0.f, 0.f, 1.f));
		//}
		{
			Entity floor = scene->CreateEntity();
			glm::vec3 floorPos = glm::vec3(-3.5f, -5.5f, -5.f);
			floor.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), floorPos));
			floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
			floor.AddComponent<ConstraintComponent>(floorPos, floorHalfSize, -40.f, glm::vec3(0.f, 0.f, 1.f));
		}
		{
			Entity floor = scene->CreateEntity();
			glm::vec3 floorPos = glm::vec3(2.f, -9.5f, -5.f);
			floor.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), floorPos));
			floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
			floor.AddComponent<ConstraintComponent>(floorPos, floorHalfSize, 50.f, glm::vec3(0.f, 0.f, 1.f));
		}
		{
			Entity floor = scene->CreateEntity();
			glm::vec3 floorPos = glm::vec3(-3.5f, -13.5f, -5.f);
			floor.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), floorPos));
			floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
			floor.AddComponent<ConstraintComponent>(floorPos, floorHalfSize, -40.f, glm::vec3(0.f, 0.f, 1.f));
		}



		// WHY WHEN I CHANGE THE ORDER COLLISION IS NOT DETECED :SOB:
		{
			Entity entity = scene->CreateEntity();
			glm::vec3 position = glm::vec3(1.f, 0.f, -5.f);
			entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), position));
			auto& p = entity.AddComponent<PhysicsComponent>(position, 1.f, glm::vec3(3.0f, 0.2f, 0.5f));
			p.velocity = glm::vec3(0.f, 0.f, 0.f);
			p.gravity = glm::vec3(0.f);
			entity.AddComponent<CollisionComponent>(glm::vec3(3.0f, 0.2f, 0.5f));
			entity.AddComponent<ShapeComponent>(new NormalCube(glm::vec3(3.0f, 0.2f, 0.5f)), glm::vec4(0.f, 0.f, 1.f, 1.f));
		}
		{
			Entity entity = scene->CreateEntity();
			glm::vec3 position = glm::vec3(-0.7f, 3.f, -5.f);
			entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), position));
			auto& p = entity.AddComponent<PhysicsComponent>(position, 1.f, glm::vec3(0.5f)/*, -50.f, glm::vec3(0.f, 0.f, 1.f)*/);
			//p.velocity = glm::vec3(0.f, -5.f, 0.f);
			//p.gravity = glm::vec3(0.f);
			entity.AddComponent<CollisionComponent>(glm::vec3(0.5f));
			entity.AddComponent<ShapeComponent>(new NormalCube(), glm::vec4(0.f, 0.f, 1.f, 1.f));
		}
		// Stack of spheres
		//float z = -4.f;
		//glm::mat4 transform;
		//glm::vec4 sphereColor = glm::vec4(0.0f, 1.0f, 0.5f, 1.f);
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
		//			transform = glm::translate(glm::mat4(1.0f), position);
		//			transform = glm::scale(transform, glm::vec3(0.5f));
		//			entity.AddComponent<TransformComponent>(transform);
		//			entity.AddComponent<PhysicsComponent>(position);
		//			entity.AddComponent<CollisionComponent>(0.25f);
		//			entity.AddComponent<ModelComponent>(ModelLibrary::sphere.get(), sphereColor);
		//			x += 0.5f;
		//		}
		//		y +=0.5f;
		//	}
		//	z -= 0.5f;
		//}

		// Stack of cubes
		//float z = -4.f;
		//glm::mat4 transform;
		//glm::vec4 cubeColor = glm::vec4(0.0f, 1.0f, 0.5f, 1.f);
		//for (int i = 0; i < 1; i++)
		//{
		//	float y = 2.f;
		//	for (int j = 0; j < 5; j++)
		//	{
		//		float x = -1.f;
		//		for (int k = 0; k < 2; k++)
		//		{
		//			Entity entity = scene->CreateEntity();
		//			glm::vec3 position = glm::vec3(x, y, z);
		//			transform = glm::translate(glm::mat4(1.0f), position);
		//			transform = glm::scale(transform, glm::vec3(0.5f));
		//			entity.AddComponent<TransformComponent>(transform);
		//			entity.AddComponent<PhysicsComponent>(position, 1.f, glm::vec3(0.5f));
		//			entity.AddComponent<CollisionComponent>(glm::vec3(0.25f));
		//			entity.AddComponent<ShapeComponent>(new NormalCube(), cubeColor);
		//			x += 0.6f;
		//		}
		//		y += 0.6f;
		//	}
		//	z -= 0.6f;
		//}

		//entity.AddComponent<PhysicsComponent>(position, 1.f, glm::vec3(0.5f), 21.37f, glm::vec3(0.f, 0.f, 1.f));

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
				projection = glm::perspective(glm::radians(camera->fov), (float)width / (float)height, 0.1f, 100.0f);

				Renderer::linesShader->Bind();
				Renderer::linesShader->SetUniform4fm("view", glm::value_ptr(view));
				Renderer::linesShader->SetUniform4fm("projection", glm::value_ptr(projection));
				
				Renderer::DrawDebug();

				shader.Bind();
				shader.SetUniform4fm("view", glm::value_ptr(view));
				shader.SetUniform4fm("projection", glm::value_ptr(projection));

				shader.SetUniform3fv("viewPos", glm::value_ptr(camera->position));

				// Physics for entities
				ApplyForces(scene);
				Update(scene, dt); // Update physics
				SolveConstraints(scene);
				SolveCollision(scene);
				UpdateTransform(scene); // Update transformComp with position & rotation from physicsComp after physics simulation


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
