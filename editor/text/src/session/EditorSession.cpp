#include <ScopeCanvas/editor/text/session/EditorSession.h>
#include <stdexcept>

namespace ScopeCanvas::Editor::Text::Session {
Position previousPosition(const Document& document, Position position) {
    if (position.column == 0) {
        if (position.line == 0)
            return position;
        const auto previousLine = position.line - 1;
        return {previousLine, document.line(previousLine).size()};
    }

    auto column = position.column - 1;
    const auto line = document.line(position.line);
    while (column > 0 && (static_cast<unsigned char>(line[column]) & 0xC0U) == 0x80U)
        --column;

    return {position.line, column};
}

Position nextPosition(const Document& document, Position position) {
    const auto line = document.line(position.line);
    if (position.column == line.size()) {
        if (position.line + 1 == document.lineCount())
            return position;

        return {position.line + 1, 0};
    }

    auto column = position.column + 1;
    while (column < line.size() && (static_cast<unsigned char>(line[column]) & 0xC0U) == 0x80U)
        ++column;

    return {position.line, column};
}

bool ordinaryTyping(std::string_view text) {
    return !text.empty() && text.find_first_of("\r\n") == std::string_view::npos;
}

void EditorSession::select(Position anchor, Position active) {
    if (!m_document.valid(anchor) || !m_document.valid(active))
        throw std::out_of_range("Invalid editor selection");

    breakHistoryGroup();
    m_anchor = anchor;
    m_caret = active;
    m_preferredVisualColumn.reset();
}

bool EditorSession::typeText(std::string_view value) {
    if (value.empty())
        return false;

    const auto group =
        ordinaryTyping(value) && selection().normalized().empty() ? HistoryGroup::Typing : HistoryGroup::None;

    return replaceSelectionImpl(value, group);
}

bool EditorSession::newline() {
    auto ending = m_document.lineEnding(m_caret.line);
    if (ending == LineEnding::None && m_caret.line > 0)
        ending = m_document.lineEnding(m_caret.line - 1);

    return replaceSelectionImpl(ending == LineEnding::CRLF ? "\r\n" : "\n", HistoryGroup::None);
}

bool EditorSession::backspace() {
    if (!editable())
        return false;

    auto range = selection().normalized();
    if (range.empty()) {
        const auto begin = previousPosition(m_document, m_caret);
        if (begin == m_caret)
            return false;

        range = {begin, m_caret};
        checkpoint();
        m_caret = m_document.replace(range, {});
        m_anchor = m_caret;
        m_preferredVisualColumn.reset();

        return true;
    }

    return replaceSelectionImpl({}, HistoryGroup::None);
}

bool EditorSession::deleteForward() {
    if (!editable())
        return false;

    auto range = selection().normalized();
    if (range.empty()) {
        const auto end = nextPosition(m_document, m_caret);
        if (end == m_caret)
            return false;

        range = {m_caret, end};
        checkpoint();
        m_caret = m_document.replace(range, {});
        m_anchor = m_caret;
        m_preferredVisualColumn.reset();

        return true;
    }

    return replaceSelectionImpl({}, HistoryGroup::None);
}

std::optional<std::string> EditorSession::cut() {
    if (!editable() || selection().normalized().empty())
        return std::nullopt;

    auto result = copy();
    if (!replaceSelectionImpl({}, HistoryGroup::None))
        return std::nullopt;

    return result;
}

bool EditorSession::undo() {
    if (!canUndo())
        return false;

    m_redo.push_back(snapshot());
    auto state = std::move(m_undo.back());
    m_undo.pop_back();
    restore(std::move(state));
    breakHistoryGroup();

    return true;
}

bool EditorSession::redo() {
    if (!canRedo())
        return false;

    m_undo.push_back(snapshot());
    auto state = std::move(m_redo.back());
    m_redo.pop_back();
    restore(std::move(state));
    breakHistoryGroup();

    return true;
}

std::size_t EditorSession::replaceAll(std::string_view query, std::string_view replacement, FindOptions options) {
    const auto matches = Search::findMatches(document(), query, options);
    if (matches.empty())
        return 0;

    std::string text = document().text();
    for (std::size_t i = matches.size(); i-- > 0;) {
        const auto begin = document().offset(matches[i].range.start);
        const auto end = document().offset(matches[i].range.end);
        text.replace(begin, end - begin, replacement);
    }
    select({0, 0}, {document().lineCount() - 1, document().line(document().lineCount() - 1).size()});

    return replaceSelection(text) ? matches.size() : 0U;
}

void EditorSession::restore(Snapshot state) {
    m_document = std::move(state.document);
    m_caret = state.caret;
    m_anchor = state.anchor;
    m_preferredVisualColumn = state.preferredVisualColumn;
}

void EditorSession::checkpoint(HistoryGroup group) {
    if (group == HistoryGroup::None || m_historyGroup != group)
        m_undo.push_back(snapshot());
    m_redo.clear();
    m_historyGroup = group;
}

bool EditorSession::replaceSelectionImpl(std::string_view value, HistoryGroup group) {
    if (!editable())
        return false;

    checkpoint(group);
    m_caret = m_document.replace(selection(), value);
    m_anchor = m_caret;
    m_preferredVisualColumn.reset();

    return true;
}
} // namespace ScopeCanvas::Editor::Text::Session
