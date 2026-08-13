#pragma once

namespace ScopeCanvas::Editor::Text::Input {

enum class Key {
    CharacterLeft,
    CharacterRight,
    WordLeft,
    WordRight,
    LineUp,
    LineDown,
    DocumentStart,
    DocumentEnd,
    Home,
    End,
    PageUp,
    PageDown,
    InsertMode,
    NormalMode
};

struct Modifiers {
    bool shift{};
    bool ctrl{};
};
} // namespace ScopeCanvas::Editor::Text::Input
