
#ifndef LABRENDERTYPES_DEPTHTEST
#define LABRENDERTYPES_DEPTHTEST

#include <string>

namespace lab { namespace Render {
    
    enum class DepthTest : int
    {
        less = 0, lequal, never, equal, greater, notequal, gequal, always
    };

}}

#endif
