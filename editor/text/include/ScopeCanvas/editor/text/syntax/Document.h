#pragma once

#include <ScopeCanvas/editor/text/Document.h>
#include <ScopeCanvas/editor/text/annotation/Spans.h>
#include <ScopeCanvas/editor/text/syntax/ColorProfile.h>
#include <ScopeCanvas/editor/text/syntax/StyleOverride.h>

namespace ScopeCanvas::Editor::Text::Syntax {
struct Document {
    std::vector<Line> lines;
    std::vector<std::string> sourceLines;
    std::vector<LexerState> endStates;

    [[nodiscard]] std::vector<Annotation::ForegroundSpan>
    styleSyntax(const Text::Document& document, const ColorProfile& colors,
                const std::vector<StyleOverride>& overrides = {},
                Annotation::Color defaultColor = {220, 224, 235, 255}) const;
};
} // namespace ScopeCanvas::Editor::Text::Syntax
