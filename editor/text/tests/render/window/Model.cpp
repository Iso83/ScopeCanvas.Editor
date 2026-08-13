#include "TestAssert.h"

#include <ScopeCanvas/editor/text/diff/Layout.h>
#include <ScopeCanvas/editor/text/render/window/Frame.h>
#include <ScopeCanvas/editor/text/render/window/Model.h>
#include <ScopeCanvas/editor/text/session/EditorSession.h>
#include <ScopeCanvas/editor/text/zoom/Controller.h>

using namespace ScopeCanvas::Editor::Text;
using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text::Diff;
using namespace ScopeCanvas::Editor::Text::Session;
using namespace ScopeCanvas::Editor::Text::Render::Window;
using namespace ScopeCanvas::Editor::Text::Zoom;

int test_clips_rows_and_never_numbers_diff_gaps() {

    const Document left("added\none\ntwo\nthree");
    const Document right("one\ntwo\nthree");
    const Layout layout = Layout::calculate(left, right);
    const Annotation::Model annotations;
    Settings settings{20.0F, 8.0F, 6.0F, 24.0F, 10.0F, 1, 4.0F, 12.0F};
    settings.showLineNumbers = true;
    settings.showMarkerGutter = true;
    const Frame rightFrame = Render::Window::Model{}.buildFrame(right, layout.rightProjection(), annotations, nullptr,
                                                                settings, {17.0F, 0.0F}, 40.0F);

    CPPTEST_ASSERT(rightFrame.firstVisibleRow == 0U);
    CPPTEST_ASSERT(rightFrame.endVisibleRow == 3U);
    CPPTEST_ASSERT(rightFrame.rows.size() == 3U);
    CPPTEST_ASSERT(rightFrame.rows[0].gap);
    CPPTEST_ASSERT(!rightFrame.rows[0].logicalLine);
    CPPTEST_ASSERT(rightFrame.rows[0].lineNumber.empty());
    CPPTEST_ASSERT(rightFrame.rows[0].text.empty());
    CPPTEST_ASSERT(rightFrame.rows[1].lineNumber == "1");
    CPPTEST_ASSERT(rightFrame.rows[1].text == "one");
    CPPTEST_ASSERT(rightFrame.markerGutterWidth == 24.0F);
    CPPTEST_ASSERT(rightFrame.codeOriginX == rightFrame.lineNumberGutterWidth + 38.0F);
    CPPTEST_ASSERT(rightFrame.horizontalOffset == 17.0F);
    CPPTEST_ASSERT(rightFrame.lineHeight == 20.0F);
    CPPTEST_ASSERT(rightFrame.glyphAdvance == 8.0F);
    CPPTEST_ASSERT(rightFrame.layerOrder.front() == Layer::RowBackground);
    CPPTEST_ASSERT(rightFrame.layerOrder.back() == Layer::FoldControl);

    const Frame scrolled = Render::Window::Model{}.buildFrame(left, layout.leftProjection(), annotations, nullptr,
                                                              settings, {3.0F, 3.0F}, 20.0F);
    CPPTEST_ASSERT(scrolled.firstVisibleRow == 2U);
    CPPTEST_ASSERT(scrolled.endVisibleRow == 4U);
    return 0;
}

int test_treats_stale_layout_lines_as_gaps() {
    const Document previous("zero\none\ntwo\nthree");
    const Layout staleLayout = Layout::calculate(previous, previous);
    const Document replacement("sample");
    const Frame frame =
        Render::Window::Model{}.buildFrame(replacement, staleLayout.leftProjection(), {}, nullptr, {}, {}, 200.0F);
    CPPTEST_ASSERT(frame.rows.size() == staleLayout.rows().size());
    CPPTEST_ASSERT(frame.rows[0].logicalLine == 0U);
    CPPTEST_ASSERT(frame.rows[0].text == "sample");
    for (std::size_t row = 1; row < frame.rows.size(); ++row) {
        CPPTEST_ASSERT(frame.rows[row].gap);
        CPPTEST_ASSERT(!frame.rows[row].logicalLine.has_value());
        CPPTEST_ASSERT(frame.rows[row].lineNumber.empty());
        CPPTEST_ASSERT(frame.rows[row].text.empty());
    }
    return 0;
}

int test_carries_editor_layers_without_marking_gaps() {
    const Document document("alpha\nbeta\ngamma");
    const Layout layout = Layout::calculate(document, document);
    Annotation::Model annotations;
    annotations.setBackgroundSpans(document, {{{{0, 1}, {1, 2}}, {20, 40, 80, 255}, "bg", 1}});
    annotations.setForegroundSpans(document,
                                   {{{{1, 1}, {1, 4}}, {180, 220, 255, 255}, FontStyle::Bold, "kw", "fg", 2}});
    annotations.setDiagnostics(document, {{{{1, 0}, {2, 2}}, DiagnosticSeverity::Error, "bad", "diag"}});
    annotations.setFoldRegions(document, {{"fold", {{1, 0}, {2, 5}}, true, "beta ..."}});
    EditorSession session(document);
    session.select({0, 2}, {1, 3});

    const Frame frame = Render::Window::Model{}.buildFrame(document, layout.leftProjection(), annotations, &session, {},
                                                           {}, 80.0F, true);
    CPPTEST_ASSERT(frame.rows.size() == 3U);
    CPPTEST_ASSERT(frame.rows[0].selection.has_value());
    CPPTEST_ASSERT(frame.rows[0].selection->startColumn == 2U);
    CPPTEST_ASSERT(frame.rows[0].selection->endColumn == 5U);
    CPPTEST_ASSERT(frame.rows[0].backgroundSpans.size() == 1U);
    CPPTEST_ASSERT(frame.rows[0].backgroundSpans[0].startColumn == 1U);
    CPPTEST_ASSERT(frame.rows[1].currentLine);
    CPPTEST_ASSERT(frame.rows[1].caret.has_value());
    CPPTEST_ASSERT(frame.rows[1].caret->column == 3U);
    CPPTEST_ASSERT(frame.rows[1].foregroundSpans.size() == 1U);
    CPPTEST_ASSERT(frame.rows[1].diagnostics.size() == 1U);
    CPPTEST_ASSERT(frame.rows[1].diagnostics[0].severity == DiagnosticSeverity::Error);
    CPPTEST_ASSERT(frame.rows[1].foldControl.has_value());
    CPPTEST_ASSERT(frame.rows[1].foldControl->id == "fold");
    CPPTEST_ASSERT(frame.rows[1].foldControl->collapsed);
    CPPTEST_ASSERT(!frame.rows[1].foldGutterGuide);
    return 0;
}

int test_row_geometry_uses_shared_origin_and_fractional_scroll() {
    Document document("zero\none\ntwo\nthree");
    const Layout layout = Layout::calculate(document, document);
    Settings settings{};
    settings.lineHeight = 25.5F;
    settings.fontAscent = 17.25F;
    settings.fontDescent = -4.25F;
    Scroll scroll{};
    scroll.verticalRows = 1.25F;
    const Frame frame =
        Render::Window::Model{}.buildFrame(document, layout.leftProjection(), {}, nullptr, settings, scroll, 51.0F);
    CPPTEST_ASSERT(std::abs(frame.verticalRows - 1.25F) < 0.001F);
    CPPTEST_ASSERT(std::abs(frame.baselineFromRowTop - 19.25F) < 0.001F);

    const auto firstHit = Zoom::Controller{}.hitTest(frame, frame.codeOriginX, 0.0F);
    const auto secondHit = Zoom::Controller{}.hitTest(frame, frame.codeOriginX, 20.0F);
    CPPTEST_ASSERT(firstHit.visualRow == 1U);
    CPPTEST_ASSERT(secondHit.visualRow == 2U);
    Scroll otherScroll = scroll;
    otherScroll.horizontalPixels = 37.0F;
    const Frame otherFrame = Render::Window::Model{}.buildFrame(document, layout.rightProjection(), {}, nullptr,
                                                                settings, otherScroll, 51.0F);
    CPPTEST_ASSERT(frame.horizontalOffset == 0.0F);
    CPPTEST_ASSERT(otherFrame.horizontalOffset == 37.0F);
    for (std::size_t row = 0; row < 100U; ++row) {
        const float direct = (static_cast<float>(row) - frame.verticalRows) * frame.lineHeight;
        const float next = (static_cast<float>(row + 1U) - frame.verticalRows) * frame.lineHeight;
        CPPTEST_ASSERT(std::abs((next - direct) - frame.lineHeight) < 0.001F);
    }
    return 0;
}

int test_maps_tabs_hidden_markers_and_utf8_columns() {
    Document utf8Document("a\xC3\xA9\tb");
    EditorSession utf8Session(utf8Document);
    utf8Session.moveCaret({0, 3});
    const Frame utf8Frame = Render::Window::Model{}.buildFrame(
        utf8Session.document(), Layout::calculate(utf8Session.document(), utf8Session.document()).leftProjection(), {},
        &utf8Session, {}, {}, 80.0F, true, true);
    CPPTEST_ASSERT(Render::Window::Model::visualColumnCount(utf8Document.line(0)) == 5U);
    CPPTEST_ASSERT(utf8Frame.rows[0].sourceToVisualColumns[3] == 2U);
    CPPTEST_ASSERT(utf8Frame.rows[0].caret->column == 2U);
    CPPTEST_ASSERT(utf8Frame.rows[0].visualToSourceColumns[2] == 3U);
    return 0;
}

int test_maps_find_highlights_to_frame_rows() {
    Document document("before\nmatch");
    const auto matches = Search::findMatches(document, "match", {});

    CPPTEST_ASSERT(matches.size() == 1U);

    Annotation::Model highlights;
    highlights.setBackgroundSpans(document, {{matches[0].range, {255, 175, 35, 190}, "active-find", 50}});

    const Frame frame = Render::Window::Model{}.buildFrame(
        document, Layout::calculate(document, document).leftProjection(), highlights, nullptr, {}, {}, 80.0F);

    CPPTEST_ASSERT(frame.rows[1].backgroundSpans.size() == 1U);
    CPPTEST_ASSERT(frame.rows[1].backgroundSpans[0].startColumn == 0U);
    CPPTEST_ASSERT(frame.rows[1].backgroundSpans[0].endColumn == 5U);
    return 0;
}

int test_builds_collapsed_fold_text_and_expanded_guides() {
    const Document document("a\nb\nc\nd");

    Annotation::Model annotations;
    annotations.setFoldRegions(document, {{"fold", {{1, 0}, {3, 1}}, true, "b ..."}});

    const Layout base = Layout::calculate(document, document);
    const Layout folded = Layout::folded(base, annotations, Annotation::Model{});

    const Frame collapsed =
        Render::Window::Model{}.buildFrame(document, folded.leftProjection(), annotations, nullptr, {}, {}, 80.0F);

    CPPTEST_ASSERT(collapsed.rows[1].foldControl.has_value());
    CPPTEST_ASSERT(collapsed.rows[1].text == "b ...");

    annotations.setFoldCollapsed("fold", false);

    const Frame expanded =
        Render::Window::Model{}.buildFrame(document, base.leftProjection(), annotations, nullptr, {}, {}, 80.0F);

    CPPTEST_ASSERT(expanded.rows[1].foldControl.has_value());
    CPPTEST_ASSERT(expanded.rows[1].foldGutterGuide);
    CPPTEST_ASSERT(expanded.rows[2].foldGutterGuide);
    CPPTEST_ASSERT(expanded.rows[3].foldGutterGuide);
    return 0;
}

int test_identity_projection_compact_gutter_and_decorations() {
    const Document document("alpha\nbeta\ngamma");
    Annotation::Model annotations;
    annotations.setDecorations(document,
                               {{{{0, 2}, {2, 2}}, UnderlineStyle::Straight, {10, 20, 30, 255}, "link", "nav", 4}});
    Settings compact;
    compact.codePadding = 3.0F;
    const Frame frame = Render::Window::Model{}.buildFrame(document, Projection::Projected::identity(document),
                                                           annotations, nullptr, compact, {}, 80.0F);
    CPPTEST_ASSERT(frame.lineNumberGutterWidth == 0.0F);
    CPPTEST_ASSERT(frame.markerGutterWidth == 0.0F);
    CPPTEST_ASSERT(frame.codeOriginX == 3.0F);
    CPPTEST_ASSERT(frame.rows[0].lineNumber.empty());
    CPPTEST_ASSERT(frame.rows[0].decorations[0].startColumn == 2U);
    CPPTEST_ASSERT(frame.rows[0].decorations[0].endColumn == 5U);
    CPPTEST_ASSERT(frame.rows[1].decorations[0].startColumn == 0U);
    CPPTEST_ASSERT(frame.rows[2].decorations[0].endColumn == 2U);
    CPPTEST_ASSERT(std::ranges::find(frame.layerOrder, Layer::Decoration) != frame.layerOrder.end());
    return 0;
}

int main() {
    CPPTEST_RUN(test_clips_rows_and_never_numbers_diff_gaps);
    CPPTEST_RUN(test_treats_stale_layout_lines_as_gaps);
    CPPTEST_RUN(test_carries_editor_layers_without_marking_gaps);
    CPPTEST_RUN(test_row_geometry_uses_shared_origin_and_fractional_scroll);
    CPPTEST_RUN(test_maps_tabs_hidden_markers_and_utf8_columns);
    CPPTEST_RUN(test_maps_find_highlights_to_frame_rows);
    CPPTEST_RUN(test_builds_collapsed_fold_text_and_expanded_guides);
    CPPTEST_RUN(test_identity_projection_compact_gutter_and_decorations);
    return 0;
}
