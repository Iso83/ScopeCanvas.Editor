#include <ScopeCanvas/editor/text/render/window/TextView.h>
#include <cmath>

namespace ScopeCanvas::Editor::Text::Render::Window {
TextView::TextView(Session::EditorSession& session)
    : m_session(&session), m_unfoldedProjection(Projection::Projected::identity(session.document())),
      m_projection(m_unfoldedProjection), m_input(session, m_scroll) {
    m_canvas.registerDrawContext(&m_drawContext);
}

void TextView::synchronizeFoldProjection() {
    std::vector<Projection::Projected::Row> rows;
    for (const auto& row : m_unfoldedProjection.rows()) {
        const bool hidden = row.logicalLine && std::ranges::any_of(m_annotations.foldRegions(), [&](const auto& fold) {
                                return fold.collapsed && *row.logicalLine > fold.range.start.line &&
                                       *row.logicalLine < fold.range.end.line;
                            });
        if (!hidden)
            rows.push_back(row);
    }
    m_projection = Projection::Projected(std::move(rows));
    m_input.markDirty();
}

bool TextView::setFoldCollapsed(std::string_view id, bool collapsed) {
    if (!m_annotations.setFoldCollapsed(std::string{id}, collapsed))
        return false;

    synchronizeFoldProjection();
    return true;
}

bool TextView::toggleFold(std::string_view id) {
    const auto* fold = m_annotations.foldRegion(std::string{id});
    return fold && setFoldCollapsed(id, !fold->collapsed);
}

bool TextView::toggleFoldAt(float x, float y) {
    const float markerStart = m_frame.lineNumberGutterWidth;
    const float markerEnd = markerStart + m_frame.markerGutterWidth;
    if (x < markerStart || x >= markerEnd || y < 0.0F || m_frame.lineHeight <= 0.0F)
        return false;

    const auto visualRow = static_cast<std::size_t>(std::floor(y / m_frame.lineHeight + m_frame.verticalRows));
    const auto row = std::ranges::find(m_frame.rows, visualRow, &decltype(m_frame.rows)::value_type::visualRow);
    return row != m_frame.rows.end() && row->foldControl && toggleFold(row->foldControl->id);
}

const Frame& TextView::buildFrame(float viewportHeight, bool active, bool showCaret) {
    m_frame = Model{}.buildFrame(m_session->document(), m_projection, m_annotations, m_session, m_settings, m_scroll,
                                 viewportHeight, active, showCaret);
    m_drawContext.setFrame(m_frame);
    return m_frame;
}

void VerticalLink::synchronize(TextView& source, TextView& destination) noexcept {
    if (m_updating)
        return;

    m_updating = true;
    destination.scroll().verticalRows = source.scroll().verticalRows;
    m_updating = false;
}
} // namespace ScopeCanvas::Editor::Text::Render::Window
