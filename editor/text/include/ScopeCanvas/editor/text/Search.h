#pragma once

#include <ScopeCanvas/editor/text/Document.h>

namespace ScopeCanvas::Editor::Text {
struct FindOptions {
    bool caseSensitive{};
    bool wholeWord{};
};

struct FindMatch {
    Range range;
    bool operator==(const FindMatch&) const = default;
};

class Search {
public:
    [[nodiscard]] static std::vector<FindMatch> findMatches(const Document& document, std::string_view query,
                                                            FindOptions options = {});
    [[nodiscard]] static std::size_t nextMatchIndex(const std::vector<FindMatch>& matches, Position from,
                                                    bool previous);
};
} // namespace ScopeCanvas::Editor::Text
