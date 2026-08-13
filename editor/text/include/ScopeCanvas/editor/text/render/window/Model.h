#pragma once

#include <ScopeCanvas/editor/text/annotation/Model.h>
#include <ScopeCanvas/editor/text/projection/Projected.h>
#include <ScopeCanvas/editor/text/render/window/Frame.h>
#include <ScopeCanvas/editor/text/render/window/Scroll.h>
#include <ScopeCanvas/editor/text/render/window/Settings.h>
#include <ScopeCanvas/editor/text/session/EditorSession.h>

namespace ScopeCanvas::Editor::Text::Render::Window {
class Model {
public:
    [[nodiscard]] static std::size_t visualColumnCount(std::string_view text) noexcept;
    [[nodiscard]] Frame
    buildFrame(const Document& document, const Projection::Projected& projection, const Annotation::Model& annotations,
               const Session::EditorSession* session, const Settings& settings, const Scroll& scroll,
               float viewportHeight, bool activePane = false, bool showCaret = true,
               const std::vector<Annotation::StyledLineSegment>* preparedForeground = nullptr) const;
};
} // namespace ScopeCanvas::Editor::Text::Render::Window
