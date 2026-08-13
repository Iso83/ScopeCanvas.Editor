#pragma once

namespace ScopeCanvas::Editor::Text::Zoom {
struct Limits {
    float minimum{0.5F};
    float maximum{3.0F};
    float step{1.10F};
};
} // namespace ScopeCanvas::Editor::Text::Zoom
