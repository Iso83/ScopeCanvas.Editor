#pragma once

#include <cstdint>
#include <stdlib.h>
#include <string>
#include <vector>

namespace ScopeCanvas::Editor::Text::Syntax {
enum class TokenKind : std::uint8_t {
    PlainText,
    Keyword,
    TypeKeyword,
    PreprocessorDirective,
    Identifier,
    NumericLiteral,
    StringLiteral,
    CharacterLiteral,
    Comment,
    Operator,
    Punctuation
};

struct Token {
    std::size_t line{};
    std::size_t startColumn{};
    std::size_t endColumn{};
    TokenKind kind{TokenKind::PlainText};
    std::string detail;
    auto operator<=>(const Token&) const = default;
};

struct Line {
    std::size_t line{};
    std::vector<Token> tokens;
};

struct LexerState {
    std::string mode;
    bool continued{};
    bool operator==(const LexerState&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Syntax
