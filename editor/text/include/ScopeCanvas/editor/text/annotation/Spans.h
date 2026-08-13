#pragma once

#include <ScopeCanvas/editor/text/Range.h>
#include <ScopeCanvas/editor/text/annotation/Color.h>
#include <ScopeCanvas/editor/text/annotation/FontStyle.h>
#include <string>

namespace ScopeCanvas::Editor::Text::Annotation {
struct ForegroundSpan {
    Range range;
    Color color;
    FontStyle style{FontStyle::Regular};
    std::string classification;
    std::string id;
    int precedence{};
};

struct BackgroundSpan {
    Range range;
    Color color;
    std::string id;
    int precedence{};
};
} // namespace ScopeCanvas::Editor::Text::Annotation
