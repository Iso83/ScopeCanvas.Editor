#include <ScopeCanvas/editor/text/syntax/profiles/Cpp.h>

namespace ScopeCanvas::Editor::Text::Syntax::Profiles {

CppProfile::CppProfile()
    : m_keywords{"alignas",   "alignof",      "asm",       "break",         "case",      "catch",    "class",
                 "concept",   "const",        "consteval", "constexpr",     "constinit", "continue", "co_await",
                 "co_return", "co_yield",     "decltype",  "default",       "delete",    "do",       "else",
                 "enum",      "explicit",     "export",    "extern",        "false",     "for",      "friend",
                 "goto",      "if",           "inline",    "mutable",       "namespace", "new",      "noexcept",
                 "nullptr",   "operator",     "override",  "private",       "protected", "public",   "requires",
                 "return",    "sizeof",       "static",    "static_assert", "struct",    "switch",   "template",
                 "this",      "thread_local", "throw",     "true",          "try",       "typedef",  "typeid",
                 "typename",  "union",        "using",     "virtual",       "volatile",  "while"},
      m_typeKeywords{"auto", "bool",  "char",   "char8_t",  "char16_t", "char32_t", "double", "float", "int",
                     "long", "short", "signed", "unsigned", "void",     "wchar_t",  "size_t", "std"} {}

std::vector<Token> CppProfile::tokenizeLine(std::string_view text, std::size_t line, LexerState& state) const {
    std::vector<Token> result;
    auto add = [&](std::size_t start, std::size_t end, TokenKind kind, std::string detail = {}) {
        result.push_back({line, start, end, kind, std::move(detail)});
    };

    std::size_t first = 0;
    while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first])) != 0)
        ++first;

    if ((state.mode == "preprocessor" && state.continued) || (first < text.size() && text[first] == '#')) {
        add(0, text.size(), TokenKind::PreprocessorDirective, "preprocessor");
        state.continued = !text.empty() && text.back() == '\\';
        state.mode = state.continued ? "preprocessor" : std::string{};
        return result;
    }

    state.continued = false;
    std::size_t at = 0;
    while (at < text.size()) {
        if (state.mode == "string" || state.mode == "character") {
            const char quote = state.mode == "string" ? '"' : '\'';
            const std::size_t start = at;
            bool escaped = false;
            bool closed = false;

            while (at < text.size()) {
                const char current = text[at++];
                if (!escaped && current == quote) {
                    closed = true;
                    break;
                }

                escaped = !escaped && current == '\\';
                if (current != '\\')
                    escaped = false;
            }

            add(start, at, quote == '"' ? TokenKind::StringLiteral : TokenKind::CharacterLiteral, state.mode);
            if (closed || text.empty() || text.back() != '\\')
                state.mode.clear();

            continue;
        }

        if (state.mode == "block-comment") {
            const auto end = text.find("*/", at);
            if (end == std::string_view::npos) {
                add(at, text.size(), TokenKind::Comment, "block-comment");
                return result;
            }

            add(at, end + 2U, TokenKind::Comment, "block-comment");
            at = end + 2U;
            state.mode.clear();

            continue;
        }
        if (at + 1U < text.size() && text.substr(at, 2) == "//") {
            add(at, text.size(), TokenKind::Comment, "line-comment");
            break;
        }

        if (at + 1U < text.size() && text.substr(at, 2) == "/*") {
            const auto end = text.find("*/", at + 2U);
            if (end == std::string_view::npos) {
                add(at, text.size(), TokenKind::Comment, "block-comment");
                state.mode = "block-comment";
                break;
            }

            add(at, end + 2U, TokenKind::Comment, "block-comment");
            at = end + 2U;

            continue;
        }

        const char value = text[at];
        if (value == '"' || value == '\'') {
            const char quote = value;
            const std::size_t start = at++;
            bool escaped = false;
            bool closed = false;
            while (at < text.size()) {
                const char current = text[at++];
                if (!escaped && current == quote) {
                    closed = true;
                    break;
                }

                escaped = !escaped && current == '\\';
                if (current != '\\')
                    escaped = false;
            }

            add(start, at, quote == '"' ? TokenKind::StringLiteral : TokenKind::CharacterLiteral,
                quote == '"' ? "string" : "character");
            if (!closed && !text.empty() && text.back() == '\\')
                state.mode = quote == '"' ? "string" : "character";

            continue;
        }

        if (identifierStart(value)) {
            const std::size_t start = at++;
            while (at < text.size() && identifierContinue(text[at]))
                ++at;

            const auto word = text.substr(start, at - start);
            if (m_typeKeywords.contains(word))
                add(start, at, TokenKind::TypeKeyword, std::string(word));
            else if (m_keywords.contains(word))
                add(start, at, TokenKind::Keyword, std::string(word));
            else
                add(start, at, TokenKind::Identifier, "identifier");

            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(value)) != 0) {
            const std::size_t start = at++;
            while (at < text.size() &&
                   (std::isalnum(static_cast<unsigned char>(text[at])) != 0 || text[at] == '.' || text[at] == '\''))
                ++at;
            add(start, at, TokenKind::NumericLiteral, "number");

            continue;
        }

        if (operatorCharacter(value)) {
            const std::size_t start = at++;
            while (at < text.size() && operatorCharacter(text[at]))
                ++at;
            add(start, at, TokenKind::Operator, "operator");

            continue;
        }

        if (punctuation(value)) {
            add(at, at + 1U, TokenKind::Punctuation, "punctuation");
            ++at;

            continue;
        }

        const std::size_t start = at++;
        while (at < text.size() && std::isspace(static_cast<unsigned char>(text[at])) != 0)
            ++at;
        add(start, at, TokenKind::PlainText);
    }

    return result;
}
} // namespace ScopeCanvas::Editor::Text::Syntax::Profiles
