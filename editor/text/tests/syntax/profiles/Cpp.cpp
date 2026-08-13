#include "TestAssert.h"

#include <ScopeCanvas/editor/text/syntax/Highlighter.h>
#include <ScopeCanvas/editor/text/syntax/profiles/Cpp.h>
#include <algorithm>

using namespace ScopeCanvas::Editor::Text::Syntax;
using namespace ScopeCanvas::Editor::Text::Syntax::Profiles;

int test_recognizes_lexical_categories_and_ranges() {
    const ScopeCanvas::Editor::Text::Document document(
        "if different int value = 0x2A + 3;\n"
        "const char* text = \"if \\\"quoted\\\"\"; char c = '\\n'; // return\n"
        "/* if block\ncomment */ return value;\n"
        "#include <vector>\n");
    const Document syntax = Highlighter::highlight(document, CppProfile{});
    CPPTEST_ASSERT(syntax.lines.size() == document.lineCount());
    const auto has = [&](std::size_t line, std::size_t start, std::size_t end, TokenKind kind) {
        return std::ranges::any_of(syntax.lines[line].tokens, [&](const Token& token) {
            return token.startColumn == start && token.endColumn == end && token.kind == kind;
        });
    };
    CPPTEST_ASSERT(has(0, 0, 2, TokenKind::Keyword));
    CPPTEST_ASSERT(has(0, 3, 12, TokenKind::Identifier));
    CPPTEST_ASSERT(has(0, 13, 16, TokenKind::TypeKeyword));
    CPPTEST_ASSERT(has(0, 25, 29, TokenKind::NumericLiteral));
    CPPTEST_ASSERT(has(0, 30, 31, TokenKind::Operator));
    CPPTEST_ASSERT(has(0, 33, 34, TokenKind::Punctuation));
    CPPTEST_ASSERT(has(1, 19, 34, TokenKind::StringLiteral));
    CPPTEST_ASSERT(has(1, 45, 49, TokenKind::CharacterLiteral));
    CPPTEST_ASSERT(std::ranges::any_of(syntax.lines[1].tokens,
                                       [](const Token& token) { return token.kind == TokenKind::Comment; }));
    CPPTEST_ASSERT(syntax.lines[2].tokens.front().kind == TokenKind::Comment);
    CPPTEST_ASSERT(syntax.lines[3].tokens.front().kind == TokenKind::Comment);
    CPPTEST_ASSERT(std::ranges::any_of(syntax.lines[3].tokens,
                                       [](const Token& token) { return token.kind == TokenKind::Keyword; }));
    CPPTEST_ASSERT(syntax.lines[4].tokens.front().kind == TokenKind::PreprocessorDirective);
    CPPTEST_ASSERT(syntax.lines.back().tokens.empty());
    return 0;
}

int main() {
    CPPTEST_RUN(test_recognizes_lexical_categories_and_ranges);
    return 0;
}
