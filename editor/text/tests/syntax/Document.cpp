#include "TestAssert.h"

#include <ScopeCanvas/editor/text/diff/Layout.h>
#include <ScopeCanvas/editor/text/render/window/Frame.h>
#include <ScopeCanvas/editor/text/render/window/Model.h>
#include <ScopeCanvas/editor/text/syntax/Highlighter.h>
#include <ScopeCanvas/editor/text/syntax/profiles/Cpp.h>

using namespace ScopeCanvas::Editor::Text;
using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text::Diff;
using namespace ScopeCanvas::Editor::Text::Syntax;
using namespace ScopeCanvas::Editor::Text::Syntax::Profiles;
using namespace ScopeCanvas::Editor::Text::Render::Window;

int test_colors_overrides_retokenization_and_diff_independence() {
    CppProfile profile;
    ScopeCanvas::Editor::Text::Document document("if value\nreturn value;");
    const Syntax::Document syntax = Highlighter::highlight(document, profile);
    Syntax::ColorProfile first = Syntax::ColorProfile::visualStudioDark();
    Syntax::ColorProfile second = first;
    first.set(TokenKind::Keyword, {1, 2, 3, 255});
    second.set(TokenKind::Keyword, {9, 8, 7, 255});
    const auto firstSpans = syntax.styleSyntax(document, first);
    const auto secondSpans = syntax.styleSyntax(document, second);
    CPPTEST_ASSERT(firstSpans.front().range == secondSpans.front().range);
    CPPTEST_ASSERT(firstSpans.front().color == (Color{1, 2, 3, 255}));
    CPPTEST_ASSERT(secondSpans.front().color == (Color{9, 8, 7, 255}));

    const Range keywordRange{{0, 0}, {0, 2}};
    const auto overridden =
        syntax.styleSyntax(document, first, {{keywordRange, Color{30, 40, 50, 255}, false, "external", 5}});
    Annotation::Model model;
    model.setForegroundSpans(document, overridden);
    const auto resolved = model.resolvedForeground(document);
    CPPTEST_ASSERT(resolved.front().color == (Color{30, 40, 50, 255}));
    const auto suppressed =
        syntax.styleSyntax(document, first, {{keywordRange, std::nullopt, true, "plain", 0}}, {200, 201, 202, 255});
    model.setForegroundSpans(document, suppressed);
    CPPTEST_ASSERT(model.resolvedForeground(document).front().color == (Color{200, 201, 202, 255}));

    document.replace({{0, 0}, {0, 2}}, "while");
    const Syntax::Document updated = Highlighter::highlight(document, profile);
    CPPTEST_ASSERT(updated.lines[0].tokens.front().endColumn == 5U);
    CPPTEST_ASSERT(updated.lines[0].tokens.front().kind == TokenKind::Keyword);

    const ScopeCanvas::Editor::Text::Document right("return value;");
    CPPTEST_ASSERT(updated.lines.size() == document.lineCount());
    CPPTEST_ASSERT(updated.lines[0].line == 0U);
    return 0;
}

int test_prepared_segments_can_be_reused_during_frame_build() {
    std::string text;
    for (std::size_t line = 0; line < 600U; ++line)
        text += "if (value" + std::to_string(line) + " >= 42) return value" + std::to_string(line) + ";\n";
    const ScopeCanvas::Editor::Text::Document document(text);
    const Syntax::Document syntax = Highlighter::highlight(document, CppProfile{});
    const std::vector<ForegroundSpan> cached = syntax.styleSyntax(document, Syntax::ColorProfile::visualStudioDark());
    Annotation::Model annotations;
    annotations.setForegroundSpans(document, cached);
    const std::vector<StyledLineSegment> prepared = annotations.resolvedForeground(document);

    const Frame frame =
        Render::Window::Model{}.buildFrame(document, Layout::calculate(document, document).leftProjection(),
                                           annotations, nullptr, {}, {0.0F, 300.0F}, 120.0F, false, true, &prepared);
    CPPTEST_ASSERT(frame.firstVisibleRow < frame.endVisibleRow);
    CPPTEST_ASSERT(std::ranges::any_of(frame.rows, [](const ScopeCanvas::Editor::Text::Render::Window::Row& row) {
        return row.logicalLine.has_value() && !row.foregroundSpans.empty();
    }));
    CPPTEST_ASSERT(cached.size() > document.lineCount());
    return 0;
}

int main() {
    CPPTEST_RUN(test_colors_overrides_retokenization_and_diff_independence);
    CPPTEST_RUN(test_prepared_segments_can_be_reused_during_frame_build);
    return 0;
}
