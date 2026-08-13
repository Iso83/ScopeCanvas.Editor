#pragma once

#include <ScopeCanvas/editor/text/Document.h>
#include <ScopeCanvas/editor/text/diff/Types.h>
#include <optional>
#include <vector>

namespace ScopeCanvas::Editor::Text::Projection {
class Projected {
public:
    struct Row {
        std::optional<std::size_t> logicalLine;
        Diff::Kind kind{Diff::Kind::Unchanged};

        bool operator==(const Row&) const = default;
    };

private:
    std::vector<Row> m_rows;
    std::vector<std::size_t> m_visualRows;

public:
    Projected() = default;
    explicit Projected(std::vector<Row> rows);

    [[nodiscard]] const std::vector<Row>& rows() const noexcept {
        return m_rows;
    }

    [[nodiscard]] std::optional<std::size_t> visualRow(std::size_t logicalLine) const noexcept;
    [[nodiscard]] std::optional<std::size_t> logicalLine(std::size_t visualRow) const noexcept;

    [[nodiscard]] static Projected identity(const Document& document);
};
} // namespace ScopeCanvas::Editor::Text::Projection
