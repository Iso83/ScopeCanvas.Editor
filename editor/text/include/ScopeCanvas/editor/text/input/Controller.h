#pragma once

#include <ScopeCanvas/editor/text/input/Geometry.h>
#include <ScopeCanvas/editor/text/input/Result.h>
#include <ScopeCanvas/editor/text/input/Types.h>
#include <ScopeCanvas/editor/text/render/window/Scroll.h>
#include <ScopeCanvas/editor/text/session/EditorSession.h>
#include <chrono>

namespace ScopeCanvas::Editor::Text::Input {
class Controller {
private:
    Session::EditorSession* m_session{};
    Render::Window::Scroll* m_scroll{};
    bool m_focused{};
    bool m_dragging{};
    bool m_dirty{true};
    std::chrono::steady_clock::time_point m_blinkEpoch{};

public:
    Controller(Session::EditorSession& session, Render::Window::Scroll& scroll)
        : m_session(&session), m_scroll(&scroll) {}

    [[nodiscard]] bool focused() const noexcept {
        return m_focused;
    }
    [[nodiscard]] bool caretVisible(std::chrono::steady_clock::time_point now) const noexcept;
    [[nodiscard]] std::string_view modeIndicator() const noexcept {
        return m_session->mode() == Session::Mode::Insert ? "INSERT" : "NORMAL";
    }
    [[nodiscard]] bool dirty() const noexcept {
        return m_dirty;
    }
    void clearDirty() noexcept {
        m_dirty = false;
    }
    void markDirty() noexcept {
        m_dirty = true;
    }
    Result focus(std::chrono::steady_clock::time_point now) noexcept;
    void blur() noexcept {
        m_focused = false;
        m_dragging = false;
        markDirty();
    }

    Result mousePress(const Geometry& geometry, float x, float y, Modifiers modifiers,
                      std::chrono::steady_clock::time_point now);
    Result mouseDrag(const Geometry& geometry, float x, float y, std::chrono::steady_clock::time_point now);
    Result mouseRelease() noexcept {
        m_dragging = false;
        return {true, false, false, false};
    }
    Result key(Key key, Modifiers modifiers, std::size_t pageRows = 10U);
    Result textInput(std::string_view text);

private:
    [[nodiscard]] Position clampHit(const Geometry& geometry, float x, float y) const;
    [[nodiscard]] Position moveHorizontal(const Document& document, Position position, Key key) const;
    [[nodiscard]] Position moveVertical(Session::EditorSession& editor, int deltaRows) const;
    void resetBlink(std::chrono::steady_clock::time_point now) noexcept {
        m_blinkEpoch = now;
        markDirty();
    }
};
} // namespace ScopeCanvas::Editor::Text::Input
