#pragma once

#include "imgui.h"
#include <unordered_map>

//Enable ImVecX operators
#define IMGUI_DEFINE_MATH_OPERATORS

#define RELX(a) (ImGui::Flavo::getInstance().windowSize.x * a)
#define RELY(b) (ImGui::Flavo::getInstance().windowSize.y * b)
#define REL(a,b) (ImVec2(RELX(a), RELY(b)))

//ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
#define INVISIBLE() (ImGui::Flavo::getInstance().invisibleWindowFlags)

namespace DirectX {
	namespace SimpleMath {
		struct Vector2;
	}
}

namespace ImGui
{
	class Flavo
	{
	public:
		enum class FontType;
		static Flavo& getInstance();
		void init(DirectX::SimpleMath::Vector2 windowSize);

		std::unordered_map<FontType, ImFont*> fontMap;
		ImVec2 windowSize;
		ImGuiWindowFlags invisibleWindowFlags;

	private:
		void loadAllFonts();
	};

	enum class Flavo::FontType
	{
		CALIBRI_REGULAR_16,
		CALIBRI_REGULAR_20,
		CALIBRI_REGULAR_26,

		CALIBRI_BOLD_26
	};

	bool CheckboxNoLabel(const char* label, bool* v);
}
