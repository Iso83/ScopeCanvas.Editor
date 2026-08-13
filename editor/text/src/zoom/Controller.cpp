#include <ScopeCanvas/editor/text/zoom/Controller.h>
#include <cmath>

using ScopeCanvas::Engine::Render::Text::FontMetrics;
using namespace ScopeCanvas::Editor::Text::Render::Window;

namespace ScopeCanvas::Editor::Text::Zoom {
bool Controller::setZoom(float zoom) noexcept {
    const float clamped = std::clamp(zoom, m_limits.minimum, m_limits.maximum);
    if (std::abs(clamped - m_zoom) < 0.0001F)
        return false;

    m_zoom = clamped;

    return true;
}

Settings Controller::effectiveSettings(const Settings& base, const FontMetrics& metrics) const noexcept {
    Settings settings = base;
    settings.lineHeight = std::max(metrics.lineHeight * m_zoom, 1.0F);
    settings.glyphAdvance = std::max(metrics.glyphAdvance * m_zoom, 1.0F);
    settings.lineNumberPadding = std::max(base.lineNumberPadding * m_zoom, 0.0F);
    settings.markerGutterWidth = std::max(base.markerGutterWidth * m_zoom, 0.0F);
    settings.codePadding = std::max(base.codePadding * m_zoom, 0.0F);
    settings.gutterSpacing = std::max(base.gutterSpacing * m_zoom, 0.0F);
    settings.foldControlWidth = std::max(base.foldControlWidth * m_zoom, 0.0F);
    settings.fontWorldScale = m_zoom;
    settings.fontAscent = metrics.ascent * m_zoom;
    settings.fontDescent = metrics.descent * m_zoom;

    return settings;
}

float Controller::preserveVisualAnchor(float previousVerticalRows, float anchorViewportY, float oldLineHeight,
                                       float newLineHeight) const noexcept {
    if (oldLineHeight <= 0.0F || newLineHeight <= 0.0F)
        return std::max(previousVerticalRows, 0.0F);

    const float anchoredRow = previousVerticalRows + std::max(anchorViewportY, 0.0F) / oldLineHeight;

    return std::max(anchoredRow - std::max(anchorViewportY, 0.0F) / newLineHeight, 0.0F);
}

HitTestResult Controller::hitTest(const Frame& frame, float x, float y) noexcept {
    const float lineHeight = std::max(frame.lineHeight, 1.0F);
    const float glyphAdvance = std::max(frame.glyphAdvance, 1.0F);
    const auto visualRow =
        static_cast<std::size_t>(std::floor(std::max(frame.verticalRows + std::max(y, 0.0F) / lineHeight, 0.0F)));
    const bool inCodeArea = x >= frame.codeOriginX;
    std::size_t column{};
    if (inCodeArea)
        column = static_cast<std::size_t>(std::floor((x - frame.codeOriginX + frame.horizontalOffset) / glyphAdvance));

    return {visualRow, column, inCodeArea};
}
} // namespace ScopeCanvas::Editor::Text::Zoom
