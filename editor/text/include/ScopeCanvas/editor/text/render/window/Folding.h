#pragma once

#include <string>

namespace ScopeCanvas::Editor::Text::Render::Window {
struct FoldControl {
    std::string id;
    bool collapsed{};
    std::string placeholder;
    bool operator==(const FoldControl&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Render::Window
