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

		//window = std::make_unique<Window>(1280, 720);
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

			dispatcher.Dispatch<KeyPressedEvent>(std::bind(&Engine::OnKeyPressed, this, std::placeholders::_1));
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
			//Entity entity = scene->CreateEntity();

			//glm::vec3 position = glm::vec3(randX(), 5.f, randZ());
			//entity.AddComponent<TransformComponent>(position, 21.37f, glm::vec3(0.f, 0.f, 1.f));
			//entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.5f));
			//entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
			//entity.AddComponent<ShapeComponent>(new NormalCube(), glm::vec4(0.5f, 0.0f, 0.5f, 1.0f));

			Entity entity = scene->CreateEntity();
			glm::vec3 position = glm::vec3(0.0f, 5.f, -5.f);
			glm::vec3 halfSize = glm::vec3(0.5f);
			entity.AddComponent<TransformComponent>(position, 60.f, glm::vec3(0.f, 0.f, 1.f));
			entity.AddComponent<RigidBodyComponent>(1.f, halfSize);
			entity.AddComponent<ColliderComponent>(halfSize);
			entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(1.f, 0.f, 1.f, 1.f));
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

	void Engine::OnKeyPressed(const KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == GLFW_KEY_1)
		{
			scene->ClearEntities();
			scene->SpawnFloorScene();
		}
		else if (e.GetKeyCode() == GLFW_KEY_2)
		{
			scene->ClearEntities();
			scene->SpawnTestScene();
		}
		else if (e.GetKeyCode() == GLFW_KEY_3)
		{
			scene->ClearEntities();
			scene->SpawnTiltedFloorsScene();
		}
		else if (e.GetKeyCode() == GLFW_KEY_4)
		{
			scene->SpawnCribbingTower();
		}
		else if (e.GetKeyCode() == GLFW_KEY_5)
		{
			glm::vec3 halfSize = glm::vec3(0.1f, 0.1f, 1.f);
			glm::vec3 position = glm::vec3(-1.f, 2.f, -5.f);
			Entity entity = scene->CreateEntity();
			entity.AddComponent<TransformComponent>(position);
			entity.AddComponent<RigidBodyComponent>(1.f, halfSize);
			entity.AddComponent<ColliderComponent>(halfSize);
			entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(0.1f, 0.5f, 0.7f, 1.f));
		}
		else if (e.GetKeyCode() == GLFW_KEY_6)
		{
			// @TODO: In future this will be other constraints scene (e.g. hinges, joints, rope, PBD?)
			/*
			//Entity sphere1 = scene->CreateEntity();
			//{
			//	glm::vec3 position = glm::vec3(0.f, 0.f, -5.f);
			//	float radius = 1.f;
			//	sphere1.AddComponent<TransformComponent>(position);
			//	//auto& rigidBodyComp = sphere1.AddComponent<RigidBodyComponent>(1.f, radius);
			//	//rigidBodyComp.gravity = glm::vec3(0.f);
			//	sphere1.AddComponent<ColliderComponent>(radius);
			//	sphere1.AddComponent<ShapeComponent>(new Sphere(radius), glm::vec4(1.f, 0.f, 1.f, 1.f));
			//}

			//Entity sphere2 = scene->CreateEntity();
			//{
			//	glm::vec3 position = glm::vec3(-2.f, 0.f, -5.f);
			//	float radius = 1.f;
			//	sphere2.AddComponent<TransformComponent>(position);
			//	auto& rigidBodyComp = sphere2.AddComponent<RigidBodyComponent>(1.f, radius);
			//	//rigidBodyComp.gravity = glm::vec3(0.f);
			//	sphere2.AddComponent<ColliderComponent>(radius);
			//	sphere2.AddComponent<ShapeComponent>(new Sphere(radius), glm::vec4(1.f, 0.f, 1.f, 1.f));
			//}

			//Entity sphere3 = scene->CreateEntity();
			//{
			//	glm::vec3 position = glm::vec3(-4.f, 0.f, -5.f);
			//	float radius = 1.f;
			//	sphere3.AddComponent<TransformComponent>(position);
			//	auto& rigidBodyComp = sphere3.AddComponent<RigidBodyComponent>(1.f, radius);
			//	//rigidBodyComp.gravity = glm::vec3(0.f);
			//	sphere3.AddComponent<ColliderComponent>(radius);
			//	sphere3.AddComponent<ShapeComponent>(new Sphere(radius), glm::vec4(1.f, 0.f, 1.f, 1.f));
			//}

			//Entity entity = scene->CreateEntity();
			//entity.AddComponent<ElasticConstraintComponent>(&sphere1, &sphere2, 3.f);

			//Entity entity2 = scene->CreateEntity();
			//entity2.AddComponent<ElasticConstraintComponent>(&sphere2, &sphere3, 3.f);
			*/
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


		scene->SpawnFloorScene();


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

				view = camera->GetView();
				auto [width, height] = window->GetWindowSize();

				// @TODO: width and height can be 0 and cause a crash
				projection = camera->GetProjection(width, height);

				Renderer::linesShader->Bind();
				Renderer::linesShader->SetUniform4fm("view", glm::value_ptr(view));
				Renderer::linesShader->SetUniform4fm("projection", glm::value_ptr(projection));
				
				//Renderer::DrawDebug();

				shader.Bind();
				shader.SetUniform4fm("view", glm::value_ptr(view));
				shader.SetUniform4fm("projection", glm::value_ptr(projection));

				shader.SetUniform3fv("viewPos", glm::value_ptr(camera->position));


				//auto& r = sphere2.GetComponent<RigidBodyComponent>();
				//r.velocity.x = sin(glfwGetTime()) * 30.f;
				//r.velocity.z = cos(glfwGetTime()) * 30.f;

				// Physics for entities
				ApplyForces(scene);
				Update(scene, dt); // Update physics
				//UpdateConstraints(scene, dt);
				CheckCollisions(scene);
				SolveCollisions();
				UpdateTransform(scene); // Update transformComp with position & rotation from rigidBodyComp after physics simulation


				// Render entities
				RenderShapes(scene, shader);
				RenderModels(scene, shader);
				RenderCollisionShapes(scene);


				// Render light cube
				//model = glm::translate(glm::mat4(1.0f), lightCube.position);
				//lightCube.shader->Bind();
				//lightCube.shader->SetUniform4fm("MVP", glm::value_ptr(projection * view * model));
				//Renderer::Draw(*lightCube.shape->vao, *lightCube.shape->ibo, *lightCube.shader);

				ui->EndFrame();
				window->Update();
			}
		}
	}
}
