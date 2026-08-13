#pragma once

#include <ScopeCanvas/editor/text/syntax/Tokens.h>
#include <string_view>

namespace ScopeCanvas::Editor::Text::Syntax {
class Profile {
public:
    virtual ~Profile() = default;
    [[nodiscard]] virtual std::vector<Token> tokenizeLine(std::string_view text, std::size_t line,
                                                          LexerState& state) const = 0;

protected:
    static bool identifierStart(char value) {
        return std::isalpha(static_cast<unsigned char>(value)) != 0 || value == '_';
    }
    static bool identifierContinue(char value) {
        return std::isalnum(static_cast<unsigned char>(value)) != 0 || value == '_';
    }
    static bool punctuation(char value) {
        return std::string_view("(){}[];,.").find(value) != std::string_view::npos;
    }
    static bool operatorCharacter(char value) {
        return std::string_view("+-*/%=!<>&|^~?:").find(value) != std::string_view::npos;
    }
};
} // namespace ScopeCanvas::Editor::Text::Syntax
