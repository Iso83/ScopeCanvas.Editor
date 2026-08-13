#pragma once

#include <algorithm>

namespace ScopeCanvas::Editor::Text::Scroll {
struct Vertical {
    float totalRows{};
    float visibleRows{};
    float verticalRows{};

    [[nodiscard]] float maximumVerticalRows() const noexcept {
        return std::max(totalRows - visibleRows, 0.0F);
    }
    [[nodiscard]] float normalizedVerticalPosition() const noexcept {
        const float maximum = maximumVerticalRows();
        return maximum > 0.0F ? std::clamp(verticalRows / maximum, 0.0F, 1.0F) : 0.0F;
    }
    void clamp() noexcept {
        verticalRows = std::clamp(verticalRows, 0.0F, maximumVerticalRows());
    }
    void setNormalizedVerticalPosition(float normalized) noexcept {
        verticalRows = std::clamp(normalized, 0.0F, 1.0F) * maximumVerticalRows();
    }
    void scrollRows(float deltaRows) noexcept {
        verticalRows += deltaRows;
        clamp();
    }
};
} // namespace ScopeCanvas::Editor::Text::Scroll
