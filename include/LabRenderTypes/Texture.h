
#ifndef LABRENDERTYPES_TEXTURE
#define LABRENDERTYPES_TEXTURE

#include <string>

namespace lab { namespace Render {
    
    enum class TextureType {
        none,
        f32x1, f32x2, f32x3, f32x4,
        f16x1, f16x2, f16x3, f16x4,
        u8x1,  u8x2,  u8x3,  u8x4,
        s8x1,  s8x2,  s8x3,  s8x4
    };

    struct texture
    {
        std::string name;
        TextureType format { TextureType::none };
        float scale {1.f};
    };

}}

#endif
