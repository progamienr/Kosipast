#include "MaterialEditor.h"

#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_stdlib.h>

#include "../ConfigManager/ConfigManager.h"
#include "../../Visuals/Materials/Materials.h"

void CMaterialEditor::MainWindow()
{
	if (!IsOpen)
		return;

	using namespace ImGui;

	SetNextWindowSize(ImVec2(400, 380), ImGuiCond_Once);
	if (Begin("Material Manager", &IsOpen, ImGuiWindowFlags_NoCollapse))
	{
		// Toolbar
		{
			if (Button("Edit"))
			{
				TextEditor.SetText(F::Materials.GetVMT(CurrentMaterial));
				TextEditor.SetReadOnly(LockedMaterial);
				EditorOpen = true;
			}

			SameLine();
			if (Button("Remove"))
				F::Materials.RemoveMaterial(CurrentMaterial);

			SameLine();
			if (Button("Open Folder"))
				ShellExecuteA(nullptr, "open", MaterialFolder.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
		}

		PushItemWidth(GetWindowSize().x - 2 * GetStyle().WindowPadding.x);

		// New material input
		static std::string newName;
		if (InputTextWithHint("###MaterialName", "New Material name", &newName, ImGuiInputTextFlags_EnterReturnsTrue))
			F::Materials.AddMaterial(newName);

		// Material list
		if (BeginChild("ListChild###Materials"))
		{
			if (BeginListBox("###MaterialList", { GetWindowWidth(), GetWindowHeight() }))
			{
				for (auto const& mat : F::Materials.m_ChamMaterials)
				{
					if (Selectable(mat.sName.c_str(), CurrentMaterial == mat.sName))
					{
						CurrentMaterial = mat.sName;
						LockedMaterial = mat.bLocked;
						EditorOpen = false;
					}
				}

				EndListBox();
			}
		}
		EndChild();

		PopItemWidth();
		End();
	}
}

void CMaterialEditor::EditorWindow()
{
	if (!EditorOpen || !IsOpen)
		return;

	using namespace ImGui;

	SetNextWindowSize(ImVec2(450, 400), ImGuiCond_Once);
	if (Begin("Material Editor", &EditorOpen, ImGuiWindowFlags_NoCollapse))
	{
		// Toolbar
		if (!LockedMaterial)
		{
			if (Button("Save"))
			{
				auto text = TextEditor.GetText();
				text.erase(text.end() - 1, text.end()); // get rid of random newline
				F::Materials.EditMaterial(CurrentMaterial, text);
			}

			SameLine();
			if (Button("Save and close"))
			{
				auto text = TextEditor.GetText();
				text.erase(text.end() - 1, text.end()); // get rid of random newline
				F::Materials.EditMaterial(CurrentMaterial, text);
				EditorOpen = false;
			}

			SameLine();
			if (Button("Cancel"))
				EditorOpen = false;

			Text("Editing: %s", CurrentMaterial.c_str());
		}
		else
		{
			if (Button("Close"))
				EditorOpen = false;

			Text("Showing: %s", CurrentMaterial.c_str());
		}

		// Text editor
		TextEditor.Render("TextEditor");

		End();
	}
}

void CMaterialEditor::Render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(10, 10));
	MainWindow();
	EditorWindow();
	ImGui::PopStyleVar(2);
}