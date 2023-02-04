#include "ImGuiExtension.h"
#include "imgui_internal.h"
#include "Logger.h"
#include "../../Framework/Include/FWindow.h"

ImGui::Flavo& ImGui::Flavo::getInstance() {
	static Flavo instance;
	return instance;
}

void ImGui::Flavo::init(Vector2 windowSize) {
	loadAllFonts();

	//set universal params
	invisibleWindowFlags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
	this->windowSize = ImVec2(
		windowSize.x,
		windowSize.y
	);

	if (!framework::FWindow::getInstance().isEditorWindow()) {
		//set palette
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowBorderSize = 0.0f;
		style.FramePadding = ImVec2(0.0f, 0.0f);
		style.WindowPadding = ImVec2(0.0f, 0.0f);
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = 1.0f;
	}
}

void ImGui::Flavo::loadAllFonts() {
	ImGuiIO& io = ImGui::GetIO();
	fontMap.insert({ FontType::CALIBRI_REGULAR_16, io.Fonts->AddFontFromFileTTF("../Data/Fonts/calibri.ttf", 16.0f) });
	fontMap.insert({ FontType::CALIBRI_REGULAR_20, io.Fonts->AddFontFromFileTTF("../Data/Fonts/calibri.ttf", 20.0f) });
	fontMap.insert({ FontType::CALIBRI_REGULAR_26, io.Fonts->AddFontFromFileTTF("../Data/Fonts/calibri.ttf", 26.0f) });
	fontMap.insert({ FontType::CALIBRI_BOLD_26, io.Fonts->AddFontFromFileTTF("../Data/Fonts/calibrib.ttf", 26.0f) });
}

bool ImGui::CheckboxNoLabel(const char* label, bool* v) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2, label_size.y + style.FramePadding.y * 2)); // We want a square shape to we use Y twice
	ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;
	if (label_size.x > 0)
		SameLine(0, style.ItemInnerSpacing.x);
	const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
	if (label_size.x > 0) {
		ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	RenderNavHighlight(total_bb, id);
	RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
	if (*v) {
		const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
		const float pad = ImMax(1.0f, (float)(int)(check_sz / 6.0f));
		RenderCheckMark(check_bb.Min + ImVec2(pad, pad), GetColorU32(ImGuiCol_CheckMark), check_bb.GetWidth() - pad * 2.0f);
	}

	return pressed;
}