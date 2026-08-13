#include "TestAssert.h"

#include <ScopeCanvas/editor/text/diff/Layout.h>
#include <ScopeCanvas/editor/text/render/window/Model.h>
#include <ScopeCanvas/editor/text/zoom/Controller.h>

using namespace ScopeCanvas::Editor::Text;
using namespace ScopeCanvas::Editor::Text::Diff;
using namespace ScopeCanvas::Editor::Text::Render::Window;
using namespace ScopeCanvas::Editor::Text::Zoom;

int test_scales_metrics_hit_testing_and_preserves_anchor() {
    Zoom::Controller zoom{{0.5F, 2.0F, 1.25F}};
    CPPTEST_ASSERT(zoom.zoomIn());
    const Settings base{20.0F, 8.0F, 6.0F, 24.0F, 10.0F, 1, 4.0F, 12.0F};
    const Settings scaled = zoom.effectiveSettings(base, {
                                                             .ascent = 15.0F,
                                                             .descent = -5.0F,
                                                             .lineHeight = 20.0F,
                                                             .glyphAdvance = 8.0F,
                                                             .pixelSize = 16U,
                                                         });
    CPPTEST_ASSERT(scaled.lineHeight == 25.0F);
    CPPTEST_ASSERT(scaled.glyphAdvance == 10.0F);
    CPPTEST_ASSERT(scaled.markerGutterWidth == 30.0F);
    CPPTEST_ASSERT(scaled.fontWorldScale == 1.25F);

    const Document document("one\ntwo\nthree\nfour");
    const Layout layout = Layout::calculate(document, document);
    const Frame frame = Render::Window::Model{}.buildFrame(document, layout.leftProjection(), {}, nullptr, scaled,
                                                           {15.0F, 2.0F}, 50.0F);
    CPPTEST_ASSERT(frame.rows.size() == 3U);
    CPPTEST_ASSERT(frame.lineHeight == 25.0F);
    CPPTEST_ASSERT(frame.glyphAdvance == 10.0F);
    CPPTEST_ASSERT(frame.fontWorldScale == 1.25F);
    CPPTEST_ASSERT(zoom.hitTest(frame, frame.codeOriginX + 26.0F, 27.0F).visualRow == 3U);
    CPPTEST_ASSERT(zoom.hitTest(frame, frame.codeOriginX + 26.0F, 27.0F).column == 4U);
    const float preserved = zoom.preserveVisualAnchor(10.0F, 40.0F, 20.0F, 25.0F);
    CPPTEST_ASSERT(std::abs(preserved - 10.4F) < 0.001F);
    CPPTEST_ASSERT(zoom.setZoom(10.0F));
    CPPTEST_ASSERT(zoom.zoom() == 2.0F);
    CPPTEST_ASSERT(zoom.reset());
    CPPTEST_ASSERT(zoom.zoom() == 1.0F);
    return 0;
}

int test_covers_scale_derived_visible_rows_and_hit_test_edges() {
    Zoom::Controller zoom{{0.25F, 4.0F, 2.0F}};
    CPPTEST_ASSERT(zoom.setZoom(0.5F));
    const Settings base{18.0F, 9.0F, 5.0F, 24.0F, 7.0F, 0, 3.0F, 10.0F};
    const Settings small = zoom.effectiveSettings(
        base, {.ascent = 12.0F, .descent = -4.0F, .lineHeight = 18.0F, .glyphAdvance = 9.0F, .pixelSize = 14U});
    CPPTEST_ASSERT(small.lineHeight == 9.0F);
    CPPTEST_ASSERT(small.glyphAdvance == 4.5F);
    CPPTEST_ASSERT(small.lineNumberPadding == 2.5F);
    CPPTEST_ASSERT(small.markerGutterWidth == 12.0F);
    CPPTEST_ASSERT(small.codePadding == 3.5F);
    CPPTEST_ASSERT(small.gutterSpacing == 1.5F);
    CPPTEST_ASSERT(small.foldControlWidth == 5.0F);

    const Document document("zero\none\ntwo\nthree\nfour");
    const Layout layout = Layout::calculate(document, document);
    const Frame smallFrame =
        Render::Window::Model{}.buildFrame(document, layout.leftProjection(), {}, nullptr, small, {0.0F, 2.0F}, 26.9F);
    CPPTEST_ASSERT(smallFrame.rows.size() == 3U);
    CPPTEST_ASSERT(smallFrame.firstVisibleRow == 2U);
    CPPTEST_ASSERT(zoom.hitTest(smallFrame, smallFrame.codeOriginX - 1.0F, -5.0F).visualRow == 2U);
    CPPTEST_ASSERT(!zoom.hitTest(smallFrame, smallFrame.codeOriginX - 1.0F, -5.0F).inCodeArea);
    CPPTEST_ASSERT(zoom.hitTest(smallFrame, smallFrame.codeOriginX + 8.9F, 17.9F).visualRow == 3U);
    CPPTEST_ASSERT(zoom.hitTest(smallFrame, smallFrame.codeOriginX + 8.9F, 17.9F).column == 1U);

    CPPTEST_ASSERT(zoom.setZoom(3.0F));
    const Settings large = zoom.effectiveSettings(
        base, {.ascent = 12.0F, .descent = -4.0F, .lineHeight = 18.0F, .glyphAdvance = 9.0F, .pixelSize = 14U});
    const Frame largeFrame = Render::Window::Model{}.buildFrame(document, layout.leftProjection(), {}, nullptr, large,
                                                                {20.0F, 0.0F}, 108.0F);
    CPPTEST_ASSERT(largeFrame.rows.size() == 2U);
    CPPTEST_ASSERT(largeFrame.lineHeight == 54.0F);
    CPPTEST_ASSERT(largeFrame.glyphAdvance == 27.0F);
    CPPTEST_ASSERT(zoom.hitTest(largeFrame, largeFrame.codeOriginX + 80.9F, 107.9F).visualRow == 1U);
    CPPTEST_ASSERT(zoom.hitTest(largeFrame, largeFrame.codeOriginX + 80.9F, 107.9F).column == 3U);
    CPPTEST_ASSERT(std::abs(zoom.preserveVisualAnchor(4.0F, 18.0F, 9.0F, 54.0F) - 5.6666665F) < 0.001F);
    return 0;
}

int main() {
    CPPTEST_RUN(test_scales_metrics_hit_testing_and_preserves_anchor);
    CPPTEST_RUN(test_covers_scale_derived_visible_rows_and_hit_test_edges);
    return 0;
}
