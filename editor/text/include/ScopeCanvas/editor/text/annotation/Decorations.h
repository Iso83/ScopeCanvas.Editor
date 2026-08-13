#pragma once

#include <ScopeCanvas/editor/text/Range.h>
#include <ScopeCanvas/editor/text/annotation/Color.h>
#include <string>

namespace ScopeCanvas::Editor::Text::Annotation {
enum class UnderlineStyle { Straight };

struct TextDecoration {
    Range range;
    UnderlineStyle style{UnderlineStyle::Straight};
    Color color;
    std::string id;
    std::string classification;
    int precedence{};
};
} // namespace ScopeCanvas::Editor::Text::Annotation
