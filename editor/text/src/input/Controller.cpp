#include <ScopeCanvas/editor/text/input/Controller.h>
#include <ScopeCanvas/editor/text/zoom/Controller.h>

using namespace ScopeCanvas::Editor::Text::Session;

namespace ScopeCanvas::Editor::Text::Input {
Position clampLineColumn(const Document& document, std::size_t line, std::size_t column) {
    line = std::min(line, document.lineCount() - 1U);
    column = std::min(column, document.line(line).size());
    while (column > 0 && !document.valid(Position{line, column}))
        --column;

    return {line, column};
}

Position previousCodePoint(const Document& document, Position position) {
    if (position.column == 0) {
        if (position.line == 0)
            return position;
        const auto line = position.line - 1;

        return {line, document.line(line).size()};
    }

    auto column = position.column - 1;
    const auto text = document.line(position.line);
    while (column > 0 && (static_cast<unsigned char>(text[column]) & 0xC0U) == 0x80U)
        --column;

    return {position.line, column};
}

Position nextCodePoint(const Document& document, Position position) {
    const auto text = document.line(position.line);
    if (position.column == text.size()) {
        if (position.line + 1 == document.lineCount())
            return position;

        return {position.line + 1, 0};
    }

    auto column = position.column + 1;
    while (column < text.size() && (static_cast<unsigned char>(text[column]) & 0xC0U) == 0x80U)
        ++column;

    return {position.line, column};
}

bool wordByte(unsigned char value) {
    return std::isalnum(value) != 0 || value == '_';
}

bool Controller::caretVisible(std::chrono::steady_clock::time_point now) const noexcept {
    if (!m_focused || m_session->readOnly() || m_session->mode() != Mode::Insert)
        return false;

    return (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_blinkEpoch).count() / 500) % 2 == 0;
}

Result Controller::focus(std::chrono::steady_clock::time_point now) noexcept {
    const bool changed = !m_focused;
    m_focused = true;
    resetBlink(now);
    return {true, changed, false, false};
}

Result Controller::mousePress(const Geometry& geometry, float x, float y, Modifiers modifiers,
                              std::chrono::steady_clock::time_point now) {
    focus(now);
    m_session->moveCaret(clampHit(geometry, x, y), modifiers.shift);
    m_dragging = true;
    markDirty();
    return {true, true, false, false};
}

Result Controller::mouseDrag(const Geometry& geometry, float x, float y, std::chrono::steady_clock::time_point now) {
    if (!m_dragging || !m_focused)
        return {};
    Result result{true, true, false, false};
    if (y < 0.0F) {
        m_scroll->verticalRows = std::max(m_scroll->verticalRows - 1.0F, 0.0F);
        result.scrollDirty = true;
    } else if (y > geometry.viewportHeight) {
        m_scroll->verticalRows += 1.0F;
        result.scrollDirty = true;
    }
    m_session->moveCaret(clampHit(geometry, x, y), true);
    resetBlink(now);
    return result;
}

Result Controller::key(Key input, Modifiers modifiers, std::size_t pageRows) {
    if (!m_focused)
        return {};
    auto& editor = *m_session;
    Position target = editor.caret();
    bool navigation = true;
    switch (input) {
        case Key::InsertMode:
            editor.setMode(Mode::Insert);
            navigation = false;
            break;
        case Key::NormalMode:
            editor.setMode(Mode::Normal);
            navigation = false;
            break;
        case Key::CharacterLeft:
        case Key::CharacterRight:
        case Key::WordLeft:
        case Key::WordRight:
            target = moveHorizontal(editor.document(), target, input);
            break;
        case Key::LineUp:
            target = moveVertical(editor, -1);
            break;
        case Key::LineDown:
            target = moveVertical(editor, 1);
            break;
        case Key::Home:
            target = {target.line, 0U};
            break;
        case Key::End:
            target = {target.line, editor.document().line(target.line).size()};
            break;
        case Key::DocumentStart:
            target = {0, 0};
            break;
        case Key::DocumentEnd:
            target = {editor.document().lineCount() - 1U,
                      editor.document().line(editor.document().lineCount() - 1U).size()};
            break;
        case Key::PageUp:
            target = moveVertical(editor, -static_cast<int>(pageRows));
            break;
        case Key::PageDown:
            target = moveVertical(editor, static_cast<int>(pageRows));
            break;
    }
    const auto preferred = editor.preferredVisualColumn().value_or(editor.caret().column);
    if (navigation) {
        editor.moveCaret(target, modifiers.shift);
        if (input == Key::LineUp || input == Key::LineDown || input == Key::PageUp || input == Key::PageDown)
            editor.setPreferredVisualColumn(preferred);
    }
    markDirty();
    return {true, true, false, input == Key::PageUp || input == Key::PageDown};
}

Result Controller::textInput(std::string_view text) {
    if (!m_focused || !m_session->typeText(text))
        return {};
    markDirty();
    return {true, true, true, false};
}

Position Controller::clampHit(const Geometry& geometry, float x, float y) const {
    const auto hit = Zoom::Controller::hitTest(geometry.frame, x, y);
    const auto& document = m_session->document();
    std::size_t line = document.lineCount() - 1U;
    for (const auto& row : geometry.frame.rows) {
        if (row.visualRow == hit.visualRow && row.logicalLine) {
            line = *row.logicalLine;
            const std::size_t column =
                row.visualToSourceColumns.empty()
                    ? hit.column
                    : row.visualToSourceColumns[std::min(hit.column, row.visualToSourceColumns.size() - 1U)];
            return clampLineColumn(document, line, column);
        }
    }
    return clampLineColumn(document, line, hit.column);
}

Position Controller::moveHorizontal(const Document& document, Position position, Key input) const {
    if (input == Key::CharacterLeft)
        return previousCodePoint(document, position);

    if (input == Key::CharacterRight)
        return nextCodePoint(document, position);

    if (input == Key::WordLeft) {
        auto p = previousCodePoint(document, position);
        while (p != Position{} && p.column > 0 &&
               !wordByte(static_cast<unsigned char>(document.line(p.line)[p.column - 1])))
            p = previousCodePoint(document, p);
        while (p.column > 0 && wordByte(static_cast<unsigned char>(document.line(p.line)[p.column - 1])))
            p = previousCodePoint(document, p);
        return p;
    }

    auto p = position;
    while (nextCodePoint(document, p) != p && p.column < document.line(p.line).size() &&
           wordByte(static_cast<unsigned char>(document.line(p.line)[p.column])))
        p = nextCodePoint(document, p);
    while (nextCodePoint(document, p) != p && p.column < document.line(p.line).size() &&
           !wordByte(static_cast<unsigned char>(document.line(p.line)[p.column])))
        p = nextCodePoint(document, p);

    return p;
}

Position Controller::moveVertical(EditorSession& editor, int deltaRows) const {
    const auto& document = editor.document();
    const auto preferred = editor.preferredVisualColumn().value_or(editor.caret().column);
    const auto line = static_cast<std::size_t>(std::clamp<int>(static_cast<int>(editor.caret().line) + deltaRows, 0,
                                                               static_cast<int>(document.lineCount() - 1U)));
    return clampLineColumn(document, line, preferred);
}
} // namespace ScopeCanvas::Editor::Text::Input
