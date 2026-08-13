#pragma once

#include <ScopeCanvas/editor/text/Document.h>

namespace ScopeCanvas::Editor::Text::Session {
struct Snapshot {
    Document document;
    Position caret;
    Position anchor;
    std::optional<std::size_t> preferredVisualColumn;
};
} // namespace ScopeCanvas::Editor::Text::Session
