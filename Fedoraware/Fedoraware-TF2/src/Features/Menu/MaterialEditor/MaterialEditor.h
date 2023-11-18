#pragma once
#include "../../Feature.h"
#include <ImGui/TextEditor.h>

class CMaterialEditor
{
	void MainWindow();
	void EditorWindow();

	TextEditor TextEditor;

	bool EditorOpen = false;
	std::string CurrentMaterial;
	bool LockedMaterial;

public:
	void Render();

	bool IsOpen = false;
};

ADD_FEATURE(CMaterialEditor, MaterialEditor)