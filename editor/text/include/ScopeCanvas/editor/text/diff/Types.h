#pragma once

#include <optional>
#include <stdlib.h>

namespace ScopeCanvas::Editor::Text::Diff {
enum class Kind { Unchanged, Added, Removed, Modified };

struct VisualPosition {
    std::size_t row{};
    std::size_t column{};
    auto operator<=>(const VisualPosition&) const = default;
};

struct VisualRange {
    VisualPosition start{};
    VisualPosition end{};
    bool operator==(const VisualRange&) const = default;
};

struct Row {
    std::optional<std::size_t> left;
    std::optional<std::size_t> right;
    Kind kind{Kind::Unchanged};
    bool operator==(const Row&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Diff
