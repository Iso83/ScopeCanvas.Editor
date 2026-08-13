#include "TestAssert.h"

#include <ScopeCanvas/editor/text/annotation/Model.h>
#include <ScopeCanvas/editor/text/syntax/Highlighter.h>

using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text::Syntax;

int test_update_retokenizes_only_changed_lines_and_state_dependents() {
    class CountingProfile final : public Profile {
    public:
        mutable std::size_t calls{};

        std::vector<Token> tokenizeLine(std::string_view text, std::size_t line, LexerState& state) const override {
            ++calls;
            if (text == "begin")
                state.mode = "continued";
            else if (text == "end")
                state.mode.clear();
            return {{line, 0, text.size(), state.mode.empty() ? TokenKind::Identifier : TokenKind::Comment}};
        }
    } profile;

    ScopeCanvas::Editor::Text::Document document("zero\none\ntwo\nthree\nfour");
    Document syntax = Highlighter::highlight(document, profile);
    CPPTEST_ASSERT(profile.calls == 5U);

    document.replace({{2, 0}, {2, 3}}, "changed");
    Highlighter::update(document, profile, syntax);
    CPPTEST_ASSERT(profile.calls == 6U);
    CPPTEST_ASSERT(syntax.lines[2].tokens.front().endColumn == 7U);

    document.replace({{1, 0}, {1, 3}}, "begin");
    Highlighter::update(document, profile, syntax);
    CPPTEST_ASSERT(profile.calls == 10U);
    CPPTEST_ASSERT(syntax.lines[1].tokens.front().kind == TokenKind::Comment);
    CPPTEST_ASSERT(syntax.lines[4].tokens.front().kind == TokenKind::Comment);
    return 0;
}

int main() {
    CPPTEST_RUN(test_update_retokenizes_only_changed_lines_and_state_dependents);
    return 0;
}
