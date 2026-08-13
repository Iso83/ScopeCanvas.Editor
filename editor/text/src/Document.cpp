#include <ScopeCanvas/editor/text/Document.h>
#include <fstream>

using namespace ScopeCanvas::Editor::Text::Annotation;

namespace ScopeCanvas::Editor::Text {
Document Document::load(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream)
        throw std::runtime_error("Unable to open text document");

    return Document(std::string(std::istreambuf_iterator<char>(stream), {}));
}

void Document::save(const std::filesystem::path& path) const {
    std::ofstream stream(path, std::ios::binary);
    const auto value = text();
    stream.write(value.data(), static_cast<std::streamsize>(value.size()));
    if (!stream)
        throw std::runtime_error("Unable to save text document");
}

[[nodiscard]] std::vector<LineSegment> Document::clipToLines(Range range) const {
    range = range.normalized();
    std::vector<LineSegment> result;

    if (!valid(range) || range.empty())
        return result;
    for (std::size_t line = range.start.line; line <= range.end.line; ++line) {
        const auto start = line == range.start.line ? range.start.column : 0;
        const auto end = line == range.end.line ? range.end.column : this->line(line).size();
        if (start < end)
            result.push_back({line, start, end});
    }

    return result;
}

std::string Document::text() const {
    std::string result;
    for (const auto& line : m_lines) {
        result += line.content;
        if (line.ending == LineEnding::LF)
            result += '\n';
        else if (line.ending == LineEnding::CRLF)
            result += "\r\n";
    }

    return result;
}

bool boundary(std::string_view value, std::size_t column) {
    return column <= value.size() &&
           (column == value.size() || (static_cast<unsigned char>(value[column]) & 0xC0U) != 0x80U);
}

bool Document::valid(Position p) const noexcept {
    return p.line < m_lines.size() && boundary(m_lines[p.line].content, p.column);
}

std::size_t Document::offset(Position p) const {
    if (!valid(p))
        throw std::out_of_range("Invalid text position");
    std::size_t result = p.column;
    for (std::size_t i = 0; i < p.line; ++i)
        result += m_lines[i].content.size() + (m_lines[i].ending == LineEnding::CRLF ? 2 : 1);
    return result;
}

Position Document::position(std::size_t value) const {
    const auto textSize = text().size();
    if (value > textSize)
        throw std::out_of_range("Invalid text offset");

    std::size_t base = 0;
    for (std::size_t i = 0; i < m_lines.size(); ++i) {
        const auto end = base + m_lines[i].content.size();
        if (value <= end) {
            Position p{i, value - base};
            if (!valid(p))
                throw std::out_of_range("Offset splits UTF-8");

            return p;
        }

        const auto endingSize =
            m_lines[i].ending == LineEnding::CRLF ? 2U : (m_lines[i].ending == LineEnding::LF ? 1U : 0U);
        if (value < end + endingSize)
            throw std::out_of_range("Offset splits a line ending");
        base = end + endingSize;
    }

    return {m_lines.size() - 1, m_lines.back().content.size()};
}

Position Document::replace(Range r, std::string_view value) {
    r = r.normalized();
    auto all = text();
    const auto begin = offset(r.start);
    const auto finish = offset(r.end);
    all.replace(begin, finish - begin, value);
    assign(all);

    return position(begin + value.size());
}

std::string Document::visibleText(std::size_t line, bool showHiddenCharacters) const {
    std::string result(this->line(line));
    if (!showHiddenCharacters)
        return result;

    std::string expanded;
    for (char c : result) {
        if (c == ' ')
            expanded += "·";
        else if (c == '\t')
            expanded += "→\t";
        else
            expanded += c;
    }

    if (lineEnding(line) == LineEnding::CRLF)
        expanded += "␍␊";
    else if (lineEnding(line) == LineEnding::LF)
        expanded += "␊";
    else if (line + 1 == lineCount())
        expanded += "⟂";

    return expanded;
}

void Document::assign(std::string_view text) {
    m_lines.clear();
    std::size_t start = 0;
    for (std::size_t i = 0; i < text.size();) {
        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            m_lines.push_back({std::string(text.substr(start, i - start)), LineEnding::CRLF});
            i += 2;
            start = i;
        } else if (text[i] == '\n') {
            m_lines.push_back({std::string(text.substr(start, i - start)), LineEnding::LF});
            ++i;
            start = i;
        } else
            ++i;
    }
    m_lines.push_back({std::string(text.substr(start)), LineEnding::None});
}
} // namespace ScopeCanvas::Editor::Text
