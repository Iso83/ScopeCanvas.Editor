#include "TestAssert.h"

#include <ScopeCanvas/editor/text/Document.h>

using namespace ScopeCanvas::Editor::Text;

int test_preserves_line_endings_and_utf8() {
    const std::string source = "alpha\r\nbeta\nUTF-8: caf\xC3\xA9";
    const Document document(source);
    CPPTEST_ASSERT(document.text() == source);
    CPPTEST_ASSERT(document.lineCount() == 3U);
    CPPTEST_ASSERT(document.lineEnding(0) == LineEnding::CRLF);
    CPPTEST_ASSERT(document.lineEnding(1) == LineEnding::LF);
    CPPTEST_ASSERT(document.lineEnding(2) == LineEnding::None);
    CPPTEST_ASSERT((!document.valid(Position{2, 11})));
    CPPTEST_ASSERT((document.position(document.offset({2, 12})) == Position{2, 12}));
    return 0;
}

int test_empty_and_final_newline_retain_their_exact_shape() {
    const Document empty;
    CPPTEST_ASSERT(empty.lineCount() == 1U);
    CPPTEST_ASSERT(empty.line(0).empty());
    CPPTEST_ASSERT(empty.lineEnding(0) == LineEnding::None);
    CPPTEST_ASSERT(empty.text().empty());

    const Document finalNewline("one\n");
    CPPTEST_ASSERT(finalNewline.lineCount() == 2U);
    CPPTEST_ASSERT(finalNewline.line(1).empty());
    CPPTEST_ASSERT(finalNewline.lineEnding(0) == LineEnding::LF);
    CPPTEST_ASSERT(finalNewline.lineEnding(1) == LineEnding::None);
    CPPTEST_ASSERT(finalNewline.text() == "one\n");
    return 0;
}

int test_positions_use_utf8_byte_columns_and_reject_partial_offsets() {
    const Document document("a\t\xC3\xA9\r\nlast");
    CPPTEST_ASSERT((document.valid(Position{0, 0})));
    CPPTEST_ASSERT((document.valid(Position{0, 2})));
    CPPTEST_ASSERT((!document.valid(Position{0, 3})));
    CPPTEST_ASSERT(document.valid({{1, 4}, {0, 0}}));
    CPPTEST_ASSERT(!document.valid({{0, 3}, {1, 4}}));
    CPPTEST_ASSERT(document.offset({0, 4}) == 4U);
    CPPTEST_ASSERT(document.offset({1, 0}) == 6U);
    CPPTEST_ASSERT((document.position(2) == Position{0, 2}));
    CPPTEST_ASSERT((document.position(6) == Position{1, 0}));

    bool splitUtf8Rejected = false;
    bool splitCrlfRejected = false;
    bool pastEndRejected = false;
    try {
        (void)document.position(3);
    } catch (const std::out_of_range&) {
        splitUtf8Rejected = true;
    }
    try {
        (void)document.position(5);
    } catch (const std::out_of_range&) {
        splitCrlfRejected = true;
    }
    try {
        (void)document.position(11);
    } catch (const std::out_of_range&) {
        pastEndRejected = true;
    }
    CPPTEST_ASSERT(splitUtf8Rejected);
    CPPTEST_ASSERT(splitCrlfRejected);
    CPPTEST_ASSERT(pastEndRejected);
    return 0;
}

int test_string_and_file_round_trips_preserve_mixed_endings() {
    const std::string source = "first\r\nsecond\nthird\rtext";
    const auto path = std::filesystem::temp_directory_path() / "codestructure-text-document-roundtrip.txt";
    Document(source).save(path);
    const auto loaded = Document::load(path).text();
    std::filesystem::remove(path);
    CPPTEST_ASSERT(loaded == source);
    return 0;
}

int test_insert_erase_and_replace_work_across_lines() {
    Document document("one\r\ntwo\nthree");
    CPPTEST_ASSERT((document.insert({0, 3}, "!\nnext") == Position{1, 4}));
    CPPTEST_ASSERT(document.text() == "one!\nnext\r\ntwo\nthree");
    document.erase({{1, 2}, {2, 1}});
    CPPTEST_ASSERT(document.text() == "one!\nnewo\nthree");
    CPPTEST_ASSERT((document.replace({{2, 5}, {0, 0}}, "done") == Position{0, 4}));
    CPPTEST_ASSERT(document.text() == "done");
    return 0;
}

int test_multiline_ranges_clip_first_intermediate_and_final_lines() {
    const Document document("abcd\nefgh\nijkl");
    const auto segments = document.clipToLines({{0, 2}, {2, 2}});
    CPPTEST_ASSERT(segments.size() == 3U);
    CPPTEST_ASSERT(segments[0].startColumn == 2U);
    CPPTEST_ASSERT(segments[0].endColumn == 4U);
    CPPTEST_ASSERT(segments[1].startColumn == 0U);
    CPPTEST_ASSERT(segments[1].endColumn == 4U);
    CPPTEST_ASSERT(segments[2].endColumn == 2U);
    return 0;
}

int test_returns_text_with_visible_hidden_characters() {
    Document document("one One stone\r\nword\tend\nlast");

    CPPTEST_ASSERT(document.visibleText(0, true) == "one·One·stone␍␊");
    CPPTEST_ASSERT(document.visibleText(1, true) == "word→\tend␊");
    CPPTEST_ASSERT(document.visibleText(2, true) == "last⟂");
    CPPTEST_ASSERT(document.visibleText(2, false) == "last");
    return 0;
}

int main() {
    CPPTEST_RUN(test_preserves_line_endings_and_utf8);
    CPPTEST_RUN(test_empty_and_final_newline_retain_their_exact_shape);
    CPPTEST_RUN(test_positions_use_utf8_byte_columns_and_reject_partial_offsets);
    CPPTEST_RUN(test_string_and_file_round_trips_preserve_mixed_endings);
    CPPTEST_RUN(test_insert_erase_and_replace_work_across_lines);
    CPPTEST_RUN(test_multiline_ranges_clip_first_intermediate_and_final_lines);
    CPPTEST_RUN(test_returns_text_with_visible_hidden_characters);
    return 0;
}
