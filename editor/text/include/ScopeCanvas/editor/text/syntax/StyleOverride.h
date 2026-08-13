#pragma once

#include <ScopeCanvas/editor/text/Range.h>
#include <ScopeCanvas/editor/text/annotation/Color.h>
#include <string>

namespace ScopeCanvas::Editor::Text::Syntax {
struct StyleOverride {
    Range range;
    std::optional<Annotation::Color> color;
    bool suppressSyntaxColor{};
    std::string classification;
    int precedence{};
};
} // namespace ScopeCanvas::Editor::Text::Syntax
