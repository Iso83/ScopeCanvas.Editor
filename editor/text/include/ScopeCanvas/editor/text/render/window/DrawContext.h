#pragma once

#include <ScopeCanvas/editor/text/render/window/Frame.h>
#include <ScopeCanvas/editor/text/render/window/Settings.h>
#include <ScopeCanvas/engine/render/text/Renderer.h>
#include <ScopeCanvas/engine/render/window/DrawContext.h>
#include <ScopeCanvas/engine/render/window/Viewport.h>
#include <memory>

namespace ScopeCanvas::Editor::Text::Render::Window {
class DrawContext final : public Engine::Render::Window::DrawContext {
private:
    Frame m_frame;
    std::shared_ptr<Engine::Render::Text::Renderer> m_textRenderer;
    ColorProfile m_colors;
    bool m_dirty{true};

public:
    DrawContext() = default;
    ~DrawContext() override = default;
    DrawContext(const DrawContext&) = delete;

    DrawContext& operator=(const DrawContext&) = delete;

    [[nodiscard]] const Frame& frame() const noexcept {
        return m_frame;
    }
    bool needsRender() override {
        return m_dirty;
    }
    void setFrame(Frame frame) {
        if (m_frame == frame)
            return;
        m_frame = std::move(frame);
        m_dirty = true;
    }

    void setTextRenderer(std::shared_ptr<Engine::Render::Text::Renderer> renderer) noexcept {
        if (m_textRenderer == renderer)
            return;
        m_textRenderer = std::move(renderer);
        m_dirty = true;
    }

    void setColorProfile(ColorProfile colors) noexcept {
        if (m_colors == colors)
            return;
        m_colors = std::move(colors);
        m_dirty = true;
    }

    void draw(ScopeCanvas::Engine::Render::Window::Viewport* view) override;
};
} // namespace ScopeCanvas::Editor::Text::Render::Window
