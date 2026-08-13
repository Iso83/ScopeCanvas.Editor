#include <ScopeCanvas/editor/text/projection/Projected.h>
#include <algorithm>
#include <limits>

namespace ScopeCanvas::Editor::Text::Projection {
Projected::Projected(std::vector<Row> rows) : m_rows(std::move(rows)) {
    std::size_t lineCount = 0;
    for (const auto& row : m_rows)
        if (row.logicalLine)
            lineCount = std::max(lineCount, *row.logicalLine + 1U);

    m_visualRows.assign(lineCount, std::numeric_limits<std::size_t>::max());
    for (std::size_t row = 0; row < m_rows.size(); ++row)
        if (m_rows[row].logicalLine)
            m_visualRows[*m_rows[row].logicalLine] = row;
}

std::optional<std::size_t> Projected::visualRow(std::size_t logicalLine) const noexcept {
    return logicalLine < m_visualRows.size() && m_visualRows[logicalLine] != std::numeric_limits<std::size_t>::max()
               ? std::optional{m_visualRows[logicalLine]}
               : std::nullopt;
}

std::optional<std::size_t> Projected::logicalLine(std::size_t visualRow) const noexcept {
    return visualRow < m_rows.size() ? m_rows[visualRow].logicalLine : std::nullopt;
}

Projected Projected::identity(const Document& document) {
    std::vector<Row> rows;
    rows.reserve(document.lineCount());
    for (std::size_t line = 0; line < document.lineCount(); ++line)
        rows.push_back({line, Diff::Kind::Unchanged});
    return Projected(std::move(rows));
}
} // namespace ScopeCanvas::Editor::Text::Projection
