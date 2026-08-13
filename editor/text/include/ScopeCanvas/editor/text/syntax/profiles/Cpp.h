#pragma once

#include <ScopeCanvas/editor/text/syntax/Profile.h>
#include <functional>
#include <set>
#include <string>

namespace ScopeCanvas::Editor::Text::Syntax::Profiles {
class CppProfile final : public Profile {
private:
    std::set<std::string, std::less<>> m_keywords;
    std::set<std::string, std::less<>> m_typeKeywords;

public:
    CppProfile();
    [[nodiscard]] std::vector<Token> tokenizeLine(std::string_view text, std::size_t line,
                                                  LexerState& state) const override;
};
} // namespace ScopeCanvas::Editor::Text::Syntax::Profiles
