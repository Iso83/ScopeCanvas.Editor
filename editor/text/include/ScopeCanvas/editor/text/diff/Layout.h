#pragma once

#include <ScopeCanvas/editor/text/annotation/Model.h>
#include <ScopeCanvas/editor/text/diff/Types.h>
#include <ScopeCanvas/editor/text/projection/Projected.h>
#include <limits>

namespace ScopeCanvas::Editor::Text::Diff {
class Layout {
private:
    std::vector<Row> m_rows;
    std::vector<std::size_t> m_leftVisualRows;
    std::vector<std::size_t> m_rightVisualRows;

public:
    [[nodiscard]] const std::vector<Row>& rows() const noexcept {
        return m_rows;
    }

    [[nodiscard]] std::optional<std::size_t> visualRow(bool leftPane, std::size_t logicalLine) const {
        const auto& mapping = leftPane ? m_leftVisualRows : m_rightVisualRows;
        return logicalLine < mapping.size() && mapping[logicalLine] != std::numeric_limits<std::size_t>::max()
                   ? std::optional{mapping[logicalLine]}
                   : std::nullopt;
    }

    [[nodiscard]] std::optional<std::size_t> logicalLine(bool leftPane, std::size_t visualRow) const {
        if (visualRow >= m_rows.size())
            return {};

        return leftPane ? m_rows[visualRow].left : m_rows[visualRow].right;
    }

    [[nodiscard]] std::optional<VisualPosition> visualPosition(bool leftPane, Position position) const {
        const auto row = visualRow(leftPane, position.line);
        return row ? std::optional{VisualPosition{*row, position.column}} : std::nullopt;
    }

    [[nodiscard]] std::optional<Position> logicalPosition(bool leftPane, VisualPosition position) const {
        const auto line = logicalLine(leftPane, position.row);
        return line ? std::optional{Position{*line, position.column}} : std::nullopt;
    }

    [[nodiscard]] std::optional<VisualRange> visualRange(bool leftPane, Range range) const {
        const auto start = visualPosition(leftPane, range.start);
        const auto end = visualPosition(leftPane, range.end);
        return start && end ? std::optional{VisualRange{*start, *end}} : std::nullopt;
    }

    [[nodiscard]] Projection::Projected leftProjection() const;
    [[nodiscard]] Projection::Projected rightProjection() const;
    static Layout calculate(const Document& left, const Document& right);
    static Layout folded(const Layout& source, const Annotation::Model& leftAnnotations,
                         const Annotation::Model& rightAnnotations);
};
} // namespace ScopeCanvas::Editor::Text::Diff
