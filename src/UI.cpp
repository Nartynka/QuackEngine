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

		ImGuiStyle& style = ImGui::GetStyle();
		style.FontSizeBase = 20.0f;
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.5f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.5f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.01f, 0.01f, 0.01f, 0.5f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
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

		ImGui::SeparatorText("Shape type:");

		const char* shapeType[] = { "Sphere", "Cube" };
		static int shapeIdx = 0;

		RenderCombo("Shape", shapeType, IM_ARRAYSIZE(shapeType), shapeIdx);

		static float radius = 1.f;
		static float halfSize[3] = { 0.5f, 0.5f, 0.5f };

		if (shapeIdx == 0) // Sphere
		{
			ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, 0.0f, "%.2f");
		}
		else if (shapeIdx == 1) // Cube
		{
			ImGui::DragFloat3("Half Size", halfSize, 0.01f, 0.f, 0.f, "%.2f");
		}


		ImGui::SeparatorText("Entity type:");

		static const char* entityType[] = { "Rigid body", "Static body" };
		static int entityTypeIdx = 0;
		RenderCombo("Entity type", entityType, IM_ARRAYSIZE(entityType), entityTypeIdx);

		// Values for physics component
		static float mass = 1.f, bounce = 0.7f, friction = 0.98f, velocity[3];
		static bool bShowAdvanced, bGravity = true;

		if (entityTypeIdx == 0) // Rigid Body
		{
			ImGui::DragFloat("Mass", &mass, 0.01f, 0.0f, 0.0f,"%.2f");
			ImGui::DragFloat("Restitution", &bounce, 0.01f, 0.0f, 0.0f, "%.2f");
			ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 0.f, "%.2f");

			ImGui::Checkbox("Advanced", &bShowAdvanced);

			if (bShowAdvanced)
			{
				ImGui::Indent();
				ImGui::Checkbox("Gravity", &bGravity);
				ImGui::DragFloat3("Velocity", velocity, 0.1f, 0.f, 0.f, "%.2f");
				ImGui::Unindent();
			}

		}

		ImGui::SeparatorText("World orientation:");

		static float position[3] = {0.f, 5.f, -5.f};
		ImGui::DragFloat3("Position", position, 0.1f, 0.f, 0.f, "%.2f");

		static float angle, axis[3];
		ImGui::DragFloat("Angle", &angle, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat3("Rotation axis", axis, 0.1f, 0.f, 0.f, "%.2f");

		static float color[3] = { 0.5f, 0.0f, 0.5f }; // default color
		ImGui::Text("Color:"); ImGui::SameLine();
		ImGui::ColorEdit3("Color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		if (ImGui::Button("Create", { -FLT_MIN, 30.0f }))
		{
			Entity entity = context->CreateEntity();

			glm::vec3 position_v = glm::vec3(position[0], position[1], position[2]);
			glm::vec3 halfSize_v = glm::vec3(halfSize[0], halfSize[1], halfSize[2]);
			glm::vec3 axis_v = glm::vec3(axis[0], axis[1], axis[2]);
			glm::vec4 color_v = glm::vec4(color[0], color[1], color[2], 1.f);

			entity.AddComponent<TransformComponent>(position_v, angle, axis_v);

			if (shapeIdx == 0) // Sphere
			{
				entity.AddComponent<ColliderComponent>(radius);
				entity.AddComponent<ShapeComponent>(new Sphere(radius), color_v);
			}
			else if (shapeIdx == 1) // Cube
			{
				entity.AddComponent<ColliderComponent>(halfSize_v);
				entity.AddComponent<ShapeComponent>(new NormalCube(halfSize_v), color_v);
			}

			RigidBodyComponent* rigidBody = nullptr;

			if (entityTypeIdx == 0) // Rigid body
			{
				if (shapeIdx == 0) // Sphere
				{
					rigidBody = &entity.AddComponent<RigidBodyComponent>(mass, radius, bounce, friction);
				}
				else if (shapeIdx == 1) // Cube
				{
					rigidBody = &entity.AddComponent<RigidBodyComponent>(mass, halfSize_v, bounce, friction);
				}

				if (bShowAdvanced && rigidBody)
				{
					if (!bGravity)
						rigidBody->gravity = glm::vec3(0.f);

					rigidBody->velocity = glm::vec3(velocity[0], velocity[1], velocity[2]);
				}
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
