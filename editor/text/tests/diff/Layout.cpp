#include "TestAssert.h"

#include <ScopeCanvas/editor/text/diff/Layout.h>
#include <ScopeCanvas/editor/text/session/EditorSession.h>

using namespace ScopeCanvas::Editor::Text;
using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text::Diff;
using namespace ScopeCanvas::Editor::Text::Session;

int test_aligns_gap_rows_without_changing_documents() {
    const Document left("new\nsame\nchanged\nend");
    const Document right("same\nold\nend\ngone");
    const auto layout = Layout::calculate(left, right);
    CPPTEST_ASSERT(left.text() == "new\nsame\nchanged\nend");
    CPPTEST_ASSERT(right.text() == "same\nold\nend\ngone");
    CPPTEST_ASSERT(layout.rows().size() == 5U);
    CPPTEST_ASSERT(layout.rows()[0].left == 0U);
    CPPTEST_ASSERT(!layout.rows()[0].right.has_value());
    CPPTEST_ASSERT(layout.rows()[2].kind == Kind::Modified);
    CPPTEST_ASSERT(!layout.rows()[4].left.has_value());
    return 0;
}

int test_classifies_insertions_deletions_replacements_and_boundaries() {
    const auto layout =
        Layout::calculate(Document("added-first\nsame\nnew\nlast"), Document("same\nold\nlast\nremoved-last"));
    CPPTEST_ASSERT(layout.rows().size() == 5U);
    CPPTEST_ASSERT(layout.rows()[0].kind == Kind::Added);
    CPPTEST_ASSERT(layout.rows()[0].left == 0U);
    CPPTEST_ASSERT(!layout.rows()[0].right);
    CPPTEST_ASSERT(layout.rows()[1].kind == Kind::Unchanged);
    CPPTEST_ASSERT(layout.rows()[2].kind == Kind::Modified);
    CPPTEST_ASSERT(layout.rows()[3].kind == Kind::Unchanged);
    CPPTEST_ASSERT(layout.rows()[4].kind == Kind::Removed);
    CPPTEST_ASSERT(!layout.rows()[4].left);
    CPPTEST_ASSERT(layout.rows()[4].right == 3U);
    return 0;
}

int test_handles_empty_text_and_repeated_equal_lines_deterministically() {
    const auto empty = Layout::calculate(Document(), Document());
    CPPTEST_ASSERT(empty.rows().size() == 1U);
    CPPTEST_ASSERT(empty.rows()[0].kind == Kind::Unchanged);

    const Document left("repeat\nleft\nrepeat\nend");
    const Document right("repeat\nright\nrepeat\nend");
    const auto first = Layout::calculate(left, right);
    const auto second = Layout::calculate(left, right);
    CPPTEST_ASSERT(first.rows() == second.rows());
    CPPTEST_ASSERT(first.rows().size() == 4U);
    CPPTEST_ASSERT(first.rows()[1].kind == Kind::Modified);
    CPPTEST_ASSERT(first.rows()[2].kind == Kind::Unchanged);
    return 0;
}

int test_large_documents_limit_recalculation_to_changed_middle() {
    constexpr std::size_t LineCount = 5000U;
    constexpr std::size_t ChangedLine = LineCount / 2U;
    std::string leftText;
    std::string rightText;
    for (std::size_t line = 0; line < LineCount; ++line) {
        const std::string value = "line " + std::to_string(line);
        leftText += line == ChangedLine ? "edited " + value : value;
        rightText += value;
        if (line + 1U < LineCount) {
            leftText += '\n';
            rightText += '\n';
        }
    }

    const auto layout = Layout::calculate(Document(leftText), Document(rightText));
    CPPTEST_ASSERT(layout.rows().size() == LineCount);
    CPPTEST_ASSERT(layout.rows()[ChangedLine - 1U].kind == Kind::Unchanged);
    CPPTEST_ASSERT(layout.rows()[ChangedLine].kind == Kind::Modified);
    CPPTEST_ASSERT(layout.rows()[ChangedLine + 1U].kind == Kind::Unchanged);
    return 0;
}

int test_maps_positions_ranges_and_gap_rows_in_both_directions() {
    const auto layout = Layout::calculate(Document("new\nsame\nend"), Document("same\nend\ngone"));
    CPPTEST_ASSERT(layout.visualRow(true, 0) == 0U);
    CPPTEST_ASSERT(layout.visualRow(false, 0) == 1U);
    CPPTEST_ASSERT(!layout.visualRow(true, 3));
    CPPTEST_ASSERT(!layout.logicalLine(false, 0));
    CPPTEST_ASSERT(!layout.logicalPosition(false, {0, 2}));
    CPPTEST_ASSERT((layout.logicalPosition(true, {0, 2}) == Position{0, 2}));
    CPPTEST_ASSERT((layout.visualPosition(false, {1, 3}) == VisualPosition{2, 3}));
    CPPTEST_ASSERT((layout.visualRange(false, {{0, 1}, {1, 2}}) == VisualRange{{1, 1}, {2, 2}}));
    return 0;
}

int test_recomputation_after_edit_uses_current_documents_without_mutation() {
    EditorSession current(Document("one\nthree"));
    const Document history("one\ntwo\nthree");
    const auto before = Layout::calculate(current.document(), history);
    CPPTEST_ASSERT(before.rows().size() == 3U);
    CPPTEST_ASSERT(before.rows()[1].kind == Kind::Removed);

    current.moveCaret({0, 3});
    CPPTEST_ASSERT(current.typeText(" changed"));
    const auto after = Layout::calculate(current.document(), history);
    CPPTEST_ASSERT(after.rows().size() == 3U);
    CPPTEST_ASSERT(after.rows()[0].kind == Kind::Modified);
    CPPTEST_ASSERT(after.rows()[1].kind == Kind::Removed);
    CPPTEST_ASSERT(current.document().text() == "one changed\nthree");
    CPPTEST_ASSERT(history.text() == "one\ntwo\nthree");
    return 0;
}

int test_folded_hides_collapsed_rows_without_mutating_documents() {
    const Document left("a\nb\nc\nd");
    const Document right("a\nb\nc\nd");

    Annotation::Model leftAnnotations;
    Annotation::Model rightAnnotations;
    leftAnnotations.setFoldRegions(left, {{"fold", {{1, 0}, {3, 1}}, true, "b ..."}});

    const Layout base = Layout::calculate(left, right);
    const Layout folded = Layout::folded(base, leftAnnotations, rightAnnotations);

    CPPTEST_ASSERT(base.rows().size() == 4U);
    CPPTEST_ASSERT(folded.rows().size() == 3U);
    CPPTEST_ASSERT(folded.visualRow(true, 1).has_value());
    CPPTEST_ASSERT(!folded.visualRow(true, 2).has_value());
    CPPTEST_ASSERT(!folded.visualRow(false, 2).has_value());

    CPPTEST_ASSERT(left.text() == "a\nb\nc\nd");
    CPPTEST_ASSERT(right.text() == "a\nb\nc\nd");
    return 0;
}

int main() {
    CPPTEST_RUN(test_aligns_gap_rows_without_changing_documents);
    CPPTEST_RUN(test_classifies_insertions_deletions_replacements_and_boundaries);
    CPPTEST_RUN(test_handles_empty_text_and_repeated_equal_lines_deterministically);
    CPPTEST_RUN(test_large_documents_limit_recalculation_to_changed_middle);
    CPPTEST_RUN(test_maps_positions_ranges_and_gap_rows_in_both_directions);
    CPPTEST_RUN(test_recomputation_after_edit_uses_current_documents_without_mutation);
    CPPTEST_RUN(test_folded_hides_collapsed_rows_without_mutating_documents);
    return 0;
}
