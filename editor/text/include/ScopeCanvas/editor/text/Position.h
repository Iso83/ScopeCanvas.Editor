#pragma once

#include <optional>
#include <stdlib.h>

namespace ScopeCanvas::Editor::Text {
struct Position {
    std::size_t line{};
    std::size_t column{};
    auto operator<=>(const Position&) const = default;
};
} // namespace ScopeCanvas::Editor::Text
