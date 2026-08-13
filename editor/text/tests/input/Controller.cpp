#include "TestAssert.h"

#include <ScopeCanvas/editor/text/input/Controller.h>
#include <ScopeCanvas/editor/text/projection/Projected.h>
#include <ScopeCanvas/editor/text/render/window/Model.h>

using namespace ScopeCanvas::Editor::Text;
using namespace ScopeCanvas::Editor::Text::Input;
using namespace ScopeCanvas::Editor::Text::Render::Window;
using namespace ScopeCanvas::Editor::Text::Session;

int test_single_session_input_and_read_only_boundary() {
    EditorSession session(Document("abc"));
    Scroll scroll;
    Controller input(session, scroll);
    const auto now = std::chrono::steady_clock::time_point{};
    CPPTEST_ASSERT(input.focus(now).handled);
    CPPTEST_ASSERT(input.caretVisible(now));
    CPPTEST_ASSERT(input.textInput("!").documentChanged);
    CPPTEST_ASSERT(session.document().text() == "!abc");

    EditorSession history(Document("history"), true);
    Controller readOnly(history, scroll);
    readOnly.focus(now);
    CPPTEST_ASSERT(!readOnly.textInput("blocked").handled);
    CPPTEST_ASSERT(history.document().text() == "history");
    return 0;
}

int test_navigation_and_projected_mouse_hit() {
    EditorSession session(Document("one two\nabc\nabcdef"));
    Scroll scroll;
    Controller input(session, scroll);
    input.focus({});
    input.key(Key::End, {});
    input.key(Key::WordLeft, {true, true});
    CPPTEST_ASSERT((session.anchor() == Position{0, 7}));
    CPPTEST_ASSERT((session.caret() == Position{0, 4}));

    Settings settings;
    const auto frame = Model{}.buildFrame(session.document(), Projection::Projected::identity(session.document()), {},
                                          &session, settings, scroll, 80.0F, true, true);
    input.mousePress({frame, 80.0F}, frame.codeOriginX + frame.glyphAdvance, frame.lineHeight * 1.2F, {}, {});
    CPPTEST_ASSERT((session.caret() == Position{1, 1}));
    CPPTEST_ASSERT(input.mouseDrag({frame, 20.0F}, frame.codeOriginX, 30.0F, {}).scrollDirty);
    CPPTEST_ASSERT(scroll.verticalRows == 1.0F);
    return 0;
}

int main() {
    CPPTEST_RUN(test_single_session_input_and_read_only_boundary);
    CPPTEST_RUN(test_navigation_and_projected_mouse_hit);
    return 0;
}
