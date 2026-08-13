#pragma once

#include <ScopeCanvas/editor/text/annotation/Color.h>
#include <ScopeCanvas/editor/text/syntax/Tokens.h>

namespace ScopeCanvas::Editor::Text::Syntax {
class ColorProfile {
private:
    std::vector<Annotation::Color> m_colors =
        std::vector<Annotation::Color>(11U, Annotation::Color{220, 220, 220, 255});

public:
    [[nodiscard]] Annotation::Color color(TokenKind kind) const noexcept {
        const auto index = static_cast<std::size_t>(kind);
        return index < m_colors.size() ? m_colors[index] : Annotation::Color{220, 220, 220, 255};
    }
    void set(TokenKind kind, Annotation::Color color) {
        m_colors[static_cast<std::size_t>(kind)] = color;
    }

    [[nodiscard]] static ColorProfile visualStudioDark();
};
} // namespace ScopeCanvas::Editor::Text::Syntax
