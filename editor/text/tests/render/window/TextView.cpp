#include "TestAssert.h"

#include <ScopeCanvas/editor/text/render/window/TextView.h>

using namespace ScopeCanvas::Editor::Text;
using namespace ScopeCanvas::Editor::Text::Render::Window;

int test_collapsed_fold_filters_projection_and_expands_again() {
    Session::EditorSession session(Document("SELECT\n    first_name,\n    last_name\nFROM people;"));
    TextView view(session);
    view.annotations().setFoldRegions(session.document(), {{"projection", {{0, 0}, {3, 0}}, false, "SELECT …"}});
    view.synchronizeFoldProjection();
    CPPTEST_ASSERT(view.projection().rows().size() == 4U);
    CPPTEST_ASSERT(view.toggleFold("projection"));
    CPPTEST_ASSERT(view.projection().rows().size() == 2U);
    CPPTEST_ASSERT(view.projection().rows()[0].logicalLine == 0U);
    CPPTEST_ASSERT(view.projection().rows()[1].logicalLine == 3U);
    CPPTEST_ASSERT(view.toggleFold("projection"));
    CPPTEST_ASSERT(view.projection().rows().size() == 4U);
    return 0;
}

int test_fold_gutter_hit_toggles_the_visible_control_only() {
    Session::EditorSession session(Document("SELECT\n    value\nFROM data;"));
    TextView view(session);
    view.settings().showMarkerGutter = true;
    view.annotations().setFoldRegions(session.document(), {{"projection", {{0, 0}, {2, 0}}, false, "SELECT …"}});
    view.synchronizeFoldProjection();
    const auto& frame = view.buildFrame(200.0F);
    CPPTEST_ASSERT(frame.rows[0].foldControl.has_value());
    CPPTEST_ASSERT(!view.toggleFoldAt(frame.codeOriginX + 1.0F, frame.lineHeight * 0.5F));
    CPPTEST_ASSERT(view.toggleFoldAt(0.0F, frame.lineHeight * 0.5F));
    CPPTEST_ASSERT(view.projection().rows().size() == 2U);
    return 0;
}

int main() {
    CPPTEST_RUN(test_collapsed_fold_filters_projection_and_expands_again);
    CPPTEST_RUN(test_fold_gutter_hit_toggles_the_visible_control_only);
    return 0;
}
