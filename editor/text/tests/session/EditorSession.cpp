#include "TestAssert.h"

#include <ScopeCanvas/editor/text/session/EditorSession.h>

using namespace ScopeCanvas::Editor::Text;
using namespace ScopeCanvas::Editor::Text::Session;

int test_multiline_edits_and_history_round_trip_exactly() {
    EditorSession editor(Document("one\r\ntwo\nthree"));
    editor.select({0, 1}, {1, 2});
    CPPTEST_ASSERT(editor.copy() == "ne\r\ntw");
    CPPTEST_ASSERT(editor.replaceSelection("X\nY"));
    CPPTEST_ASSERT(editor.document().text() == "oX\nYo\nthree");
    CPPTEST_ASSERT(editor.undo());
    CPPTEST_ASSERT(editor.document().text() == "one\r\ntwo\nthree");
    CPPTEST_ASSERT(editor.redo());
    CPPTEST_ASSERT(editor.document().text() == "oX\nYo\nthree");
    return 0;
}

int test_read_only_rejects_every_mutation() {
    EditorSession reference(Document("locked"), true);
    reference.select({0, 0}, {0, 6});
    CPPTEST_ASSERT(reference.copy() == "locked");
    CPPTEST_ASSERT(!reference.typeText("changed"));
    CPPTEST_ASSERT(!reference.newline());
    CPPTEST_ASSERT(!reference.replaceSelection("changed"));
    CPPTEST_ASSERT(!reference.backspace());
    CPPTEST_ASSERT(!reference.deleteForward());
    CPPTEST_ASSERT(!reference.cut().has_value());
    CPPTEST_ASSERT(!reference.paste("changed"));
    CPPTEST_ASSERT(!reference.canUndo());
    CPPTEST_ASSERT(reference.document().text() == "locked");
    return 0;
}

int test_selection_keeps_anchor_while_active_endpoint_crosses_it() {
    EditorSession editor(Document("abcdef"));
    editor.select({0, 3}, {0, 5});
    CPPTEST_ASSERT((editor.selection().normalized() == Range{{0, 3}, {0, 5}}));
    editor.moveCaret({0, 1}, true);
    CPPTEST_ASSERT((editor.anchor() == Position{0, 3}));
    CPPTEST_ASSERT((editor.caret() == Position{0, 1}));
    CPPTEST_ASSERT((editor.selection().normalized() == Range{{0, 1}, {0, 3}}));
    editor.moveCaret({0, 4}, true);
    CPPTEST_ASSERT((editor.anchor() == Position{0, 3}));
    CPPTEST_ASSERT((editor.selection().normalized() == Range{{0, 3}, {0, 4}}));
    return 0;
}

int test_editing_commands_handle_utf8_line_endings_and_clipboard_text() {
    EditorSession editor(Document("a\xC3\xA9\r\nbeta"));
    editor.select({0, 1}, {0, 3});
    CPPTEST_ASSERT(editor.backspace());
    CPPTEST_ASSERT(editor.document().text() == "a\r\nbeta");

    editor.moveCaret({1, 0});
    CPPTEST_ASSERT(editor.backspace());
    CPPTEST_ASSERT(editor.document().text() == "abeta");
    editor.moveCaret({0, 1});
    CPPTEST_ASSERT(editor.newline());
    CPPTEST_ASSERT(editor.document().text() == "a\nbeta");

    editor.select({0, 0}, {1, 1});
    auto cut = editor.cut();
    CPPTEST_ASSERT(cut.has_value());
    CPPTEST_ASSERT(*cut == "a\nb");
    CPPTEST_ASSERT(editor.document().text() == "eta");
    CPPTEST_ASSERT(editor.paste(*cut));
    CPPTEST_ASSERT(editor.document().text() == "a\nbeta");
    return 0;
}

int test_forward_delete_removes_whole_code_points_and_crosses_lines() {
    EditorSession editor(Document("a\xC3\xA9\r\nb"));
    editor.moveCaret({0, 1});
    CPPTEST_ASSERT(editor.deleteForward());
    CPPTEST_ASSERT(editor.document().text() == "a\r\nb");
    CPPTEST_ASSERT(editor.undo());
    CPPTEST_ASSERT(editor.document().text() == "a\xC3\xA9\r\nb");
    CPPTEST_ASSERT((editor.caret() == Position{0, 1}));
    CPPTEST_ASSERT((editor.anchor() == Position{0, 1}));
    CPPTEST_ASSERT(editor.redo());
    CPPTEST_ASSERT(editor.deleteForward());
    CPPTEST_ASSERT(editor.document().text() == "ab");
    return 0;
}

int test_history_coalesces_typing_but_not_unrelated_commands() {
    EditorSession editor;
    CPPTEST_ASSERT(editor.typeText("a"));
    CPPTEST_ASSERT(editor.typeText("b"));
    CPPTEST_ASSERT(editor.typeText("c"));
    CPPTEST_ASSERT(editor.document().text() == "abc");
    CPPTEST_ASSERT(editor.undo());
    CPPTEST_ASSERT(editor.document().text().empty());
    CPPTEST_ASSERT(editor.redo());
    CPPTEST_ASSERT(editor.document().text() == "abc");

    CPPTEST_ASSERT(editor.newline());
    CPPTEST_ASSERT(editor.typeText("d"));
    CPPTEST_ASSERT(editor.undo());
    CPPTEST_ASSERT(editor.document().text() == "abc\n");
    CPPTEST_ASSERT(editor.undo());
    CPPTEST_ASSERT(editor.document().text() == "abc");
    return 0;
}

int test_history_restores_selection_and_preferred_visual_column() {
    EditorSession editor(Document("first\nsecond"));
    editor.select({0, 1}, {1, 3});
    editor.setPreferredVisualColumn(8);
    CPPTEST_ASSERT(editor.replaceSelection("x"));
    CPPTEST_ASSERT(editor.document().text() == "fxond");
    CPPTEST_ASSERT(editor.undo());
    CPPTEST_ASSERT(editor.document().text() == "first\nsecond");
    CPPTEST_ASSERT((editor.anchor() == Position{0, 1}));
    CPPTEST_ASSERT((editor.caret() == Position{1, 3}));
    CPPTEST_ASSERT(editor.preferredVisualColumn() == 8U);
    CPPTEST_ASSERT(editor.redo());
    CPPTEST_ASSERT((editor.caret() == Position{0, 2}));
    return 0;
}

int test_normal_mode_rejects_text_mutations() {
    EditorSession editor(Document("text"));
    editor.setMode(Mode::Normal);
    editor.select({0, 0}, {0, 4});
    CPPTEST_ASSERT(editor.copy() == "text");
    CPPTEST_ASSERT(!editor.typeText("x"));
    CPPTEST_ASSERT(!editor.paste("x"));
    CPPTEST_ASSERT(!editor.cut().has_value());
    CPPTEST_ASSERT(editor.document().text() == "text");
    return 0;
}

int test_replaces_all_matches_as_one_undoable_operation() {
    EditorSession editor(Document("cat catalog cat\ncat"));

    CPPTEST_ASSERT(editor.replaceAll("cat", "dog", {true, true}) == 3U);
    CPPTEST_ASSERT(editor.document().text() == "dog catalog dog\ndog");

    CPPTEST_ASSERT(editor.undo());
    CPPTEST_ASSERT(editor.document().text() == "cat catalog cat\ncat");

    EditorSession readonly(Document("cat"), true);
    CPPTEST_ASSERT(readonly.replaceAll("cat", "dog", {}) == 0U);
    CPPTEST_ASSERT(readonly.document().text() == "cat");
    return 0;
}

int main() {
    CPPTEST_RUN(test_multiline_edits_and_history_round_trip_exactly);
    CPPTEST_RUN(test_read_only_rejects_every_mutation);
    CPPTEST_RUN(test_selection_keeps_anchor_while_active_endpoint_crosses_it);
    CPPTEST_RUN(test_editing_commands_handle_utf8_line_endings_and_clipboard_text);
    CPPTEST_RUN(test_forward_delete_removes_whole_code_points_and_crosses_lines);
    CPPTEST_RUN(test_history_coalesces_typing_but_not_unrelated_commands);
    CPPTEST_RUN(test_history_restores_selection_and_preferred_visual_column);
    CPPTEST_RUN(test_normal_mode_rejects_text_mutations);
    CPPTEST_RUN(test_replaces_all_matches_as_one_undoable_operation);
    return 0;
}
