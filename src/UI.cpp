#include "UI.h"

#include "Window.h"
#include "Scene.h"
#include "Entity.h"
#include "ModelLibrary.h"
#include "Assert.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>


namespace Quack
{
	UI::UI(Window* window, Scene* scene) : context(scene)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui_ImplGlfw_InitForOpenGL(window->GetWindow(), true);
		ImGui_ImplOpenGL3_Init();
	}

	UI::~UI()
	{
		Shutdown();
	}

	void UI::StartFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		RenderStats();
		RenderMenu();

		// Last created window by imgui takes the focus and we don't want that
		static bool isFirstFrame = true;
		if (isFirstFrame)
		{
			ImGui::SetWindowFocus(nullptr);
			isFirstFrame = false;
		}
	}

	void UI::EndFrame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void UI::RenderStats()
	{
		ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::SeparatorText("Stats:");

		ImGui::Text("Entity count: %i", context->GetEntityCount());

		ImGui::End();
	}

	void UI::RenderMenu()
	{
		ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		//ImGui::SetWindowFocus(nullptr);
		ImGui::SeparatorText("Entity type:");

		static const char* entityType[] = { "Particle", "Constraint"};
		static int entityTypeIdx = 0;
		RenderCombo("Entity type", entityType, IM_ARRAYSIZE(entityType), entityTypeIdx);

		// Values for physics component
		static float mass = 1.f, bounce = 0.7f, friction = 0.98f;

		static int shapeIdx = 0;
		static float radius = 0.5f;
		static float halfSize[3] = {0.5f, 0.5f, 0.5f};

		if (entityTypeIdx == 0) // Particle
		{
			ImGui::DragFloat("Mass", &mass, 0.01f, 0.0f, 0.0f,"%.2f");
			ImGui::DragFloat("Restitution (bounce)", &bounce, 0.01f, 0.0f, 0.0f, "%.2f");
			ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 0.f, "%.2f");

			const char* shapeType[] = { "Sphere", "Cube" };

			RenderCombo("Shape", shapeType, IM_ARRAYSIZE(shapeType), shapeIdx);

			if (shapeIdx == 0) // Sphere
			{
				ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, 0.0f, "%.2f");
			}
			else if (shapeIdx == 1) // Cube
			{
				ImGui::DragFloat3("Half Size", halfSize, 0.01f, 0.f, 0.f, "%.2f");
			}
		}
		else if (entityTypeIdx == 1) // Constraint
		{
			// for now we only support cube constraints
			ImGui::DragFloat3("Half Size", halfSize, 0.01f, 0.f, 0.f, "%.2f");
		}

		ImGui::SeparatorText("World orientation:");

		static float position[3];
		ImGui::DragFloat3("Position", position, 0.1f, 0.f, 0.f, "%.2f");

		static float angle, axis[3];
		ImGui::DragFloat("Angle", &angle, 0.1f, 0.0f, 1.0f, "%.2f");
		ImGui::DragFloat3("Rotation axis", axis, 0.1f, 0.f, 0.f, "%.2f");
		// @TODO: pass color to entities
		static float outlineColor[3];
		ImGui::Text("Color:"); ImGui::SameLine();
		ImGui::ColorEdit3("Color", outlineColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		if (ImGui::Button("Create", { -FLT_MIN, 30.0f }))
		{
			Entity entity = context->CreateEntity();

			glm::vec3 position_v = glm::vec3(position[0], position[1], position[2]);
			glm::vec3 axis_v = glm::vec3(axis[0], axis[1], axis[2]);

			entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), position_v));

			if (entityTypeIdx == 0) // Particle
			{
				entity.AddComponent<PhysicsComponent>(position_v, mass, bounce, friction, angle, axis_v);

				if (shapeIdx == 0) // Sphere
				{
					// @TODO: ModelLibrary::sphere should take radius, and then pass radius to collision component
					entity.AddComponent<CollisionComponent>(0.5f);
					entity.AddComponent<ModelComponent>(ModelLibrary::sphere.get());
				}
				else if (shapeIdx == 1) // Cube
				{
					glm::vec3 halfSize_v = glm::vec3(halfSize[0], halfSize[1], halfSize[2]);
					entity.AddComponent<CollisionComponent>(halfSize_v);
					entity.AddComponent<ShapeComponent>(new Cube(halfSize_v));
				}
			}
			else if (entityTypeIdx == 1) // Constraint
			{
				glm::vec3 halfSize_v = glm::vec3(halfSize[0], halfSize[1], halfSize[2]);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize_v));
				entity.AddComponent<ConstraintComponent>(position_v, halfSize_v, angle, axis_v);
			}
		}

		ImGui::End();
	}


	void UI::RenderCombo(const char* label, const char* elements[], int count, int& currentIdx)
	{
		if (ImGui::BeginCombo(label, elements[currentIdx]))
		{
			for (int n = 0; n < count; n++)
			{
				const bool isSelected = (currentIdx == n);
				if (ImGui::Selectable(elements[n], isSelected))
					currentIdx = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	void UI::SetContext(Scene* scene)
	{
		context = scene;
	}

	void UI::Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

}
