#include "TestAssert.h"

#include <ScopeCanvas/editor/text/Search.h>

using namespace ScopeCanvas::Editor::Text;

int test_finds_matches_with_traversal_and_options() {
    Document document("one One stone\r\nword\tend\nlast");

    auto matches = Search::findMatches(document, "one", {});
    CPPTEST_ASSERT(matches.size() == 3U);
    CPPTEST_ASSERT(Search::nextMatchIndex(matches, {0, 1}, false) == 1U);
    CPPTEST_ASSERT(Search::nextMatchIndex(matches, {1, 0}, false) == 0U);
    CPPTEST_ASSERT(Search::nextMatchIndex(matches, {0, 1}, true) == 0U);

    matches = Search::findMatches(document, "one", {true, true});
    CPPTEST_ASSERT(matches.size() == 1U);
    CPPTEST_ASSERT((matches[0].range == Range{{0, 0}, {0, 3}}));

    Document finalLine("before\nmatch");
    matches = Search::findMatches(finalLine, "match", {});
    CPPTEST_ASSERT(matches.size() == 1U);
    CPPTEST_ASSERT((matches[0].range == Range{{1, 0}, {1, 5}}));
    return 0;
}

int main() {
    CPPTEST_RUN(test_finds_matches_with_traversal_and_options);
    return 0;
}
