#pragma once

#include <ScopeCanvas/editor/text/Search.h>
#include <ScopeCanvas/editor/text/session/HistoryGroup.h>
#include <ScopeCanvas/editor/text/session/Mode.h>
#include <ScopeCanvas/editor/text/session/Snapshot.h>

namespace ScopeCanvas::Editor::Text::Session {
class EditorSession {
private:
    Document m_document;
    Position m_caret{};
    Position m_anchor{};
    std::optional<std::size_t> m_preferredVisualColumn;
    bool m_readOnly{};
    Mode m_mode{Mode::Insert};
    std::vector<Snapshot> m_undo;
    std::vector<Snapshot> m_redo;
    HistoryGroup m_historyGroup{HistoryGroup::None};

public:
    explicit EditorSession(Document document = {}, bool readOnly = false)
        : m_document(std::move(document)), m_readOnly(readOnly) {}

    [[nodiscard]] const Document& document() const noexcept {
        return m_document;
    }
    [[nodiscard]] Position caret() const noexcept {
        return m_caret;
    }
    [[nodiscard]] Position anchor() const noexcept {
        return m_anchor;
    }
    [[nodiscard]] Range selection() const noexcept {
        return {m_anchor, m_caret};
    }
    [[nodiscard]] std::optional<std::size_t> preferredVisualColumn() const noexcept {
        return m_preferredVisualColumn;
    }
    [[nodiscard]] bool readOnly() const noexcept {
        return m_readOnly;
    }
    [[nodiscard]] Mode mode() const noexcept {
        return m_mode;
    }

    void setMode(Mode mode) noexcept {
        if (m_mode != mode)
            breakHistoryGroup();
        m_mode = mode;
    }
    void setPreferredVisualColumn(std::optional<std::size_t> column) noexcept {
        m_preferredVisualColumn = column;
    }

    void select(Position anchor, Position active);
    void moveCaret(Position active, bool extendSelection = false) {
        select(extendSelection ? m_anchor : active, active);
    }
    bool typeText(std::string_view text);
    bool newline();
    bool replaceSelection(std::string_view text) {
        return replaceSelectionImpl(text, HistoryGroup::None);
    }
    bool backspace();
    bool deleteForward();

    [[nodiscard]] std::string copy() const {
        return m_document.slice(selection());
    }
    [[nodiscard]] std::optional<std::string> cut();
    bool paste(std::string_view text) {
        if (text.empty())
            return false;
        return replaceSelectionImpl(text, HistoryGroup::None);
    }

    [[nodiscard]] bool canUndo() const noexcept {
        return !m_undo.empty() && !m_readOnly;
    }
    [[nodiscard]] bool canRedo() const noexcept {
        return !m_redo.empty() && !m_readOnly;
    }
    bool undo();
    bool redo();

    std::size_t replaceAll(std::string_view query, std::string_view replacement, FindOptions options = {});

private:
    [[nodiscard]] bool editable() const noexcept {
        return !m_readOnly && m_mode == Mode::Insert;
    }
    [[nodiscard]] Snapshot snapshot() const {
        return {m_document, m_caret, m_anchor, m_preferredVisualColumn};
    }
    void restore(Snapshot snapshot);
    void checkpoint(HistoryGroup group = HistoryGroup::None);
    void breakHistoryGroup() noexcept {
        m_historyGroup = HistoryGroup::None;
    }
    bool replaceSelectionImpl(std::string_view text, HistoryGroup group);
};
} // namespace ScopeCanvas::Editor::Text::Session
