#include "TestAssert.h"

#include <ScopeCanvas/editor/text/annotation/Model.h>

using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text;

int test_normalizes_foreground_ranges_and_resolves_overlap_deterministically() {
    const Document document("abcdefgh");
    Model annotations;
    annotations.setForegroundSpans(document,
                                   {{{{0, 6}, {0, 1}}, {10, 20, 30, 255}, FontStyle::Italic, "base", "a", 1},
                                    {{{0, 2}, {0, 5}}, {40, 50, 60, 255}, FontStyle::Bold, "keyword", "b", 2},
                                    {{{0, 3}, {0, 4}}, {70, 80, 90, 255}, FontStyle::Regular, "later", "c", 2},
                                    {{{0, 8}, {0, 8}}, {}, FontStyle::Regular, {}, "empty", 9},
                                    {{{4, 0}, {4, 1}}, {}, FontStyle::Regular, {}, "invalid", 9}});

    CPPTEST_ASSERT(annotations.foregroundSpans().size() == 3U);
    const auto resolved = annotations.resolvedForeground(document);
    CPPTEST_ASSERT(resolved.size() == 5U);
    CPPTEST_ASSERT(resolved[0].startColumn == 1U);
    CPPTEST_ASSERT(resolved[0].endColumn == 2U);
    CPPTEST_ASSERT(resolved[0].id == "a");
    CPPTEST_ASSERT(resolved[1].id == "b");
    CPPTEST_ASSERT(resolved[2].startColumn == 3U);
    CPPTEST_ASSERT(resolved[2].endColumn == 4U);
    CPPTEST_ASSERT(resolved[2].id == "c");
    CPPTEST_ASSERT(resolved[3].id == "b");
    CPPTEST_ASSERT(resolved[4].id == "a");
    return 0;
}

int test_clips_background_and_diagnostic_multiline_ranges() {
    const Document document("abcd\nefgh\nijkl");
    Model annotations;
    annotations.setBackgroundSpans(document, {{{{0, 2}, {2, 2}}, {1, 2, 3, 4}, "selection", 0}});
    annotations.setDiagnostics(document, {{{{0, 3}, {1, 2}}, DiagnosticSeverity::Error, "broken", "diag"}});

    const auto background = annotations.clippedBackground(document, 0);
    CPPTEST_ASSERT(background == std::vector<LineSegment>({{0, 2, 4}, {1, 0, 4}, {2, 0, 2}}));
    const auto diagnostic = annotations.clippedDiagnostic(document, 0);
    CPPTEST_ASSERT(diagnostic == std::vector<LineSegment>({{0, 3, 4}, {1, 0, 2}}));
    CPPTEST_ASSERT(annotations.diagnostics()[0].severity == DiagnosticSeverity::Error);
    return 0;
}

int test_manages_fold_identity_state_and_stale_ranges() {
    Document left("added\none\ntwo\nthree");
    const Document right("one\ntwo\nthree");
    Model annotations;
    annotations.setFoldRegions(left, {{"body", {{1, 0}, {3, 5}}, true, "one ... three"},
                                      {"body", {{0, 0}, {0, 5}}, false, "duplicate"},
                                      {"", {{0, 0}, {0, 5}}, false, "missing id"}});
    CPPTEST_ASSERT(annotations.foldRegions().size() == 1U);
    CPPTEST_ASSERT(annotations.foldRegion("body")->collapsed);
    CPPTEST_ASSERT(annotations.setFoldCollapsed("body", false));
    CPPTEST_ASSERT(!annotations.foldRegion("body")->collapsed);
    CPPTEST_ASSERT(!annotations.setFoldCollapsed("unknown", true));

    left = Document("short");
    annotations.synchronize(left);
    CPPTEST_ASSERT(annotations.foldRegions().empty());
    return 0;
}

int main() {
    CPPTEST_RUN(test_normalizes_foreground_ranges_and_resolves_overlap_deterministically);
    CPPTEST_RUN(test_clips_background_and_diagnostic_multiline_ranges);
    CPPTEST_RUN(test_manages_fold_identity_state_and_stale_ranges);
    return 0;
}
