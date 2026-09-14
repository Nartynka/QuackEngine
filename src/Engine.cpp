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
	void MathPrimer(const std::shared_ptr<Scene> scene);

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

	int randRange(int min, int max)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(min, max);
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
		if (!scene->isPaused)
		{
			if (e.GetButton() == GLFW_MOUSE_BUTTON_LEFT)
			{
				//Entity entity = scene->CreateEntity();
				//glm::vec3 position = glm::vec3(randX(), 5.f, randZ());
				//entity.AddComponent<TransformComponent>(position, 21.37f, glm::vec3(0.f, 0.f, 1.f));
				//entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.5f));
				//entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
				//entity.AddComponent<ShapeComponent>(new NormalCube(), glm::vec4(0.5f, 0.0f, 0.5f, 1.0f));

				// Cube
				Entity entity = scene->CreateEntity();
				glm::vec3 position = glm::vec3(0.0f, 5.f, -5.f);
				glm::vec3 halfSize = glm::vec3(0.5f);
				entity.AddComponent<TransformComponent>(position/*, 60.f, glm::vec3(0.f, 0.f, 1.f)*/);
				entity.AddComponent<RigidBodyComponent>(1.f, halfSize);
				entity.AddComponent<ColliderComponent>(halfSize);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(1.f, 0.f, 1.f, 1.f));
			}
			else if(e.GetButton() == GLFW_MOUSE_BUTTON_MIDDLE)
			{
				// Sphere
				Entity entity = scene->CreateEntity();
				glm::vec3 position = glm::vec3(0.0f, 5.f, -5.f);
				entity.AddComponent<TransformComponent>(position);
				entity.AddComponent<RigidBodyComponent>(1.f, 0.5f);
				entity.AddComponent<ColliderComponent>(0.5f);
				entity.AddComponent<ShapeComponent>(new Sphere(0.5f), glm::vec4(1.f, 0.f, 1.f, 1.f));
			}
		}
	}

	void Engine::OnKeyPressed(const KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == GLFW_KEY_F)
			scene->isWireframeMode = !scene->isWireframeMode;

		if (e.GetKeyCode() == GLFW_KEY_T)
			scene->bWarmStart = !scene->bWarmStart;

		if (e.GetKeyCode() == GLFW_KEY_Y)
			scene->bPositionalCorrection = !scene->bPositionalCorrection;

		if (e.GetKeyCode() == GLFW_KEY_P)
			scene->isPaused = !scene->isPaused;

		if (e.GetKeyCode() == GLFW_KEY_L)
		{
			scene->isPaused = true;
			scene->shouldDoOneStep = true;
		}


		switch (e.GetKeyCode())
		{
		case GLFW_KEY_1:
			scene->SpawnFloorScene();
			break;
		case GLFW_KEY_2:
			scene->SpawnTestScene();
			break;
		case GLFW_KEY_3:
			scene->SpawnTiltedFloorsScene();
			break;
		case GLFW_KEY_4:
			scene->SpawnCribbingTowerScene();
			break;
		case GLFW_KEY_5:
			scene->SpawnPyramidScene();
			break;
		default:
			break;
		}


		if (e.GetKeyCode() == GLFW_KEY_6) 
		{
			scene->ClearEntities();

			glm::vec3 halfSize = glm::vec3(0.5f);
			glm::vec3 position = glm::vec3(0.f, 0.f, -5.f);
			
			
			{
				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position, 30.f, glm::vec3(1.f, 0.f, 0.f));
				entity.AddComponent<ColliderComponent>(halfSize);
				//entity.AddComponent<RigidBodyComponent>(1.f, halfSize);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize));

				position.y = 5.f;
				Entity entity2 = scene->CreateEntity();
				entity2.AddComponent<TransformComponent>(position, 30.f, glm::vec3(0.f, 0.f, 1.f));
				entity2.AddComponent<ColliderComponent>(halfSize);
				entity2.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity2.AddComponent<ShapeComponent>(new NormalCube(halfSize));
			}
			
			{
				position.x = -4.f;
				position.y = 0.f;
				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position, 45.f, glm::vec3(0.f, 0.f, 1.f));
				entity.AddComponent<ColliderComponent>(halfSize);
				//auto& r = entity.AddComponent<RigidBodyComponent>(1000.f, halfSize, 0.f);
				//r.gravity = glm::vec3(0.f);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize));

				position.y = 5.f;
				Entity entity2 = scene->CreateEntity();
				entity2.AddComponent<TransformComponent>(position, 45.f, glm::vec3(1.f, 0.f, 0.f));
				entity2.AddComponent<ColliderComponent>(halfSize);
				entity2.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity2.AddComponent<ShapeComponent>(new NormalCube(halfSize));
			}
			{
				position.x = 4.f;
				position.y = 0.f;
				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position, 90.f, glm::vec3(0.333333f, 0.f, 0.5f));
				entity.AddComponent<ColliderComponent>(halfSize);
				//entity.AddComponent<RigidBodyComponent>(1.f, halfSize);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize));

				position.y = 5.f;
				position.x = 3.8f;
				position.z = -4.5f;
				Entity entity2 = scene->CreateEntity();
				entity2.AddComponent<TransformComponent>(position, 45.f, glm::vec3(1.f, 0.44f, 0.f));
				entity2.AddComponent<ColliderComponent>(halfSize);
				entity2.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity2.AddComponent<ShapeComponent>(new NormalCube(halfSize));
			}
		}
		else if (e.GetKeyCode() == GLFW_KEY_7)
		{
			scene->ClearEntities();
			scene->SpawnFloor();

			glm::vec3 halfSize1 = glm::vec3(0.6f, 0.6f, 2.f);
			glm::vec3 halfSize2 = glm::vec3(0.5f, 0.1f, 3.f);
			glm::vec4 cubeColor = glm::vec4(0.1f, 0.5f, 0.7f, 1.f);

			glm::vec3 position = glm::vec3(-0.5f, 0.f, -5.f);
			glm::vec3 position2 = glm::vec3(position.x, position.y + halfSize2.y * 2 + halfSize1.y+0.1f, position.z);

			Entity entity = scene->CreateEntity();
			entity.AddComponent<TransformComponent>(position, 45.f, glm::vec3(0.f,0.f,1.f));
			//entity.AddComponent<RigidBodyComponent>(1.f, halfSize1);
			entity.AddComponent<ColliderComponent>(halfSize1);
			entity.AddComponent<ShapeComponent>(new NormalCube(halfSize1), cubeColor);

			Entity side = scene->CreateEntity();
			side.AddComponent<TransformComponent>(position2, 90.f, glm::vec3(0.f, 1.f, 0.f));
			side.AddComponent<RigidBodyComponent>(0.5f, halfSize2, 0.f, 1.f);
			side.AddComponent<ColliderComponent>(halfSize2);
			side.AddComponent<ShapeComponent>(new NormalCube(halfSize2), cubeColor);

			Entity cube = scene->CreateEntity();
			cube.AddComponent<TransformComponent>(glm::vec3(position.x+2.5f, position2.y + 0.26f, position2.z ));
			cube.AddComponent<ColliderComponent>(glm::vec3(0.25f));
			cube.AddComponent<RigidBodyComponent>(0.01f, glm::vec3(0.25f), 0.f, 1.f);
			cube.AddComponent<ShapeComponent>(new NormalCube(glm::vec3(0.25f)));

			Entity sphere = scene->CreateEntity();
			sphere.AddComponent<TransformComponent>(glm::vec3(position.x - 2.5f, position2.y + 15.f, position2.z));
			sphere.AddComponent<ColliderComponent>(0.5f);
			sphere.AddComponent<RigidBodyComponent>(50.f, 0.5f, 0.f);
			sphere.AddComponent<ShapeComponent>(new Sphere(0.5f));
		}

		else if (e.GetKeyCode() == GLFW_KEY_8)
		{
			scene->ClearEntities();
			scene->SpawnFloor();

			glm::vec3 halfSize = glm::vec3(0.5f);
			glm::vec3 position = glm::vec3(0.f, 0.5f, -3.f);

			{
				position.x = -2.f;
				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position);
				entity.AddComponent<ColliderComponent>(halfSize);
				entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(1.f, 0.5f, 1.f, 1.f));
			}
			{
				position.z = -6.f;
				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position);
				entity.AddComponent<ColliderComponent>(halfSize);
				entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(0.5f, 1.f, 0.5f, 1.f));
			}
			{
				position.x = 2.f;
				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position);
				entity.AddComponent<ColliderComponent>(halfSize);
				entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(0.5f, 0.5f, 1.f, 1.f));
			}
			{
				position.z = -3.f;
				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position);
				entity.AddComponent<ColliderComponent>(halfSize);
				entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(1.f, 1.f, 0.5f, 1.f));
			}

			{
				glm::vec3 halfSize = glm::vec3(2.f, 0.2f, 1.5f);
				glm::vec3 position = glm::vec3(0.f, 1.5f, -4.5f);

				Entity entity = scene->CreateEntity();
				entity.AddComponent<TransformComponent>(position);
				entity.AddComponent<ColliderComponent>(halfSize);
				entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(0.f, 1.f, 1.f, 1.f));
			}
		}
		else if (e.GetKeyCode() == GLFW_KEY_9)
		{
			float gap = 0.1f;
			glm::vec3 halfSize = glm::vec3(0.3f, 0.2f, 0.3f*3 + gap);
			glm::vec3 axis = glm::vec3(0.f, 1.f, 0.f);

			glm::vec4 colors[9] = { 
				glm::vec4(1.f, 0.5f, 0.5f, 1.f),
				glm::vec4(0.5f, 1.f, 0.5f, 1.f),
				glm::vec4(0.5f, 0.5f, 1.f, 1.f),

				glm::vec4(0.5f, 1.f, 1.f, 1.f),
				glm::vec4(1.f, 0.5f, 1.f, 1.f),
				glm::vec4(1.f, 1.f, 0.5f, 1.f),

				glm::vec4(0.2f, 0.7f, 1.f, 1.f),
				glm::vec4(1.f, 0.2f, 0.7f, 1.f),
				glm::vec4(0.7f, 1.f, 0.2f, 1.f)
			};


			// jenga
			for (int i = 0; i < 10; i++)
			{
				glm::vec3 position = glm::vec3(0.f, 0.22f, -4.f);
				position.y += halfSize.y * 2 * i+gap;
				if (i % 2)
				{
					{
						position.z += -halfSize.x * 2 - gap;
						Entity entity = scene->CreateEntity();
						entity.AddComponent<TransformComponent>(position, 90.f, axis);
						entity.AddComponent<ColliderComponent>(halfSize);
						if(i>0)
							entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
						entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), colors[randRange(0, 9)]);
					}
					{
						position.z += halfSize.x * 2 + gap;
						Entity entity = scene->CreateEntity();
						entity.AddComponent<TransformComponent>(position, 90.f, axis);
						entity.AddComponent<ColliderComponent>(halfSize);
						if (i > 0)
							entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
						entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), colors[randRange(0, 9)]);
					}
					{
						position.z += halfSize.x * 2 + gap;
						Entity entity = scene->CreateEntity();
						entity.AddComponent<TransformComponent>(position, 90.f, axis);
						entity.AddComponent<ColliderComponent>(halfSize);
						if (i > 0)
							entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
						entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), colors[randRange(0, 9)]);
					}

				}
				else
				{
					{
						position.x = -halfSize.x * 2 - gap;
						Entity entity = scene->CreateEntity();
						entity.AddComponent<TransformComponent>(position);
						entity.AddComponent<ColliderComponent>(halfSize);
						if (i > 0)
							entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
						entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), colors[randRange(0, 9)]);
					}
					{
						position.x += halfSize.x * 2 + gap;
						Entity entity = scene->CreateEntity();
						entity.AddComponent<TransformComponent>(position);
						entity.AddComponent<ColliderComponent>(halfSize);
						if (i > 0)
							entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
						entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), colors[randRange(0, 9)]);
					}
					{
						position.x = halfSize.x * 2 + gap;
						Entity entity = scene->CreateEntity();
						entity.AddComponent<TransformComponent>(position);
						entity.AddComponent<ColliderComponent>(halfSize);
						if (i > 0)
							entity.AddComponent<RigidBodyComponent>(1.f, halfSize, 0.f);
						entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), colors[randRange(0, 9)]);
					}

				}
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
				scene->dt = dt;

				view = camera->GetView();
				auto [width, height] = window->GetWindowSize();

				// @TODO: width and height can be 0 and cause a crash
				projection = camera->GetProjection(width, height);

				Renderer::linesShader->Bind();
				Renderer::linesShader->SetUniform4fm("view", glm::value_ptr(view));
				Renderer::linesShader->SetUniform4fm("projection", glm::value_ptr(projection));
				
				//Renderer::DrawDebug();

				//MathPrimer(scene);

				shader.Bind();
				shader.SetUniform4fm("view", glm::value_ptr(view));
				shader.SetUniform4fm("projection", glm::value_ptr(projection));

				shader.SetUniform3fv("viewPos", glm::value_ptr(camera->position));

				if (!scene->isPaused || scene->shouldDoOneStep)
				{
					// Physics for entities
					ApplyForces(scene);
					Update(scene, dt); // Update physics
					//UpdateConstraints(scene, dt);
					CheckCollisions(scene);
					SolveCollisions(scene);
					UpdateTransform(scene); // Update transformComp with position & rotation from rigidBodyComp after physics simulation
				}

				// Render entities
				if (!scene->isWireframeMode)
					RenderShapes(scene, shader);
				RenderModels(scene, shader);
				RenderCollisionShapes(scene);


				// Render light cube
				//model = glm::translate(glm::mat4(1.0f), lightCube.position);
				//lightCube.shader->Bind();
				//lightCube.shader->SetUniform4fm("MVP", glm::value_ptr(projection * view * model));
				//Renderer::Draw(*lightCube.shape->vao, *lightCube.shape->ibo, *lightCube.shader);

				scene->shouldDoOneStep = false;

				ui->EndFrame();
				window->Update();
			}
		}
	}
}
