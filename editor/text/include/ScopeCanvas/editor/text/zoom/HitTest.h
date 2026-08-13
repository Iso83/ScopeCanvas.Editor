#pragma once

#include <stdlib.h>

namespace ScopeCanvas::Editor::Text::Zoom {
struct HitTestResult {
    std::size_t visualRow{};
    std::size_t column{};
    bool inCodeArea{};
};
} // namespace ScopeCanvas::Editor::Text::Zoom
