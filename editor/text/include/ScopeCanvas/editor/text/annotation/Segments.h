#pragma once

#include <ScopeCanvas/editor/text/annotation/Color.h>
#include <ScopeCanvas/editor/text/annotation/FontStyle.h>
#include <stdlib.h>
#include <string>

namespace ScopeCanvas::Editor::Text::Annotation {
struct LineSegment {
    std::size_t line{}, startColumn{}, endColumn{};
    auto operator<=>(const LineSegment&) const = default;
};

struct StyledLineSegment : LineSegment {
    Color color;
    FontStyle style{FontStyle::Regular};
    std::string classification, id;
    int precedence{};
    auto operator<=>(const StyledLineSegment&) const = default;
};

struct VisualLineSegment {
    std::size_t row{}, startColumn{}, endColumn{};
    auto operator<=>(const VisualLineSegment&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Annotation
