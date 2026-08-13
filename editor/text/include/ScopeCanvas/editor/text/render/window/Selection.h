#pragma once

#include <stdlib.h>

namespace ScopeCanvas::Editor::Text::Render::Window {
struct Selection {
    std::size_t startColumn{};
    std::size_t endColumn{};
    bool operator==(const Selection&) const = default;
};

struct Caret {
    std::size_t column{};
    bool operator==(const Caret&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Render::Window
