#include <ScopeCanvas/editor/text/render/window/TextView.h>

namespace ScopeCanvas::Editor::Text::Render::Window {
TextView::TextView(Session::EditorSession& session)
    : m_session(&session), m_projection(Projection::Projected::identity(session.document())),
      m_input(session, m_scroll) {
    m_canvas.registerDrawContext(&m_drawContext);
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
