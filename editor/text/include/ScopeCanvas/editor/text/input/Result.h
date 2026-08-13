#pragma once

namespace ScopeCanvas::Editor::Text::Input {
struct Result {
    bool handled{};
    bool dirty{};
    bool documentChanged{};
    bool scrollDirty{};
};
} // namespace ScopeCanvas::Editor::Text::Input
