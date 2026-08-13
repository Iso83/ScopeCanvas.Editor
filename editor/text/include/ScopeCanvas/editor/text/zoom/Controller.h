#pragma once

#include <ScopeCanvas/editor/text/render/window/Frame.h>
#include <ScopeCanvas/editor/text/render/window/Settings.h>
#include <ScopeCanvas/editor/text/zoom/HitTest.h>
#include <ScopeCanvas/editor/text/zoom/Limits.h>
#include <ScopeCanvas/engine/render/text/FontMetrics.h>
#include <algorithm>

namespace ScopeCanvas::Editor::Text::Zoom {
class Controller {
private:
    Limits m_limits{};
    float m_zoom{1.0F};

public:
    explicit Controller(Limits limits = {}) : m_limits(limits) {
        m_limits.minimum = std::max(m_limits.minimum, 0.1F);
        m_limits.maximum = std::max(m_limits.maximum, m_limits.minimum);
        m_limits.step = std::max(m_limits.step, 1.01F);
    }

    [[nodiscard]] float zoom() const noexcept {
        return m_zoom;
    }
    [[nodiscard]] Limits limits() const noexcept {
        return m_limits;
    }
    [[nodiscard]] bool setZoom(float zoom) noexcept;
    [[nodiscard]] bool zoomIn() noexcept {
        return setZoom(m_zoom * m_limits.step);
    }
    [[nodiscard]] bool zoomOut() noexcept {
        return setZoom(m_zoom / m_limits.step);
    }
    [[nodiscard]] bool reset() noexcept {
        return setZoom(1.0F);
    }
    [[nodiscard]] Render::Window::Settings
    effectiveSettings(const Render::Window::Settings& base,
                      const Engine::Render::Text::FontMetrics& metrics) const noexcept;
    [[nodiscard]] float preserveVisualAnchor(float previousVerticalRows, float anchorViewportY, float oldLineHeight,
                                             float newLineHeight) const noexcept;
    [[nodiscard]] static HitTestResult hitTest(const Render::Window::Frame& frame, float x, float y) noexcept;
};
} // namespace ScopeCanvas::Editor::Text::Zoom
