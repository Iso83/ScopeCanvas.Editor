#pragma once

#include <ScopeCanvas/editor/text/render/window/Frame.h>

namespace ScopeCanvas::Editor::Text::Input {
struct Geometry {
    Render::Window::Frame frame;
    float viewportHeight{};
};
} // namespace ScopeCanvas::Editor::Text::Input
