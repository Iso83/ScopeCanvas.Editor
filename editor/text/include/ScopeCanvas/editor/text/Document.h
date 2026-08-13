#pragma once

#include <ScopeCanvas/editor/text/Line.h>
#include <ScopeCanvas/editor/text/Range.h>
#include <ScopeCanvas/editor/text/annotation/Segments.h>
#include <filesystem>
#include <vector>

namespace ScopeCanvas::Editor::Text {
class Document {
private:
    std::vector<Line> m_lines;

public:
    Document() {
        assign({});
    }
    explicit Document(std::string_view text) {
        assign(text);
    }

    static Document load(const std::filesystem::path& path);
    void save(const std::filesystem::path& path) const;

    [[nodiscard]] std::size_t lineCount() const noexcept {
        return m_lines.size();
    }
    [[nodiscard]] std::string_view line(std::size_t index) const {
        return m_lines.at(index).content;
    }
    [[nodiscard]] LineEnding lineEnding(std::size_t index) const {
        return m_lines.at(index).ending;
    }
    [[nodiscard]] std::vector<Annotation::LineSegment> clipToLines(Range range) const;
    [[nodiscard]] std::string text() const;
    [[nodiscard]] bool valid(Position position) const noexcept;
    [[nodiscard]] bool valid(Range range) const noexcept {
        return valid(range.start) && valid(range.end);
    }
    [[nodiscard]] std::size_t offset(Position position) const;
    [[nodiscard]] Position position(std::size_t offset) const;
    [[nodiscard]] std::string slice(Range range) const {
        range = range.normalized();
        return text().substr(offset(range.start), offset(range.end) - offset(range.start));
    }

    Position insert(Position at, std::string_view text) {
        return replace({at, at}, text);
    }
    void erase(Range range) {
        replace(range, {});
    }
    Position replace(Range range, std::string_view text);
    [[nodiscard]] std::string visibleText(std::size_t line, bool showHiddenCharacters) const;

private:
    void assign(std::string_view text);
};
} // namespace ScopeCanvas::Editor::Text
