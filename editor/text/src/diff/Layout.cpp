#include <ScopeCanvas/editor/text/diff/Layout.h>
#include <span>

using Match = std::pair<std::size_t, std::size_t>;

using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text::Projection;

namespace ScopeCanvas::Editor::Text::Diff {

namespace LCS { // Hirschberg's LCS
std::vector<std::size_t> lcsLengths(std::span<const std::string_view> left, std::span<const std::string_view> right) {
    std::vector<std::size_t> lengths(right.size() + 1);
    for (const auto leftLine : left) {
        std::size_t diagonal = 0;
        for (std::size_t j = 0; j < right.size(); ++j) {
            const auto previous = lengths[j + 1];
            if (leftLine == right[j])
                lengths[j + 1] = diagonal + 1;
            else
                lengths[j + 1] = std::max(lengths[j], lengths[j + 1]);
            diagonal = previous;
        }
    }

    return lengths;
}

void findMatches(std::span<const std::string_view> left, std::span<const std::string_view> right,
                 std::size_t leftOffset, std::size_t rightOffset, std::vector<Match>& matches) {
    if (left.empty() || right.empty())
        return;
    if (left.size() == 1) {
        const auto match = std::find(right.begin(), right.end(), left.front());
        if (match != right.end())
            matches.emplace_back(leftOffset, rightOffset + static_cast<std::size_t>(match - right.begin()));

        return;
    }

    const auto middle = left.size() / 2;
    const auto forward = lcsLengths(left.first(middle), right);
    std::vector<std::string_view> reversedLeft(left.rbegin(), left.rend());
    std::vector<std::string_view> reversedRight(right.rbegin(), right.rend());
    const auto backward = lcsLengths(std::span(reversedLeft).first(left.size() - middle), reversedRight);

    std::size_t split = 0;
    std::size_t best = 0;
    for (std::size_t j = 0; j <= right.size(); ++j) {
        const auto score = forward[j] + backward[right.size() - j];
        if (score > best) {
            best = score;
            split = j;
        }
    }

    findMatches(left.first(middle), right.first(split), leftOffset, rightOffset, matches);
    findMatches(left.subspan(middle), right.subspan(split), leftOffset + middle, rightOffset + split, matches);
}
} // namespace LCS

void appendChange(std::vector<Row>& rows, std::size_t leftBegin, std::size_t leftEnd, std::size_t rightBegin,
                  std::size_t rightEnd) {
    const auto paired = std::min(leftEnd - leftBegin, rightEnd - rightBegin);
    for (std::size_t index = 0; index < paired; ++index)
        rows.push_back({leftBegin + index, rightBegin + index, Kind::Modified});
    for (std::size_t line = leftBegin + paired; line < leftEnd; ++line)
        rows.push_back({line, {}, Kind::Added});
    for (std::size_t line = rightBegin + paired; line < rightEnd; ++line)
        rows.push_back({{}, line, Kind::Removed});
}

bool hiddenByFold(const Model& annotations, std::size_t line) {
    for (const auto& fold : annotations.foldRegions()) {
        if (!fold.collapsed)
            continue;
        if (line > fold.range.start.line && line < fold.range.end.line)
            return true;
    }

    return false;
}

Projected Layout::leftProjection() const {
    std::vector<Projected::Row> rows;
    rows.reserve(m_rows.size());
    for (const auto& row : m_rows)
        rows.push_back({row.left, row.kind});

    return Projected(std::move(rows));
}

Projected Layout::rightProjection() const {
    std::vector<Projected::Row> rows;
    rows.reserve(m_rows.size());
    for (const auto& row : m_rows)
        rows.push_back({row.right, row.kind});

    return Projected(std::move(rows));
}

Layout Layout::calculate(const Document& left, const Document& right) {
    std::vector<std::string_view> leftLines;
    std::vector<std::string_view> rightLines;
    leftLines.reserve(left.lineCount());
    rightLines.reserve(right.lineCount());

    for (std::size_t line = 0; line < left.lineCount(); ++line)
        leftLines.push_back(left.line(line));
    for (std::size_t line = 0; line < right.lineCount(); ++line)
        rightLines.push_back(right.line(line));

    std::size_t prefix = 0;
    while (prefix < leftLines.size() && prefix < rightLines.size() && leftLines[prefix] == rightLines[prefix])
        ++prefix;

    std::size_t suffix = 0;
    while (suffix < leftLines.size() - prefix && suffix < rightLines.size() - prefix &&
           leftLines[leftLines.size() - suffix - 1U] == rightLines[rightLines.size() - suffix - 1U])
        ++suffix;

    std::vector<Match> matches;
    matches.reserve(prefix + suffix);
    for (std::size_t line = 0; line < prefix; ++line)
        matches.emplace_back(line, line);

    LCS::findMatches(std::span(leftLines).subspan(prefix, leftLines.size() - prefix - suffix),
                     std::span(rightLines).subspan(prefix, rightLines.size() - prefix - suffix), prefix, prefix,
                     matches);

    for (std::size_t offset = suffix; offset > 0; --offset)
        matches.emplace_back(leftLines.size() - offset, rightLines.size() - offset);

    Layout result;
    std::size_t leftLine = 0;
    std::size_t rightLine = 0;
    for (const auto& [matchedLeft, matchedRight] : matches) {
        appendChange(result.m_rows, leftLine, matchedLeft, rightLine, matchedRight);
        result.m_rows.push_back({matchedLeft, matchedRight, Kind::Unchanged});
        leftLine = matchedLeft + 1;
        rightLine = matchedRight + 1;
    }
    appendChange(result.m_rows, leftLine, left.lineCount(), rightLine, right.lineCount());

    result.m_leftVisualRows.resize(left.lineCount());
    result.m_rightVisualRows.resize(right.lineCount());
    for (std::size_t row = 0; row < result.m_rows.size(); ++row) {
        if (result.m_rows[row].left)
            result.m_leftVisualRows[*result.m_rows[row].left] = row;
        if (result.m_rows[row].right)
            result.m_rightVisualRows[*result.m_rows[row].right] = row;
    }
    return result;
}

Layout Layout::folded(const Layout& source, const Model& leftAnnotations, const Model& rightAnnotations) {
    Layout result;
    result.m_rows.reserve(source.m_rows.size());
    for (const auto& row : source.m_rows) {
        const bool hideLeft = row.left && hiddenByFold(leftAnnotations, *row.left);
        const bool hideRight = row.right && hiddenByFold(rightAnnotations, *row.right);
        if (hideLeft || hideRight)
            continue;
        result.m_rows.push_back(row);
    }

    std::size_t leftCount = 0;
    std::size_t rightCount = 0;
    for (const auto& row : result.m_rows) {
        if (row.left)
            leftCount = std::max(leftCount, *row.left + 1U);
        if (row.right)
            rightCount = std::max(rightCount, *row.right + 1U);
    }

    result.m_leftVisualRows.assign(leftCount, std::numeric_limits<std::size_t>::max());
    result.m_rightVisualRows.assign(rightCount, std::numeric_limits<std::size_t>::max());
    for (std::size_t row = 0; row < result.m_rows.size(); ++row) {
        if (result.m_rows[row].left)
            result.m_leftVisualRows[*result.m_rows[row].left] = row;
        if (result.m_rows[row].right)
            result.m_rightVisualRows[*result.m_rows[row].right] = row;
    }

    return result;
}
} // namespace ScopeCanvas::Editor::Text::Diff
