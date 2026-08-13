#pragma once

#include <ScopeCanvas/editor/text/Position.h>

namespace ScopeCanvas::Editor::Text {
struct Range {
    Position start{};
    Position end{};
    bool operator==(const Range&) const = default;
    [[nodiscard]] Range normalized() const noexcept {
        return end < start ? Range{end, start} : *this;
    }
    [[nodiscard]] bool empty() const noexcept {
        return start == end;
    }
};
} // namespace ScopeCanvas::Editor::Text
