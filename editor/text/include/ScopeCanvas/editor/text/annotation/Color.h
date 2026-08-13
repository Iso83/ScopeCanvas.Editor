#pragma once

#include <cstdint>
#include <optional>

namespace ScopeCanvas::Editor::Text::Annotation {
struct Color {
    std::uint8_t red{}, green{}, blue{}, alpha{255};
    auto operator<=>(const Color&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Annotation
