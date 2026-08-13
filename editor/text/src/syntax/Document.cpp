#include <ScopeCanvas/editor/text/syntax/Document.h>

using namespace ScopeCanvas::Editor::Text::Annotation;

namespace ScopeCanvas::Editor::Text::Syntax {
std::vector<ForegroundSpan> Document::styleSyntax(const Text::Document& document, const ColorProfile& colors,
                                                  const std::vector<StyleOverride>& overrides,
                                                  Color defaultColor) const {
    std::vector<ForegroundSpan> spans;
    for (const auto& line : lines) {
        if (line.line >= document.lineCount())
            continue;

        for (const auto& token : line.tokens) {
            if (token.startColumn >= token.endColumn || token.endColumn > document.line(line.line).size() ||
                token.kind == TokenKind::PlainText)
                continue;

            spans.push_back({{{token.line, token.startColumn}, {token.line, token.endColumn}},
                             colors.color(token.kind),
                             FontStyle::Regular,
                             token.detail,
                             "syntax",
                             0});
        }
    }
    for (std::size_t index = 0; index < overrides.size(); ++index) {
        const auto& override = overrides[index];
        if (!document.valid(override.range) || override.range.empty())
            continue;

        spans.push_back({override.range,
                         override.suppressSyntaxColor ? defaultColor : override.color.value_or(defaultColor),
                         FontStyle::Regular, override.classification, "syntax-override-" + std::to_string(index),
                         100 + override.precedence});
    }

    return spans;
}
} // namespace ScopeCanvas::Editor::Text::Syntax
