
#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <array>
#include <filesystem>

namespace lab {

namespace fs = std::filesystem;

enum class FontName : int
{
    Default = 0, Regular, Mono, MonoSmall, Icon, COUNT
};

class FontManager
{
    fs::path resource_path;
    std::array<ImFont*, (int) FontName::COUNT> fonts;

    FontManager() = delete;
public:
    FontManager(const fs::path& resource_path);
    ImFont* GetFont(FontName);
};

class SetFont
{
public:
	SetFont(ImFont* f) { ImGui::PushFont(f); }
	~SetFont() { ImGui::PopFont(); }
};

}
