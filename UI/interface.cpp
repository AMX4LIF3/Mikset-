#include "interface.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include <chrono>
#include <iostream>
#include <format> // OMAY GAWD DE LOWD HAS THROWN HIS BLESSINGS UPON US!
#define PI 3.14f

using namespace ImGui;

void Theme_BoringGray() {
	// Boring Gray style from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(20.0f, 20.0f);
	style.WindowRounding = 11.5f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 20.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 17.4f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 3.4f);
	style.FrameRounding = 11.9f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.9f, 13.4f);
	style.ItemInnerSpacing = ImVec2(7.1f, 1.8f);
	style.CellPadding = ImVec2(12.1f, 9.2f);
	style.IndentSpacing = 0.0f;
	style.ColumnsMinSpacing = 8.7f;
	style.ScrollbarSize = 11.6f;
	style.ScrollbarRounding = 15.9f;
	style.GrabMinSize = 3.7f;
	style.GrabRounding = 20.0f;
	style.TabRounding = 9.8f;
	style.TabBorderSize = 0.0f;
	//style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.46351933f, 0.46351832f, 0.4635147f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9785408f, 0.9785388f, 0.978531f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.76862746f, 0.76862746f, 0.76862746f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.6995708f, 0.69956875f, 0.69956374f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.0f, 0.9999979f, 0.99999f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.7854077f, 0.785406f, 0.78539985f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.76824033f, 0.76823264f, 0.76823264f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.76862746f, 0.76862746f, 0.76862746f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.7982833f, 0.7982808f, 0.7982753f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.7725322f, 0.7725245f, 0.7725245f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.8025751f, 0.8025724f, 0.80256706f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.80686694f, 0.8068646f, 0.80685884f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.7296137f, 0.72961134f, 0.7296064f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55364805f, 0.55364656f, 0.5536425f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49785405f, 0.49784908f, 0.49784908f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.67811155f, 0.0931312f, 0.27312556f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6351931f, 0.635192f, 0.63518673f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.59656656f, 0.5914458f, 0.5914458f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.78969955f, 0.78969777f, 0.7896916f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.7553648f, 0.7553572f, 0.7553572f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7167382f, 0.710586f, 0.710586f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.7167382f, 0.71673673f, 0.7167311f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.695279f, 0.68931097f, 0.68931097f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.69411767f, 0.6901961f, 0.6901961f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.78969955f, 0.78969705f, 0.7896916f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.84313726f, 0.8156863f, 0.7490196f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.84313726f, 0.8156863f, 0.7490196f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.85490197f, 0.85490197f, 0.85490197f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.3501262f, 0.38693908f, 0.6180258f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.3037816f, 0.33688587f, 0.53218883f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.79399145f, 0.79398894f, 0.7939835f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.76824033f, 0.76823264f, 0.76823264f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.7167382f, 0.7167349f, 0.7167311f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.8745098f, 0.7254902f, 0.42745098f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.47843137f, 0.4f, 0.29803923f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.9607843f, 0.019607844f, 0.11764706f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90588236f, 0.6627451f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.6392157f, 0.39607844f, 0.043137256f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.9529412f, 0.94509804f, 0.92941177f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.9529412f, 0.94509804f, 0.92941177f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.88235295f, 0.8666667f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.9019608f, 0.89411765f, 0.8784314f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0627451f, 0.0627451f, 0.0627451f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.5019608f, 0.4862745f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.73333335f, 0.70980394f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.5019608f, 0.4862745f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.008526837f, 0.008583665f, 0.0075153117f, 0.502f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(9.858461e-7f, 1e-6f, 7.3390555e-7f, 0.502f);
}

void Theme_LightOrange() {
	// Comfortable Light Orange style by SouthCraftX from ImThemes
	ImGuiStyle& style = GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(20.0f, 20.0f);
	style.WindowRounding = 11.5f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 20.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 17.4f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 3.4f);
	style.FrameRounding = 11.9f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.9f, 13.4f);
	style.ItemInnerSpacing = ImVec2(7.1f, 1.8f);
	style.CellPadding = ImVec2(12.1f, 9.2f);
	style.IndentSpacing = 0.0f;
	style.ColumnsMinSpacing = 8.7f;
	style.ScrollbarSize = 11.6f;
	style.ScrollbarRounding = 15.9f;
	style.GrabMinSize = 3.7f;
	style.GrabRounding = 20.0f;
	style.TabRounding = 9.8f;
	style.TabBorderSize = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.7254902f, 0.68235296f, 0.54901963f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.90588236f, 0.8980392f, 0.88235295f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.84313726f, 0.83137256f, 0.80784315f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.8862745f, 0.8745098f, 0.84705883f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.84313726f, 0.83137256f, 0.80784315f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.84313726f, 0.83137256f, 0.80784315f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.9529412f, 0.94509804f, 0.92941177f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.9529412f, 0.94509804f, 0.92941177f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.9019608f, 0.89411765f, 0.8784314f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.9529412f, 0.94509804f, 0.92941177f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.88235295f, 0.8666667f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.84313726f, 0.83137256f, 0.80784315f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.88235295f, 0.8666667f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.96862745f, 0.050980393f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.9647059f, 0.8f, 0.02745098f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.96862745f, 0.5882353f, 0.03529412f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.88235295f, 0.8666667f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.81960785f, 0.8117647f, 0.8039216f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.84705883f, 0.84705883f, 0.84705883f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.85882354f, 0.8352941f, 0.7921569f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.89411765f, 0.89411765f, 0.89411765f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.87058824f, 0.8509804f, 0.80784315f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.84313726f, 0.8156863f, 0.7490196f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.84313726f, 0.8156863f, 0.7490196f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.85490197f, 0.85490197f, 0.85490197f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.96862745f, 0.050980393f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.88235295f, 0.8666667f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.88235295f, 0.8666667f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.92156863f, 0.9137255f, 0.8980392f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.8745098f, 0.7254902f, 0.42745098f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.47843137f, 0.4f, 0.29803923f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.9607843f, 0.019607844f, 0.11764706f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90588236f, 0.6627451f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.6392157f, 0.39607844f, 0.043137256f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.9529412f, 0.94509804f, 0.92941177f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.9529412f, 0.94509804f, 0.92941177f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.88235295f, 0.8666667f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.9019608f, 0.89411765f, 0.8784314f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0627451f, 0.0627451f, 0.0627451f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.5019608f, 0.4862745f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.73333335f, 0.70980394f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.5019608f, 0.4862745f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8039216f, 0.8235294f, 0.45490196f, 0.502f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8039216f, 0.8235294f, 0.45490196f, 0.502f);
}

void Theme_Windark()
{
	// Fork of Windark style from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 8.4f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.0f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 3.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 3.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 3.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 5.6f;
	style.ScrollbarRounding = 18.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 3.0f;
	style.TabBorderSize = 0.0f;
	//style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1254902f, 0.1254902f, 0.1254902f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34901962f, 0.34901962f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.32941177f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 0.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.3019608f, 0.3019608f, 0.3019608f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.2509804f, 0.2509804f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.21568628f, 0.21568628f, 0.21568628f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.32941177f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.32941177f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05882353f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.47058824f, 0.84313726f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.99999f, 0.99999f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.34901962f);
}

void MiksetText(std::string text, const ImVec4& color, const ImVec2& pos) {
	/*
	ImGui::PushStyleColor(ImGuiCol_TitleBg, {1.0f, 1.0f, 1.0f, 0.0f});
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, {1.0f, 1.0f, 1.0f, 0.0f});
	ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, {1.0f, 1.0f, 1.0f, 0.0f});
	ImGui::PushStyleColor(ImGuiCol_Border, {1.0f, 1.0f, 1.0f, 0.0f});
	ImGui::PushStyleColor(ImGuiCol_Text, color);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, {0.0f, 0.0f});
	
	ImGui::SetNextWindowSize({1000, 1000});
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowCollapsed(true);
	
	Begin(text.c_str(), (bool*)false,  ImGuiWindowFlags_NoMouseInputs);
	End();


	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
*/	
//you cant be fkin serious i just discovered i could simply do this
	ImGui::GetForegroundDrawList()->AddText(
		pos,
		ImColor(color),
		text.c_str()
	);
}

void XYZVec3Control(XMFLOAT3& val, int id) { // just parse in any number(except 0) just dont make 2 calls with the same ID
	PushID(id);

	PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.0f, 4.0f});


	PushStyleColor(ImGuiCol_Button, {0.8f, 0.0f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_ButtonHovered, {0.8f, 0.0f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_ButtonActive, {0.8f, 0.0f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_BorderShadow, {0.8f, 0.0f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_Border, {0.8f, 0.0f, 0.0f, 1.0f});
	PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

	ImGui::Button("X"); SameLine();

	PopStyleVar();
	for (int i=0; i < 5; i++) PopStyleColor();

	/*
style.Colors[ImGuiCol_FrameBg] = ImVec4(0.09012878f, 0.087034225f, 0.087034225f, 1.0f);
	style.FrameRounding = 0.0f;
	*/


	PushStyleColor(ImGuiCol_FrameBg, {0.09f, 0.09f, 0.09, 1.0f});
	PushStyleColor(ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f });
	PushStyleColor(ImGuiCol_BorderShadow, { 0.0f, 0.0f, 0.0f, 0.0f });
	PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	

	ImGui::SetNextItemWidth(70.0f);
	ImGui::DragFloat("##", &val.x, 0.1f); SameLine();
	PopStyleColor();
	PopStyleColor();
	PopStyleColor();
	PopStyleVar();


	PushStyleColor(ImGuiCol_Button, {0.0f, 0.6f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_ButtonHovered, {0.0f, 0.6f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_ButtonActive, {0.0f, 0.6f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_BorderShadow, {0.0f, 0.6f, 0.0f, 1.0f});
	PushStyleColor(ImGuiCol_Border, {0.0f, 0.6f, 0.0f, 1.0f});
	PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

	ImGui::Button("Y"); SameLine();

	PopStyleVar();
	for (int i=0; i < 5; i++) PopStyleColor();


	PushStyleColor(ImGuiCol_FrameBg, {0.09f, 0.09f, 0.09, 1.0f});
	PushStyleColor(ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f });
	PushStyleColor(ImGuiCol_BorderShadow, { 0.0f, 0.0f, 0.0f, 0.0f });
	PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

	ImGui::SetNextItemWidth(70.0f);

	ImGui::DragFloat("###x", &val.y, 0.1f); SameLine();

	PopStyleColor();
	PopStyleColor();
	PopStyleColor();
	PopStyleVar();


	PushStyleColor(ImGuiCol_Button, {0.0f, 0.0f, 0.8f, 1.0f});
	PushStyleColor(ImGuiCol_ButtonHovered, {0.0f, 0.0f, 0.8f, 1.0f});
	PushStyleColor(ImGuiCol_ButtonActive, {0.0f, 0.0f, 0.8f, 1.0f});
	PushStyleColor(ImGuiCol_BorderShadow, {0.0f, 0.0f, 0.8f, 1.0f});
	PushStyleColor(ImGuiCol_Border, {0.0f, 0.0f, 0.8f, 1.0f});
	PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

	ImGui::Button("Z"); SameLine();

	PopStyleVar();
	for (int i=0; i < 5; i++) PopStyleColor();

	PushStyleColor(ImGuiCol_FrameBg, {0.09f, 0.09f, 0.09, 1.0f});
	PushStyleColor(ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f });
	PushStyleColor(ImGuiCol_BorderShadow, { 0.0f, 0.0f, 0.0f, 0.0f });
	PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

	ImGui::SetNextItemWidth(70.0f);
	ImGui::DragFloat("####xx", &val.z, 0.1f);

	PopStyleColor();
	PopStyleColor();
	PopStyleColor();
	PopStyleVar();
	
	PopStyleVar();

	PopID();
	
}


ImFont* font20p;
InterFace::InterFace(SDL_Window* window, Core& core) : core(core) {
	IMGUI_CHECKVERSION();
	CreateContext();
	ImGuiIO& io = GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//SetColorStyle();
	Theme_Windark();
	ImGui_ImplSDL2_InitForD3D(window);
	ImGui_ImplDX11_Init(core.GetDevice(), core.m_deviceCtx.Get());	
	font16p = io.Fonts->AddFontFromFileTTF("UI/fonts/JetBrainsMono-Medium.ttf", 16.0f);
	font20p = io.Fonts->AddFontFromFileTTF("UI/fonts/JetBrainsMono-Medium.ttf", 20.0f);

	if (font16p == nullptr || !font16p) {
		MessageBoxA(NULL, "Ensure a vaild font and a valid path.", "Failed to load font..", MB_OK | MB_ICONERROR);
	}
}

InterFace::~InterFace() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	DestroyContext();
}

void InterFace::ProcessEvent(const SDL_Event& event) {
	ImGui_ImplSDL2_ProcessEvent(&event);
}

void InterFace::BeginFrame() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	NewFrame();
}


void InterFace::DrawEditor(InterfaceCtx& ctx) { 
	this->BeginFrame();
	

	if (font16p) PushFont(font16p);
	ImGuiIO& io = GetIO();
	
	PushFont(font20p);
	MiksetText("Fps: " + std::to_string((int)io.Framerate), {1.0f, 0.5f, 0.0f, 1.0f});
	MiksetText("Frame-Time: " + std::to_string(1000.0f / io.Framerate) + "ms", {0.0f, 0.7f, 0.0f, 1.0f}, {0, 20});
	PopFont();

	Begin("debug");
		const std::string camposStr_2f = std::format("Camera Pos: {:.2f} {:.2f} {:.2f}", ctx.camera->pos.x, ctx.camera->pos.y, ctx.camera->pos.z);
		const std::string camposStr_i  = std::format("Camera Pos: {} {} {}", (int)ctx.camera->pos.x, (int)ctx.camera->pos.y, (int)ctx.camera->pos.z);
		if (*ctx.ColliderView) Text(camposStr_2f.c_str());
		else Text(camposStr_i.c_str());

		static bool settings = false;
		if (Button("Settings")) {
			settings = true;
		}

	
		
	End();
	

	// controls tab
	static bool lightAnim  = false;

	static float pos[3] = { 10.0f, 30.0f, 10.0f };
	if (settings) {
		Begin("Settings", &settings); 
		 ImVec2 settings_winSize = GetWindowSize();
		 if (BeginTabBar("##")) {

			// graphical tab
			if (BeginTabItem("Graphical")) {
				//static vsync = this->core.vsync;
				if (RadioButton("VSync", this->core.vsync)) {
					this->core.vsync = !this->core.vsync;
				}
				if (Checkbox("Collider View", ctx.ColliderView)) {
					
				}
				EndTabItem();
			}

			// controlls tab
			if (BeginTabItem("Controlls")) {
				SliderFloat("Sensitivity", ctx.sensitivity, 0.001f, 1.0f);
				SliderFloat("Fly speed", ctx.Flyspeed, 0.1f, 100.0f);
				SliderFloat("PlayerHeight", &ctx.player->Height, 0.01f, 100.0f);
				EndTabItem();
			}


			// objects tab
			if (BeginTabItem("Objects")) {

				// light sources
				if (CollapsingHeader("Directional Light")) {
					Checkbox("Light animation", &lightAnim);
					static XMFLOAT3 rawPos = XMFLOAT3(ctx.scene->gb.LightDir.x, ctx.scene->gb.LightDir.y, ctx.scene->gb.LightDir.z);

					XYZVec3Control(rawPos, 8);
					float length = sqrtf(rawPos.x * rawPos.x + rawPos.y * rawPos.y + rawPos.z * rawPos.z);

					if (length > 0.0001f) { // just so we dont crash like last time...
						ctx.scene->gb.LightDir.x = rawPos.x / length;
						ctx.scene->gb.LightDir.y = rawPos.y / length;
						ctx.scene->gb.LightDir.z = rawPos.z / length;
					}
					else ctx.scene->gb.LightDir = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f);
					

					// seriously DirectX Math is probably the shitiest fucking math library you could ever use, jesus
				}
				

				// camera 
				if (CollapsingHeader("Camera")) {
					static float campos[3] = { ctx.camera->pos.x, ctx.camera->pos.y, ctx.camera->pos.z };
					SliderFloat3("CameraXYZ", campos, -100.0f, 100.0f);
					ctx.camera->pos = XMFLOAT3(campos[0], campos[1], campos[2]);
				}

				// entities 
				if (CollapsingHeader("Entities")) {
					static std::vector<float> scalefactor(ctx.scene->GetEntities().size(), 1.0f); // one per entity
					static std::vector<char> uniformScales(ctx.scene->GetEntities().size(), true);
					Separator();

					for (int i = 0; i < ctx.scene->GetEntities().size(); i++) {
						auto& Entity	  = (ctx.scene->GetEntities())[i]; // sigh.. a lambda..
						std::string label = Entity->Name;
						
						PushID(i); // unique id 
						if (CollapsingHeader(label.c_str())) {
							if (BeginTabBar("##x")) {


								if (BeginTabItem("Transform")) {
									Checkbox("Uniform Scaling", (bool*)&uniformScales[i]);
									//SliderFloat3("Positition", &Entity->transf.position.x, -50.0f, 50.0f);
									Text("Translation");
									SameLine();
									XYZVec3Control(Entity->transf.position, 1);
									
									Text("Rotation   ");
									SameLine();
									XYZVec3Control(Entity->transf.rotation, 3);

									if (uniformScales[i]) {
										
										Text("Scale");
										SameLine();
										if (SliderFloat(" ", &scalefactor[i], 0.001f, 50.0f)) {
											Entity->transf.scale.x = scalefactor[i];
											Entity->transf.scale.y = scalefactor[i];
											Entity->transf.scale.z = scalefactor[i];
										}
										
									}
									else {
										Text("Scale      ");
										SameLine();
										XYZVec3Control(Entity->transf.scale, 2);
										//SliderFloat3("Scale", &Entity->transf.scale.x, 0.001f, 50.0f);
									}

									//SliderFloat3("Rotation", &Entity->transf.rotation.x, -PI, PI);
																		
									EndTabItem();
								}
								

								if (BeginTabItem("Materials")) {
									SliderFloat("Roughness", &Entity->material.Roughness, 0.01f, 1.0f);
									SliderFloat("Metalness", &Entity->material.Metallic, 0.0f, 1.0f);
									SliderFloat("Ambient Influence", &Entity->material.AmbientOcclusion, 0.0f, 1.0f);

									float* colors_ptr = reinterpret_cast<float*>(&Entity->material.BaseColor);
									float* colors[3] = { &colors_ptr[0], &colors_ptr[1], &colors_ptr[2] };
									ColorPicker3("Albedo", *colors, ImGuiColorEditFlags_PickerHueWheel);

									EndTabItem();
								}
							
								if (Entity->GetModel()->HasBones && Entity->GetModel()->HasAnimations()) {

									if (BeginTabItem("Animations")) {
										bool wasAnimating = Entity->GetModel()->IsAnimating;
										static int s = 0;
										Text("Slot");
										InputInt("###", &s);
										Checkbox("Play full slot", &Entity->GetModel()->IsAnimating);

										bool nowAnimating = Entity->GetModel()->IsAnimating;
										if (nowAnimating && !wasAnimating) {
											Entity->GetModel()->PlayAnimation(s, true);
										}
										else if (!nowAnimating && wasAnimating) {
											Entity->GetModel()->StopAnimation();
										}

										static float speed = 1.0f;
										Text("Animation speed");
										SliderFloat("###xxx", &speed, 0, 10);
										Entity->GetModel()->SetAnimationSpeed(speed);

										ImGui::Text("Play Animation with Range");
										Separator();

										static int slot = 0;
										static float range[2] = { 0.0f, 1.0f };
										static bool Loop = true;
										Text("Slot");
										InputInt("###xx", &slot);
										InputFloat2("Range", range);
										Checkbox("Loop", &Loop);

										if (Button("Play"))
											Entity->GetModel()->PlayAnimationTimeStamp(slot, range[0], range[1], Loop);

										EndTabItem();
									}
									
								}
							}
							ImGui::EndTabBar();
							Separator();
							Text(" ");

						}
						Text(" ");
						PopID(); 
					} 
					
					Separator();

					// view model
					if (CollapsingHeader(ctx.viewModel->get()->Name.c_str())) {
						if (BeginTabBar("###xxxx")) {
							if (BeginTabItem("Transform")) {
								SliderFloat3("Position", &ctx.viewModel->get()->transf.position.x, -4.0f, 4.0f);

								static float scaleAmmount = 0.5f;
								if (SliderFloat("Scale", &scaleAmmount, 0.1f, 10.0f)) {
									ctx.viewModel->get()->transf.scale.x = scaleAmmount;
									ctx.viewModel->get()->transf.scale.y = scaleAmmount;
									ctx.viewModel->get()->transf.scale.z = scaleAmmount;
								}


								static float* rotation[3] = { &ctx.viewModel->get()->transf.rotation.x, &ctx.viewModel->get()->transf.rotation.y, &ctx.viewModel->get()->transf.rotation.z };
								SliderFloat3("Rotation", *rotation, -4.0f, 4.0f);
								EndTabItem();
							}

							if (BeginTabItem("Materials")) {
								SliderFloat("Roughness", &ctx.viewModel->get()->material.Roughness, 0.01f, 1.0f);
								SliderFloat("Metalness", &ctx.viewModel->get()->material.Metallic, 0.0f, 1.0f);
								SliderFloat("Ambient Occlusion", &ctx.viewModel->get()->material.AmbientOcclusion, 0.0f, 1.0f);
								float* colors_ptr = reinterpret_cast<float*>(&ctx.viewModel->get()->material.BaseColor);
								float colors[3] = { colors_ptr[0], colors_ptr[1], colors_ptr[2] };
								ColorPicker3("Albedo", colors, ImGuiColorEditFlags_PickerHueWheel);

								EndTabItem();
							}

							if (ctx.viewModel->get()->GetModel()->HasBones && ctx.viewModel->get()->GetModel()->HasAnimations()) {

								if (BeginTabItem("Animations")) {
									bool wasAnimating = ctx.viewModel->get()->GetModel()->IsAnimating;
									static int s = 0;
									Text("Slot");
									InputInt("###", &s);
									Checkbox("Play full slot", &ctx.viewModel->get()->GetModel()->IsAnimating);

									bool nowAnimating = ctx.viewModel->get()->GetModel()->IsAnimating;
									if (nowAnimating && !wasAnimating) {
										ctx.viewModel->get()->GetModel()->PlayAnimation(s, true);
									}
									else if (!nowAnimating && wasAnimating) {
										ctx.viewModel->get()->GetModel()->StopAnimation();
									}

									static float speed = 1.0f;
									Text("Animation speed");
									SliderFloat("###xxx", &speed, 0, 10);
									ctx.viewModel->get()->GetModel()->SetAnimationSpeed(speed);

									ImGui::Text("Play Animation with Range");
									Separator();

									static int slot = 0;
									static float range[2] = { 0.0f, 1.0f };
									static bool Loop = true;
									Text("Slot");
									InputInt("###xx", &slot);
									InputFloat2("Range", range);
									Checkbox("Loop", &Loop);

									if (Button("Play"))
										ctx.viewModel->get()->GetModel()->PlayAnimationTimeStamp(slot, range[0], range[1], Loop);

									EndTabItem();
								}
							}
						}
						EndTabBar();
					}
				}
				
				SetCursorPosY(settings_winSize.y - 20.0f); //  FIX: wonky position when theres alot of entities
	
				if (SmallButton("Add Entity")) OpenPopup("Insert Entity");
				SameLine();
				if (SmallButton("Remove Entity")) OpenPopup("Remove Entity");

				if (BeginPopup("Insert Entity")) {

					static char name[64] = "UnNamed";
					Text("Entity Name: "); SameLine();
					InputText("##", name, IM_ARRAYSIZE(name));
					
				
					static bool collidable = false;
					Checkbox("Collidable", &collidable); SameLine();

					static bool bake = false;
					Checkbox("Bake Model Transformation", &bake);

					static bool isAABB;
					if (collidable) {
						static int selected = 0;
						const char* items[] = { "Box", "Triangle Mesh" };
						if (Combo("Collision Type", &selected, items, IM_ARRAYSIZE(items))) {
					}

						std::string type = items[selected];
						if (type == "Triangle Mesh") isAABB = false;
						else if (type == "Box")		 isAABB = true;
					}
					
					Text("Path: "); SameLine();
					if (Button("Browse Files")) {
						IGFD::FileDialogConfig config;
						config.path = ".";
						ImGuiFileDialog::Instance()->OpenDialog("ModelSelect", "Choose File", ".glb,.obj,.gltf", config);
					}
					
					if (ImGuiFileDialog::Instance()->Display("ModelSelect")) {
						if (ImGuiFileDialog::Instance()->IsOk()) {
							std::string path = ImGuiFileDialog::Instance()->GetFilePathName();

							Entity* entity = nullptr;
							Collider coll;
							if (collidable) coll.Type = isAABB ? CollisionType::Box : CollisionType::TriangleMesh;
							entity = ctx.scene->AddEntity(core, name, path, collidable, bake);
							if (entity && collidable) {
								entity->colliders.push_back(coll);
							}

						}
					}
							//ImGuiFileDialog::Instance()->Close();

					EndPopup();
				}

				if (BeginPopup("Remove Entity")) {
					static char name[64] = "UnNamed";
					InputText("Name", name, IM_ARRAYSIZE(name));
					
					if (Button("Remove")) {
						ctx.scene->RemoveEntityByName(name);
					}
					
					EndPopup();
				}
					
				

				EndTabItem();
			}
			
			EndTabBar();
		}
		
		End();
	}

	
	if (lightAnim) {
		static float offset = 0.0f;
		offset += 2.05f * ctx.dt;

		if (offset >= 2 * PI) offset -= 2 * PI;
		float radius = 40.0f;
		ctx.scene->gb.LightDir = XMFLOAT4(std::sin(offset) * radius, pos[1], std::cos(offset) * radius, 1.0f);
	}


	if (font16p) PopFont();

	this->EndFrame();
}

void InterFace::EndFrame() {
	Render();
	ImGui_ImplDX11_RenderDrawData(GetDrawData());
}

