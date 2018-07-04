
#include "FontManager.h"
#include "imgui_impl_opengl3.h"
#include "roboto_regular.ttf.h"
#include "robotomono_regular.ttf.h"
#include "meshula-icons.h"
#include <stdint.h>

namespace lab {

FontManager::FontManager(const fs::path& resource_dir)
: resource_path(resource_dir)
{
    std::fill(fonts.begin(), fonts.end(), nullptr);
}

ImFont* FontManager::GetFont(FontName font_name)
{
    int index = (int) font_name;
    if (index >= fonts.size())
        index = (int) FontName::Default;

    if (fonts[index])
        return fonts[index];

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.MergeMode = false;

    auto io = ImGui::GetIO();
    fonts[(int) FontName::Default] = io.Fonts->AddFontDefault(&config);
    fonts[(int) FontName::Regular] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoRegularTtf, sizeof(s_robotoRegularTtf), 28, &config);
    fonts[(int) FontName::Mono] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), 28.0f, &config);
    fonts[(int) FontName::MonoSmall] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), 20.f, &config);

    ImFontConfig iconFontCfg;
    iconFontCfg.PixelSnapH = true;

    //static const ImWchar iconFontRanges[] = { (ImWchar) MI_ICON_MIN, (ImWchar) MI_ICON_MAX, 0 };
    //iconFontCfg.MergeMode = true;
    //fonts[(int) FontName::Icon] = io.Fonts->AddFontFromFileTTF(
    //    (resource_path / "fonts" / "meshula-icons" / "meshula-icons.ttf").string().c_str(), 32, &iconFontCfg, iconFontRanges);

    fonts[(int) FontName::Icon] = io.Fonts->AddFontFromFileTTF(
        (resource_path / "fonts" / "meshula-icons" / "meshula-icons.ttf").string().c_str(), 32, &iconFontCfg);

    uint8_t* data = nullptr;
    int32_t width = 0;
    int32_t height = 0;
    io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

    /*
    s_font_texture = std::make_shared<Texture>(
        static_cast<std::uint16_t>(width)
        , static_cast<std::uint16_t>(height)
        , false
        , 1
        , gfx::TextureFormat::BGRA8
        , 0
        , gfx::copy(data, width*height * 4)
        );

    io.Fonts->SetTexID(s_font_texture.get());*/

    ImGui_ImplOpenGL3_CreateFontsTexture();
    return fonts[index];
}


} // lab
