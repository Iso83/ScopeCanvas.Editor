#include <ScopeCanvas/editor/text/Search.h>
#include <algorithm>

namespace ScopeCanvas::Editor::Text {
char lowerAscii(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

bool wordByte(unsigned char value) {
    return std::isalnum(value) != 0 || value == '_';
}

bool wholeWordAt(std::string_view text, std::size_t begin, std::size_t end) {
    const bool left = begin > 0 && wordByte(static_cast<unsigned char>(text[begin - 1]));
    const bool right = end < text.size() && wordByte(static_cast<unsigned char>(text[end]));
    return !left && !right;
}

std::string comparable(std::string_view value, bool caseSensitive) {
    std::string result(value);
    if (!caseSensitive)
        std::transform(result.begin(), result.end(), result.begin(), lowerAscii);

    return result;
}

std::vector<FindMatch> Search::findMatches(const Document& document, std::string_view query, FindOptions options) {
    std::vector<FindMatch> result;
    if (query.empty())
        return result;

    const std::string needle = comparable(query, options.caseSensitive);
    for (std::size_t line = 0; line < document.lineCount(); ++line) {
        const std::string haystack = comparable(document.line(line), options.caseSensitive);
        for (std::size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + 1)) {
            const auto end = at + needle.size();
            if (!options.wholeWord || wholeWordAt(document.line(line), at, end))
                result.push_back({{{line, at}, {line, end}}});
        }
    }

    return result;
}

std::size_t Search::nextMatchIndex(const std::vector<FindMatch>& matches, Position from, bool previous) {
    if (matches.empty())
        return 0;

    if (previous) {
        for (std::size_t i = matches.size(); i-- > 0;) {
            if (matches[i].range.start < from)
                return i;
        }

        return matches.size() - 1;
    }

    for (std::size_t i = 0; i < matches.size(); ++i) {
        if (!(matches[i].range.start < from))
            return i;
    }

    return 0;
}
} // namespace ScopeCanvas::Editor::Text
