#pragma once
#include "ImGui/imgui_internal.h"

namespace ImGui
{
	/* Color_t to ImVec4 */
	inline ImVec4 ColorToVec(Color_t color)
	{
		return { Color::TOFLOAT(color.r), Color::TOFLOAT(color.g), Color::TOFLOAT(color.b), Color::TOFLOAT(color.a) };
	}

	/* ImVec4 to Color_t */
	inline Color_t VecToColor(ImVec4 color)
	{
		return {
			static_cast<byte>(color.x * 256.0f > 255 ? 255 : color.x * 256.0f),
			static_cast<byte>(color.y * 256.0f > 255 ? 255 : color.y * 256.0f),
			static_cast<byte>(color.z * 256.0f > 255 ? 255 : color.z * 256.0f),
			static_cast<byte>(color.w * 256.0f > 255 ? 255 : color.w * 256.0f)
		};
	}

	__inline void HelpMarker(const char* desc)
	{
		if (IsItemHovered())
		{
			if (Vars::Menu::ModernDesign)
			{
				SetTooltip(desc);
			}
			else
			{
				F::Menu.FeatureHint = desc;
			}
		}
	}

	__inline bool IconButton(const char* icon)
	{
		PushFont(F::Menu.IconFont);
		TextUnformatted(icon);
		const bool pressed = IsItemClicked();
		PopFont();
		return pressed;
	}

	__inline void SectionTitle(const char* title, float yOffset = 0)
	{
		Dummy({ 0, yOffset });
		PushFont(F::Menu.SectionFont);
		const ImVec2 titleSize = CalcTextSize(title);
		SetCursorPosX((GetWindowSize().x - titleSize.x) * .5f);
		Text(title);
		PopFont();

		const auto widgetPos = GetCursorScreenPos();
		GradientRect(&F::Menu.MainGradient, { widgetPos.x, widgetPos.y - 2 }, GetColumnWidth(), 3);
	}

	__inline bool TableColumnChild(const char* str_id)
	{
		TableNextColumn();
		float contentHeight = GetWindowHeight() - (F::Menu.TabHeight + GetStyle().ItemInnerSpacing.y);
		return BeginChild(str_id, { GetColumnWidth(), contentHeight }, !Vars::Menu::ModernDesign);
	}

	__inline bool SidebarButton(const char* label, bool active = false)
	{
		if (active) { PushStyleColor(ImGuiCol_Button, ImColor(38, 38, 38).Value); }
		const bool pressed = Button(label, { GetWindowSize().x - 2 * GetStyle().WindowPadding.x, 44.f });
		if (active) { PopStyleColor(); }
		return pressed;
	}

	__inline bool TabButton(const char* label, bool active = false)
	{
		TableNextColumn();
		if (active) { PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive)); }
		const bool pressed = Button(label, { GetColumnWidth(), F::Menu.TabHeight });
		if (active)
		{
			PopStyleColor();

			const auto widgetPos = GetCursorScreenPos();
			GradientRect(&F::Menu.TabGradient, { widgetPos.x, widgetPos.y - 3 }, GetColumnWidth(), 3);
		}
		return pressed;
	}

	__inline bool InputKeybind(const char* label, int& output, bool bAllowNone = true)
	{
		auto VK2STR = [&](const short key) -> std::string
		{
			switch (key)
			{
				case 0x0: return "none";
				case VK_LBUTTON: return "mouse1";
				case VK_RBUTTON: return "mouse2";
				case VK_MBUTTON: return "mouse3";
				case VK_XBUTTON1: return "mouse4";
				case VK_XBUTTON2: return "mouse5";
				case VK_CONTROL:
				case VK_LCONTROL:
				case VK_RCONTROL: return "control";
				case VK_DELETE: return "delete";
				case VK_NUMPAD0: return "num0";
				case VK_NUMPAD1: return "num1";
				case VK_NUMPAD2: return "num2";
				case VK_NUMPAD3: return "num3";
				case VK_NUMPAD4: return "num4";
				case VK_NUMPAD5: return "num5";
				case VK_NUMPAD6: return "num6";
				case VK_NUMPAD7: return "num7";
				case VK_NUMPAD8: return "num8";
				case VK_NUMPAD9: return "num9";
				case VK_PRIOR: return "pgup";
				case VK_NEXT: return "pgdown";
				case VK_HOME: return "home";
				case VK_END: return "end";
				case VK_CLEAR: return "clear";
				case VK_UP: return "up";
				case VK_DOWN: return "down";
				case VK_LEFT: return "left";
				case VK_RIGHT: return "right";
				case VK_ESCAPE: return "escape";
				case VK_F13: return "f13";
				case VK_F14: return "f14";
				case VK_F15: return "f15";
				case VK_F16: return "f16";
				case VK_F17: return "f17";
				case VK_F18: return "f18";
				case VK_F19: return "f19";
				case VK_F20: return "f20";
				case VK_F21: return "f21";
				case VK_F22: return "f22";
				case VK_F23: return "f23";
				case VK_F24: return "f24";
				case VK_PAUSE: return "pause";
			}

			std::string str = "unknown";

			CHAR output[16] = { "\0" };
			if (GetKeyNameTextA(MapVirtualKeyW(key, MAPVK_VK_TO_VSC) << 16, output, 16))
				str = output;

			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());

			return str;
		};

		static bool bCanceled = false;

		const auto id = GetID(label);
		PushID(label);

		G::InKeybind = false;
		if (GetActiveID() == id)
		{
			G::InKeybind = true;

			Button("...", ImVec2(100, 20));
			const bool bHovered = IsItemHovered();

			if (bHovered && IsMouseClicked(ImGuiMouseButton_Left))
			{
				bCanceled = true;
				ClearActiveID();
			}
			else
			{
				SetActiveID(id, GetCurrentWindow());

				for (short key = 0; key < 255; key++)
				{
					if (!(GetAsyncKeyState(key) & 0x8000))
						continue;
					if (key == VK_INSERT || key == VK_F3)
						continue;
					if (bHovered && key == VK_LBUTTON)
						continue;

					output = key;
					if (bAllowNone && key == VK_ESCAPE)
						output = 0x0;
					ClearActiveID();
					break;
				}
			}

			GetCurrentContext()->ActiveIdAllowOverlap = true;
		}
		else if (Button(VK2STR(output).c_str(), ImVec2(100, 20)) && !bCanceled)
			SetActiveID(id, GetCurrentWindow());

		if (bCanceled && !IsMouseDown(ImGuiMouseButton_Left) && !IsMouseReleased(ImGuiMouseButton_Left))
			bCanceled = false;

		SameLine();
		Text("%s", label);
		PopID();

		return true;
	}

	/* Combobox for custom materials */
	__inline bool MaterialCombo(const char* label, std::string* current_mat, ImGuiComboFlags flags = 0)
	{
		bool active = false;
		PushItemWidth(F::Menu.ItemWidth);
		if (BeginCombo(label, current_mat->c_str(), flags))
		{
			for (const auto& [name, mat] : F::MaterialEditor.MaterialMap)
			{
				if (Selectable(name.c_str(), name == *current_mat))
				{
					*current_mat = name;
					active = true;
				}
			}

			EndCombo();
		}
		PopItemWidth();

		return active;
	}

	/* Combobox with multiple selectable items */
	__inline void MultiCombo(std::vector<const char*> titles, std::vector<bool*> options,
							 const std::string& comboName)
	{
		if (titles.size() != options.size()) { return; }

		std::string preview = "<None>##";
		for (size_t i = 0; i < options.size(); i++)
		{
			if (*options[i])
			{
				if (preview == "<None>##") { preview = ""; }
				preview += titles[i];
				preview.append(", ");
			}
		}
		preview.pop_back(); preview.pop_back(); // This is a stupid but easy way to remove the last comma

		PushItemWidth(F::Menu.ItemWidth);
		if (BeginCombo(comboName.c_str(), preview.c_str()))
		{
			for (size_t i = 0; i < titles.size(); i++)
			{
				Selectable((*options[i]) ? tfm::format("+ %s", titles[i]).c_str() : titles[i], options[i], ImGuiSelectableFlags_DontClosePopups);
			}

			EndCombo();
		}
		PopItemWidth();
	}

	__inline void MultiFlags(std::vector<const char*> flagNames, std::vector<int> flagValues, int* flagVar, const std::string& comboName)
	{
		if (flagNames.size() != flagValues.size()) { return; }

		std::string preview = "<Default>##";
		if (*flagVar == 0)
		{
			preview = "<None>##";
		}
		else
		{
			for (size_t i = 0; i < flagValues.size(); i++)
			{
				if (*flagVar & flagValues[i])
				{
					if (preview == "<Default>##") { preview = ""; }
					preview += flagNames[i];
					preview.append(", ");
				}
			}
			preview.pop_back(); preview.pop_back();
		}

		PushItemWidth(F::Menu.ItemWidth);
		if (BeginCombo(comboName.c_str(), preview.c_str()))
		{
			for (size_t i = 0; i < flagNames.size(); i++)
			{
				const bool flagActive = *flagVar & flagValues[i];
				if (Selectable(flagActive ? tfm::format("+ %s", flagNames[i]).c_str() : flagNames[i], flagActive, ImGuiSelectableFlags_DontClosePopups))
				{
					if (flagActive)
					{
						*flagVar &= ~flagValues[i];
					}
					else
					{
						*flagVar |= flagValues[i];
					}
				}
			}

			EndCombo();
		}
		PopItemWidth();
	}

	__inline bool ColorPicker(const char* label, Color_t& color)
	{
		bool open = false;
		ImVec4 tempColor = ColorToVec(color);
		PushItemWidth(F::Menu.ItemWidth);
		if (ColorEdit4(label, &tempColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
		{
			color = VecToColor(tempColor);
			open = true;
		}
		PopItemWidth();
		HelpMarker(label);
		return open;
	}

	/* Inline color picker */
	__inline bool ColorPickerL(const char* label, Color_t& color, int num = 0)
	{
		SameLine(GetContentRegionMax().x - 20 - (num * 24));
		SetNextItemWidth(20);
		return ColorPicker(label, color);
	}

	__inline void TextCentered(const char* fmt)
	{
		const auto windowWidth = GetWindowSize().x;
		const auto textWidth = CalcTextSize(fmt).x;

		SetCursorPosX((windowWidth - textWidth) * 0.5f);
		Text("%s", fmt);
	}

	// Source: https://github.com/ocornut/imgui/issues/1537#issuecomment-355569554
	__inline bool ToggleButton(const char* label, bool* v)
	{
		const auto p = GetCursorScreenPos();
		auto* drawList = GetWindowDrawList();
		const auto& style = GetStyle();

		const float height = GetFrameHeight();
		const float width = height * 1.8f;
		const float radius = height * 0.50f;
		const ImVec2 labelSize = CalcTextSize(label, nullptr, true);

		InvisibleButton(label, ImVec2(width + style.ItemInnerSpacing.x + labelSize.x, height));
		if (IsItemClicked()) { *v = !*v; }

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		constexpr float ANIM_SPEED = 0.08f;
		if (g.LastActiveId == g.CurrentWindow->GetID(label))
		{
			const float tAnim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
			t = *v ? (tAnim) : (1.0f - tAnim);
		}

		const ImU32 colBg = IsItemHovered() ? ImColor(60, 60, 60) : ImColor(50, 50, 50);
		const ImU32 colCircle = (*v) ? F::Menu.Accent : ImColor(180, 180, 180);

		drawList->AddRectFilled(p, ImVec2(p.x + width, p.y + height), colBg, height * 0.5f);
		drawList->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, colCircle);
		RenderText({ p.x + width + style.ItemInnerSpacing.x, p.y + (height / 2 - labelSize.y / 2) }, label);

		return *v;
	}

#pragma region Width Components
	__inline bool WCombo(const char* label, int* current_item, std::vector<const char*> items)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return Combo(label, current_item, items.data(), items.size(), -1);
	}

	__inline bool WSlider(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f", ImGuiSliderFlags flags = 0)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return SliderFloat(label, v, v_min, v_max, format, flags);
	}

	__inline bool WSlider(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return SliderInt(label, v, v_min, v_max, format, flags);
	}

	__inline bool WInputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return InputText(label, str, flags, callback, user_data);
	}

	__inline bool WInputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return InputTextWithHint(label, hint, str, flags, callback, user_data);
	}

	__inline bool WInputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return InputInt(label, v, step, step_fast, flags);
	}

	__inline bool WToggle(const char* label, bool* v)
	{
		bool result;
		//if (Vars::Menu::ModernDesign)
		//{
			//result = ToggleButton(label, v);
		//}
		//else
		//{
			result = Checkbox(label, v);
		//}
		return result;
	}
#pragma endregion
}
