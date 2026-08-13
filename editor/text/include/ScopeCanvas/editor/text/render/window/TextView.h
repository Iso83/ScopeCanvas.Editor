#pragma once

#include <ScopeCanvas/editor/text/annotation/Model.h>
#include <ScopeCanvas/editor/text/input/Controller.h>
#include <ScopeCanvas/editor/text/projection/Projected.h>
#include <ScopeCanvas/editor/text/render/window/DrawContext.h>
#include <ScopeCanvas/editor/text/render/window/Model.h>
#include <ScopeCanvas/engine/render/window/Canvas.h>

namespace ScopeCanvas::Editor::Text::Render::Window {
class TextView {
private:
    Session::EditorSession* m_session{};
    Annotation::Model m_annotations;
    Projection::Projected m_projection;
    Settings m_settings;
    Scroll m_scroll;
    Frame m_frame;
    Engine::Render::Window::Canvas m_canvas;
    DrawContext m_drawContext;
    Input::Controller m_input;

public:
    explicit TextView(Session::EditorSession& session);

    [[nodiscard]] Session::EditorSession& session() noexcept {
        return *m_session;
    }
    [[nodiscard]] Annotation::Model& annotations() noexcept {
        return m_annotations;
    }
    [[nodiscard]] const Projection::Projected& projection() const noexcept {
        return m_projection;
    }
    [[nodiscard]] Settings& settings() noexcept {
        return m_settings;
    }
    [[nodiscard]] Scroll& scroll() noexcept {
        return m_scroll;
    }
    [[nodiscard]] const Frame& frame() const noexcept {
        return m_frame;
    }
    [[nodiscard]] Engine::Render::Window::Canvas& canvas() noexcept {
        return m_canvas;
    }
    [[nodiscard]] DrawContext& drawContext() noexcept {
        return m_drawContext;
    }
    [[nodiscard]] Input::Controller& input() noexcept {
        return m_input;
    }

    void setProjection(Projection::Projected projection) {
        m_projection = std::move(projection);
    }
    const Frame& buildFrame(float viewportHeight, bool active = false, bool showCaret = true);
};

class VerticalLink {
private:
    bool m_updating{};

public:
    void synchronize(TextView& source, TextView& destination) noexcept;
};
} // namespace ScopeCanvas::Editor::Text::Render::Window
